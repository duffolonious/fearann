/*
 * srvcontentmgr.cpp
 * Copyright (C) 2005-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "srvcontentmgr.h"

#include "common/net/msgs.h"
#include "common/configmgr.h"

#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <cerrno>

#include <deque>
#include <cstring>


/// If player has this number of bytes in send queue or higher, we don't try to
/// send more data
const unsigned int MAX_BYTES_IN_SEND_QUEUE = 8*1024;

/// Maximum size of a chunk of file being sent
const unsigned int PART_SIZE = 1024;

/// Maximum size to get current working directory
const unsigned int GETCWD_LENGTH = 1024;


/*******************************************************************************
 * SrvContentFile
 ******************************************************************************/
SrvContentFile::SrvContentFile(uint32_t transferID,
			       const std::string& root,
			       const std::string& filename,
			       const std::string& updatekey) :
	mTransferID(transferID),
	mFilename(filename), mUpdateKey(updatekey), mFile(0),
	mSize(0), mCursor(0), mPartsSent(0)
{
	mFilepath = root + "/" + filename;
	openFile();
}

SrvContentFile::~SrvContentFile()
{
	if (mFile) {
		fclose(mFile);
	}
}

bool SrvContentFile::openFile()
{
	if (mFile) {
		return true;
	} else {
		mFile = fopen(mFilepath.c_str(), "rb");
		if (mFile) {
			fseek(mFile, 0, SEEK_END);
			mSize = ftell(mFile);
			fseek(mFile, 0, SEEK_SET);
			return true;
		} else {
			LogERR("Unable to open file: '%s'", mFilepath.c_str());
			return false;
		}
	}
}

bool SrvContentFile::sendPart(const LoginData* player)
{
	if (!mFile) {
		// pretend that finished so stops trying to send parts
		return true;
	}

	MsgContentFilePart msg;
	msg.transferID = mTransferID;

	if (mCursor < mSize) {
		// LogDBG("PART: '%s' (size %u, part %u, cursor %u)", mFilename.c_str(), mSize, mPartsSent, mCursor);

		msg.partNum = mPartsSent;

		// read binary data to memblock
		char memblock[PART_SIZE+1];
		fseek(mFile, mCursor, SEEK_SET);
		msg.size = mSize - mCursor;
		if (msg.size > PART_SIZE)
			msg.size = PART_SIZE;
		int bytesRead = fread(memblock, 1, msg.size, mFile);
		if (bytesRead == -1 || bytesRead != static_cast<int>(msg.size)) {
			LogERR("Error reading file '%s': %s", mFilename.c_str(), strerror(errno));
			return true;
		}

		// copy the data over to the message
		for (size_t i = 0; i < msg.size; ++i) {
			msg.buffer.push_back(memblock[i]);
		}
		SrvNetworkMgr::instance().sendToPlayer(msg, player);

		// update cursors and final cleanup
		mCursor += msg.size;
		++mPartsSent;

		return (mCursor == mSize);
	} else if (mSize == 0) {
		LogWRN("File empty: '%s'", mFilename.c_str());
		msg.partNum = 0;
		msg.size = 0;
		SrvNetworkMgr::instance().sendToPlayer(msg, player);
		return true;
	} else {
		LogERR("Unknown error with the size/parts of a file being sent");
		return false;
	}
}

const char* SrvContentFile::getFilename() const
{
	return mFilename.c_str();
}

const char* SrvContentFile::getUpdateKey() const
{
	return mUpdateKey.c_str();
}

size_t SrvContentFile::getSize() const
{
	return mSize;
}

uint32_t SrvContentFile::getNumberOfParts() const
{
	if ((mSize % PART_SIZE) == 0)
		return (mSize/PART_SIZE);
	else
		return (mSize/PART_SIZE)+1;
}

uint32_t SrvContentFile::getTransferID() const
{
	return mTransferID;
}


/*******************************************************************************
 * SrvContentTransfer
 ******************************************************************************/
SrvContentTransfer::SrvContentTransfer(LoginData* player) :
	mPlayer(player)
{
}

LoginData* SrvContentTransfer::getPlayer() const
{
	return mPlayer;
}

size_t SrvContentTransfer::getNumberOfFiles() const
{
	return mFileList.size();
}

void SrvContentTransfer::addFile(SrvContentFile* file)
{
	mFileList.push_back(file);
}

bool SrvContentTransfer::sendPart()
{
	// send a new part of a file, remove it if it's complete
	SrvContentFile* file = mFileList.back();
	bool finished = file->sendPart(mPlayer);
	if (finished) {
		LogDBG("File finished: %s", file->getFilename());
		delete file;
		mFileList.pop_back();
	}

	if (mFileList.empty()) {
		return true;
	} else {
		return false;
	}
}


/*******************************************************************************
 * SrvContentMgr
 ******************************************************************************/
template <> SrvContentMgr* Singleton<SrvContentMgr>::INSTANCE = 0;

SrvContentMgr::SrvContentMgr() :
	mSerialCounter(0)
{

	// get current working directory
	char path[GETCWD_LENGTH];
	if (getcwd(path, sizeof(path)-1)) {
		mRootDirForOS = path;
	} else {
		LogERR("Couldn't get CWD in the content manager");
		return;
	}
	mRootDirForOS += "/";

	// get the relative path from the CWD of the client content
	string contentDir = ConfigMgr::instance().getConfigVar("Server.Content.ClientContentDir", "-");
	if (contentDir != "-") {
		mRootDirForOS += contentDir;
	} else {
		LogERR("Couldn't get config variable for client content directory");
		return;
	}

	// load the content
	loadContentTree();
}

void SrvContentMgr::finalize()
{
	for (list<SrvContentTransfer*>::iterator it = mTransferList.begin(); it != mTransferList.end(); ++it) {
		SrvContentTransfer* transfer = *it;
		LogNTC("Removing a transfer (IP %s) from the content manager (cleaning up)",
		       transfer->getPlayer()->getIP());
		transfer->getPlayer()->setDownloadingContent(false);
		it = mTransferList.erase(it);
		delete transfer;
	}
}

void SrvContentMgr::removeConnection(LoginData* loginData)
{
	for (list<SrvContentTransfer*>::iterator it = mTransferList.begin(); it != mTransferList.end(); ++it) {
		SrvContentTransfer* transfer = *it;
		if (transfer->getPlayer() == loginData) {
			LogNTC("Removing a transfer (IP %s) from the manager",
			       loginData->getIP());
			loginData->setDownloadingContent(false);
                        it = mTransferList.erase(it);
			delete transfer;
			return;
		}
	}
}

void SrvContentMgr::sendDataToClients()
{
	// mafm: This function is a bit tricky and probably will need to be
	// tweaked in the future; since too much data might flood the client and
	// too few could make it awfully slow.  With the current network code
	// and typical network speeds of today (at the time of writing this) it
	// seems to work quite fine, downloading at 2MBytes/second in local
	// system and 50-100KBytes/s in remote systems without causing packet
	// drops in the network layer or anything; so it works decently.

	if (mTransferList.size() == 0) {
		return;
	}

	// iterating through the transfers (players) to send them data
	for (list<SrvContentTransfer*>::iterator it = mTransferList.begin();
	     it != mTransferList.end(); ++it) {
		SrvContentTransfer* transfer = *it;
		// check that this player has only a few bytes in the queue,
		// otherwise skip
		while (transfer->getPlayer()->getNetlink()->getBytesInSendQueue() < MAX_BYTES_IN_SEND_QUEUE) {
			// put a new part of the file in the queue
			bool finished = transfer->sendPart();
			if (finished) {
				LogNTC("No more files left, removing transfer (IP: %s)",
				       transfer->getPlayer()->getIP());
				transfer->getPlayer()->setDownloadingContent(false);
				delete transfer;
				it = mTransferList.erase(it);
				// break the while loop to go to the next
				// iteration of for()
				break;
			}
		}
	}
}

bool SrvContentMgr::filenameFilter(const string& fileName)
{
	// at the moment we only considere garbage files ending with ~
	if (fileName[fileName.length()-1] == '~'
	    || fileName[0] == '.') {
		return true;
	} else {
		return false;
	}
}

void SrvContentMgr::loadContentTree()
{
	// mafm: This is not the most efficient or portable function in the
	// world, but it's meant to not be tricky and complex.  Symbolic links
	// and other artifacts make that the code to parse everything gets very
	// messy, so I aim for clarity and found another solution: first we
	// consider directories only, always walking forward; and later we
	// revisit the directories and consider only files in them.  The client
	// only cares about files, and will create the directories accordingly,
	// so that's what we need too.

	// setting the dir list to iterate, and pushing the root
	deque<string> dirs;
	deque<string> filesToProcess;
	dirs.push_back(mRootDirForOS);

	// iterating through all the tree for directories only
	while (!dirs.empty()) {
		// consider next directory
		string CWD = dirs.front();
		// LogDBG("Scanning: %s", CWD.c_str());
		dirs.pop_front();
		DIR* dp = opendir(CWD.c_str());
		if (!dp) {
			LogERR("Cannot open dir: '%s'", CWD.c_str());
			continue;
		}

		// read directory contents
		struct dirent* dirp = 0;
		struct stat buf;
		while ((dirp = readdir(dp)) != 0) {
			string filePath = dirp->d_name;
			string fullPath = CWD + "/" + filePath;
			// LogDBG(" -> %s %u", filePath.c_str(), fileType);

			if (filenameFilter(filePath)) {
				continue;
			} else {
				if (stat(fullPath.c_str(), &buf) != 0) {
					LogERR("Can't stat file: %s", filePath.c_str());
					continue;
				}

				if (S_ISDIR(buf.st_mode)) {
					// adding to the dirs to iterate later
					dirs.push_back(CWD + "/" + filePath);
				} else if (S_ISLNK(buf.st_mode)) {
					// if it's a link but we can open it...
					DIR* dp2 = opendir(fullPath.c_str());
					if (dp2 != 0) {
						//.. as a dir, the target is a dir
						// LogDBG("Link is dir: %s", fullPath.c_str());
						dirs.push_back(fullPath);
						// close directory when
						// finished, among other reasons
						// to avoid leaks
						closedir(dp2);
					} else if (fopen(fullPath.c_str(), "r")) {
						// .. as a file and NOT as a
						// dir, the target is a file
						// LogDBG("Link is file: %s", fullPath.c_str());
						filesToProcess.push_back(fullPath);
					}
				} else if (S_ISREG(buf.st_mode)) {
					// regular file
					filesToProcess.push_back(fullPath);
				}
			}
		}
		// close directory when finished, among other reasons to avoid
		// leaks
		closedir(dp);

		// process the files found in the directory
		while (!filesToProcess.empty()) {
			string file = filesToProcess.front();
			// LogDBG("file: %s", file.c_str());
			filesToProcess.pop_front();

			// with regular files, get the data
			struct stat statbuf;
			lstat(file.c_str(), &statbuf);
			string updatekey = StrFmt("%lu-%lu", statbuf.st_size, statbuf.st_mtime);

			// strip the root (including last /)
			file.erase(0, mRootDirForOS.length()+1);

			// push it into the tree
			mContentTree.push_back(NameValuePair(file, updatekey));
		}
	}

	LogNTC("Files in the server content tree: %zu", mContentTree.size());
}

void SrvContentMgr::clearContentTree()
{
	mContentTree.clear();
}

bool SrvContentMgr::reloadContentTree()
{
	if (!mTransferList.empty()) {
		LogWRN("There are active transfers, refusing to reload content tree");
		return false;
	} else {
		clearContentTree();
		loadContentTree();
		return true;
	}
}

void SrvContentMgr::handleQueryFiles(LoginData* loginData,
				     MsgContentQueryUpdate* msg)
{
	// mafm: comparing the files in the server with the ones in the client,
	// if the client doesn't have it or if the update key doesn't match
	// we'll add it to the update message, if the client has it and update
	// key match we ignore it

	// messages which are to be sent to the player
	MsgContentDeleteList msg_delete;
	MsgContentUpdateList msg_update;

	// new transfer
	SrvContentTransfer* transfer = new SrvContentTransfer(loginData);

	// using a hash to retrive the files quickly (it will be compared with
	// the ones in the server)
	map<string, NameValuePair> client_tree;
	for (size_t i = 0; i < msg->filepairs.size(); ++i) {
		client_tree[msg->filepairs[i].name] = msg->filepairs[i];
	}
	map<string, NameValuePair>::iterator client_file;

	// loop though all files of content tree
	for (size_t i = 0; i < mContentTree.size(); ++i) {
		// trying to get the corresponding file in the client
		client_file = client_tree.find(mContentTree[i].name);

		string action;
		if (client_file == client_tree.end()) {
			// add (same result as update)
			action = "A";
		} else if (mContentTree[i].name == (*client_file).second.name
			   && mContentTree[i].value != (*client_file).second.value) {
			// update
			action = "U";
		} else {
			// ignore
			action = "I";
		}
		// LogDBG(" %s %s", action.c_str(), mContentTree[i].filename.c_str());

		// addin or update -> prepare file to be sent and add it to the
		// tranfer (list of files to be sent related with a player)
		if (action == "A" || action == "U") {
			SrvContentFile* file = new SrvContentFile(getNewTransferID(),
								  mRootDirForOS.c_str(),
								  mContentTree[i].name.c_str(),
								  mContentTree[i].value.c_str());
			transfer->addFile(file);
			msg_update.addFile(file->getFilename(),
					   file->getUpdateKey(),
					   file->getTransferID(),
					   file->getNumberOfParts(),
					   file->getSize());
		}
		client_tree.erase(mContentTree[i].name.c_str());
  	}

	// remaining files in the client (not processed because they're not in
	// the server) -> added to delete message
	for (map<string, NameValuePair>::iterator it = client_tree.begin();
	     it != client_tree.end(); ++it) {
		msg_delete.addFile((*it).second.name);
	}
	SrvNetworkMgr::instance().sendToPlayer(msg_delete, loginData);

	// add to the transfer list only if theres something to transmit,
	// otherwise remove the new transfer created
	if (transfer->getNumberOfFiles() > 0) {
		loginData->setDownloadingContent(true);
		LogDBG("Content update for IP='%s': %zu files",
		       loginData->getIP(), transfer->getNumberOfFiles());
		mTransferList.push_back(transfer);
	} else {
		LogDBG("Content update for IP='%s': up-to-date",
		       loginData->getIP());
		delete transfer;
	}
	SrvNetworkMgr::instance().sendToPlayer(msg_update, loginData);
}

uint32_t SrvContentMgr::getNewTransferID()
{
	return ++mSerialCounter;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8

/*
 * cltcontentmgr.cpp
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
#include "client/cltconfig.h"

#include "cltcontentmgr.h"

#include "common/net/msgs.h"

#include "client/net/cltnetmgr.h"

#include <deque>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>
#include <cstdlib>


/// This suffix is used to store a key value in original_filename.suffix, we'll
/// send back the value when asking for updates, and so the server knows whether
/// the file is outdated or not
#define CONTROL_FILE_SUFFIX ".control"


/*******************************************************************************
 * PartialContentFile
 ******************************************************************************/
PartialContentFile::PartialContentFile(const char* filename,
				       const char* updateKey,
				       uint32_t size,
				       uint32_t parts) :
	mFilename(filename), mUpdateKey(updateKey),
	mSize(size), mPartsTotal(parts)
{
	mPartsCurrent = 0;
	mCompleted = false;
}

bool PartialContentFile::addPart(uint32_t numPart,
				 const vector<char>& buffer)
{
	// mafm: the return value means if the file is finished or not (and
	// thus, ready to write it to disk), not always matching with if there
	// was an error or not, so be careful

	// check for several exceptional conditions
	if (mCompleted) {
		LogWRN("File completed, refusing to add more data: '%s'",
		       mFilename.c_str());
		return true;
	} else if ((numPart == 0 && buffer.size() == 0) && mSize == 0) {
		LogWRN("Empty content file sent: %s", mFilename.c_str());
		mCompleted = true;
		return true;
	} else if (numPart != mPartsCurrent) {
		LogWRN("File '%s': received part %u, expected %u, ignoring",
		       mFilename.c_str(), numPart, mPartsCurrent);
		return false;
	} else if (buffer.size() == 0 && mSize != 0) {
		LogWRN("Empty chunk sent, but file size not zero: %s",
		       mFilename.c_str());
		mCompleted = true;
		return true;
	}

	// add the new data to the current one
	for (size_t i = 0; i < buffer.size(); ++i) {
		mData.push_back(buffer[i]);
	}
	++mPartsCurrent;

	// check whether we're finished
	if (mPartsCurrent == mPartsTotal && mData.size() == mSize) {
		mCompleted = true;
		return true;
	} else {
		return false;
	}

	/* mafm: old code left somewhere

	// mafm: we don't calculate the md5sum now so this function doesn't make
	// much sense, it might be better to delete it if we don't make reliable
	// tests

	// calculate the size
	if (mData.size() != mSize) {
		LogERR("Sizes don't match for file '%s':\n"
		       " - %zu (reported by server)\n"
		       " - %zu (data received)",
		       mFilename.c_str(), mSize, mData.size());
		return false;
	} else {
		return true;
	}
	*/
}

bool PartialContentFile::writeToDisk()
{
	string fullpath = StrFmt("%s/%s", CONTENT_DIR, mFilename.c_str());

	try {
		// sanity checks
		if (!isComplete())
			throw "File not valid/complete, refusing to write it";

		// write file
		bool writtenOK = CltContentMgr::writeFileToDisk(fullpath,
								&mData[0],
								mData.size());
		if (!writtenOK)
			throw "Failed to save file to disk";

		// write control file
		string controlFilename(fullpath + CONTROL_FILE_SUFFIX);
		writtenOK = CltContentMgr::writeFileToDisk(fullpath + CONTROL_FILE_SUFFIX,
							   mUpdateKey.data(),
							   mUpdateKey.size());
		if (!writtenOK)
			throw "Failed to save control file to disk";

		// could write the files, finishing...
		LogNTC("Saved file '%s', size '%zu'", fullpath.c_str(), mData.size());
		return true;

	} catch (const char* error) {
		LogERR("%s: '%s'", error, fullpath.c_str());
		return false;
	}
}

const char* PartialContentFile::getFilename() const
{
	return mFilename.c_str();
}

size_t PartialContentFile::getSize() const
{
	return mSize;
}

float PartialContentFile::getProgress() const
{
	// if the size is zero, the progress is 1 (in 0-1 scale)
	if (mSize == 0) {
		return 1.0f;
	} else {
		return static_cast<float>(mData.size())/static_cast<float>(mSize);
	}
}

bool PartialContentFile::isComplete() const
{
	return mCompleted;
}


/*******************************************************************************
 * CltContentMgr
 ******************************************************************************/
template <> CltContentMgr* Singleton<CltContentMgr>::INSTANCE = 0;

CltContentMgr::CltContentMgr()
{
}

CltContentMgr::~CltContentMgr()
{
	while (!mUpdateList.empty()) {
		delete (*mUpdateList.begin()).second;
		mUpdateList.erase(mUpdateList.begin());
	}
}

bool CltContentMgr::filenameFilter(const string& filename, bool verbose)
{
	try {
		if (filename[0] == '/')
			throw "begins with slash (absolute path)";
		else if (filename[0] == '.')
			throw "self directory (avoiding loops)";
		else if (filename.find("..") != filename.npos)
			throw "contains two dots together (could get out of root)";
		else if (filename[filename.length()-1] == '~')
			throw "backup file (*~)";
		else if (filename.find(".control") != filename.npos)
			throw "control file";
	} catch (const char* warning) {
		if (verbose)
			LogWRN("Ignoring suspicious file: %s: '%s'",
			       warning, filename.c_str());
		return false;
	}

	/* mafm: I leave this here, because although we're not using it at the
	 * moment (in particular we don't seend the common root of the content
	 * location to save space in messages), it may be useful as a guide for
	 * other checks

	if (fileName.compare(0, strlen(CONTENT_DIR), CONTENT_DIR))
	{
		LogWRN("'%s' doesn't begin with '%s', skipping",
		       fileName.c_str(), CONTENT_DIR);
		return false;
	}
	*/

	return true;
}

bool CltContentMgr::makeDirsIfNeeded(const string& dirpath)
{
	deque<string> dirs;

	// get each directory in the path
	string current;
	for (size_t i = 0; i < dirpath.length(); ++i) {
		if (dirpath[i] == '/') {
			// LogDBG("Added directory part: '%s'", current.c_str());
			dirs.push_back(current);
			current.clear();
		} else {
			current += dirpath[i];
		}
	}
	// tail
	if (current.length() > 0) {
		dirs.push_back(current);
	}

	/* mafm: debug
	string tmp;
	for (size_t i = 0; i < dirs.size(); ++i) {
		tmp += dirs[i] + "/";
	}
	LogDBG("Directory path parsed: '%s'", tmp.c_str());
	*/

	// we assume that the first directory exists
	string workpath(dirs.front());
	dirs.pop_front();
	while (dirs.size() > 0) {
		// consider next directory
                string subdir = dirs.front();
                // LogDBG("Creating: %s", subdir.c_str());
                dirs.pop_front();

		string fullpath(workpath + "/" + subdir);
                struct stat buf;
		if (stat(fullpath.c_str(), &buf) == 0) {
			if (S_ISDIR(buf.st_mode)) {
				// LogDBG("Already exists '%s'", fullpath.c_str());
				workpath += "/" + subdir;
				continue;
			} else {
				LogERR("Component of the path exists, but not a directory '%s'", fullpath.c_str());
				return false;
			}
		} else {
			int result = mkdir(fullpath.c_str(), 0700);
			if (result != 0) {
				LogERR("Tried but couldn't create '%s'", fullpath.c_str());
				return false;
			} else {
				workpath += "/" + subdir;
				LogNTC("Created directory '%s'", fullpath.c_str());
				continue;
			}
		}
	}

	// LogDBG("Create succesfully dir path '%s'", dirpath.c_str());
	return true;
}

bool CltContentMgr::writeFileToDisk(const std::string& filename,
				    const char* data,
				    size_t size)
{
	try {
		// make directories
		string dirpath = filename.substr(0, filename.rfind('/'));
		bool dirsOK = makeDirsIfNeeded(dirpath);
		if (!dirsOK)
			throw "Directory hierarchy not ready to write the file";

		// write file
		ofstream file;
		file.open(filename.c_str(), ios::out | ios::trunc | ios::binary);
		if (!file.is_open())
			throw "Failed opening the file in write mode";
		file.write(data, size);
		if (file.bad())
			throw "Failed writing the file to the disk";
		file.close();

		// exit successfully
		// LogNTC("Saved file '%s', size '%zu'", filename.c_str(), size);
		return true;

	} catch (const char* error) {
		LogERR("%s: '%s'", error, filename.c_str());
		return false;
	}
}

void CltContentMgr::sendUpdateQuery()
{
	// mafm: this function is very similar in client and server, although
	// not equal, but if there's some important change or improvement here
	// should be also made in the server, and vice-versa

	// make sure that we have root content dir
	bool dirsOK = makeDirsIfNeeded(string(CONTENT_DIR));
	if (!dirsOK) {
		LogERR("Can't scan local content: '%s' doesn't exist nor can be created", CONTENT_DIR);
		return;
	}

	MsgContentQueryUpdate msg;

	deque<string> dirs;
	string root = CONTENT_DIR;
	dirs.push_back(root);

	while (dirs.size() > 0) {
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
                        // LogDBG(" -> %s %u", filePath.c_str(), dirp->d_type);

                        if (!filenameFilter(filePath, false)) {
                                continue;
                        } else if (stat(fullPath.c_str(), &buf) != 0) {
				LogERR("Can't stat file: %s", filePath.c_str());
				continue;
			} else if (S_ISDIR(buf.st_mode)) {
				// adding to the dirs to iterate later
				dirs.push_back(CWD + "/" + filePath);
			} else if (S_ISREG(buf.st_mode)) {
				// process regular files (ignoring links and
				// other UNIX specific type of files)

				// it's a regular file, process it
				string controlFilename = fullPath + CONTROL_FILE_SUFFIX;
				string updateKey;

				ifstream controlFile;
				controlFile.open(controlFilename.c_str());
				if (controlFile.is_open()) {
					getline(controlFile, updateKey);
				} else {
					// update key will be empty
					LogWRN("Couldn't open control: %s", controlFilename.c_str());
				}
				controlFile.close();

				// stripping the root (it's different in client
				// and server, and it's an overhead for the
				// message
				fullPath.replace(0, root.length()+1, "");

				// add it to the message
				msg.addFile(fullPath, updateKey);
                        }
                }
                // close directory when finished, among other reasons to avoid
                // leaks
                closedir(dp);
	}

	/* mafm: debug only
	LogDBG("Send update query with files:");
	for (size_t i = 0; i < msg.filepairs.size(); ++i) {
		LogDBG(" - '%s' '%s'", msg.filepairs[i].filename.c_str(), msg.filepairs[i].updatekey.c_str());
	}
	*/

	// restore status and send the query
	CltNetworkMgr::instance().sendToServer(msg);
}

void CltContentMgr::deleteOutdatedFiles(vector<string> delete_list)
{
	LogNTC("%zu files to delete", delete_list.size());

	// make sure that we have root content dir
	bool dirsOK = makeDirsIfNeeded(string(CONTENT_DIR));
	if (!dirsOK) {
		LogERR("Can't update: '%s' doesn't exist nor can be created", CONTENT_DIR);
		return;
	}

	for (size_t i = 0; i < delete_list.size(); ++i) {
		string fileName = delete_list[i].c_str();

		// using the filter to reject dangerous names
		if (!filenameFilter(fileName, true))
			continue;

		int error = remove(fileName.c_str());
		if (error == 0) {
			LogDBG("  D '%s'", fileName.c_str());
		} else {
			LogERR("Error deleting file '%s'", fileName.c_str());
		}

		// delete the control file (we don't care about errors, it might
		// not exist or whatever
		string controlFilename = fileName + CONTROL_FILE_SUFFIX;
		remove(controlFilename.c_str());
	}
}

void CltContentMgr::addUpdatedFiles(MsgContentUpdateList* msg)
{
	// stats and notifications
	LogNTC("%zu files to update", msg->updateList.size());
	mStats.totalFiles = msg->updateList.size();
	mStats.timeDownloadBegin = time(0);

	// make sure that we have root content dir
	bool dirsOK = makeDirsIfNeeded(string(CONTENT_DIR));
	if (!dirsOK) {
		LogERR("Can't update: '%s' doesn't exist nor can be created", CONTENT_DIR);
		return;
	}

	// process message to create a list of expected files
	for (size_t i = 0; i < msg->updateList.size(); ++i) {
		// have filter into account
		if (!filenameFilter(msg->updateList[i].filename, true))
			continue;

		LogDBG("  U '%s'", msg->updateList[i].filename.c_str());
    
		uint32_t transferID = msg->updateList[i].transferID;
		PartialContentFile* pfile = 0;
		pfile = new PartialContentFile(msg->updateList[i].filename.c_str(),
						 msg->updateList[i].updatekey.c_str(),
						 msg->updateList[i].size,
						 msg->updateList[i].numParts);
		mUpdateList.insert(pair<uint32_t,PartialContentFile*>(transferID, pfile));

		mStats.totalBytes += msg->updateList[i].size;
	}

	// if the server doesn't report anything outdated, we're done
	if (mStats.totalFiles == 0)
		finished();
}

void CltContentMgr::addFilePart(MsgContentFilePart* msg)
{
	// mafm: If failures are frequent, we should provide an automatic way of
	// recovering (trying to download the file again, at least).  With the
	// current content manager and network layers (unlike the old ones) this
	// doesn't seem to happen frequently, so forcing the player to restart
	// the client (in the unlikely case that this happens, never happened so
	// far) is acceptable.

	// get file
	PartialContentFile* pfile = 0;
  	if (mUpdateList.find(msg->transferID) == mUpdateList.end()) {
		LogERR("File part msg doesn't match any expected file, transferID=%u",
			msg->transferID);
		abort();
		return;
	} else {
		pfile = (*mUpdateList.find(msg->transferID)).second;
	}

	// add part
	bool fileFinished = pfile->addPart(msg->partNum, msg->buffer);

	// calculate stats and notify listeners
	if (fileFinished) {
		// update our counters for stats
		++mStats.completedFiles;
		mStats.completedBytes += pfile->getSize();
	}
	float totalProgressPct = static_cast<float>(mStats.completedBytes)
		/ static_cast<float>(mStats.totalBytes);
	float rate = static_cast<float>(mStats.totalBytes)
		/ static_cast<float>(mStats.timeDownloadElapsed);
	rate /= 1024.0f;
	for (list<CltContentListener*>::iterator it = mListenerList.begin(); it != mListenerList.end(); ++it) {
		(*it)->filePartAdded(pfile->getFilename(),
				     pfile->getProgress(),
				     totalProgressPct,
				     rate);
	}

	// we can delete the file now
	if (fileFinished) {
		// write to the filesystem
		bool result = pfile->writeToDisk();
		if (!result)
			return;

		mUpdateList.erase(msg->transferID);
		delete pfile;
	}

	// are we finished?
	if (mUpdateList.empty())
		finished();
}

void CltContentMgr::finished()
{
	// sanity check
	if (mStats.completedFiles != mStats.totalFiles) {
		LogWRN("Completed files (%d) and total files expected to download (%d) mismatch",
		       mStats.completedFiles, mStats.totalFiles);
		return;
	}

	// calculate final statistics
	mStats.timeDownloadElapsed = time(0) - mStats.timeDownloadBegin;
	float rate;
	if (mStats.timeDownloadElapsed == 0)
		rate = 0.0f;
	else {
		rate = static_cast<float>(mStats.totalBytes)
			/ static_cast<float>(mStats.timeDownloadElapsed);
		rate /= 1024.0f;
	}

	// print to log
	string completed = StrFmt("Content update completed, %.02fKB in %us (%.02f KB/s)",
				  static_cast<float>(mStats.totalBytes)/1024.0f,
				  mStats.timeDownloadElapsed,
				  rate);
	LogNTC("%s", completed.c_str());

	// notify listeners
	for (list<CltContentListener*>::iterator it = mListenerList.begin(); it != mListenerList.end(); ++it) {
		(*it)->updateCompleted(static_cast<float>(mStats.totalBytes)/1024.0f,
				       mStats.timeDownloadElapsed,
				       rate);
	}
}

void CltContentMgr::addListener(CltContentListener* listener)
{
	// adding element, not checking for duplicates
	mListenerList.push_back(listener);
}

void CltContentMgr::removeListener(CltContentListener* listener)
{
	// removing element, including duplicates
	mListenerList.remove(listener);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8

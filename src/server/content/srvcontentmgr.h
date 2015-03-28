/*
 * srvcontentmgr.h
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

#ifndef __FEARANN_SERVER_CONTENT_MGR_H__
#define __FEARANN_SERVER_CONTENT_MGR_H__


#include "common/patterns/singleton.h"
#include "common/datatypes.h"

#include <string>
#include <list>
#include <vector>


class MsgContentQueryUpdate;
class LoginData;


/** Represents a file being sent.
 *
 * Using good ol' cstdio instead of newer fstream methods to deal with files,
 * because the compiler was spitting weird errors about templates, parts being
 * private and so forth; preventing to make a clean design in some parts of the
 * manager.
 *
 * @author mafm
 */
class SrvContentFile
{
public:
	/** Constructor with all the needed data */
	SrvContentFile(uint32_t transfer_id,
		       const std::string& root,
		       const std::string& filename,
		       const std::string& updatekey);
	/** Destructor */
	~SrvContentFile();

	/** Send a new chunk of data.  Returns true if the file is fully sent */
	bool sendPart(const LoginData* player);

	/** Returns the filename */
	const char* getFilename() const;
	/** Returns the update key */
	const char* getUpdateKey() const;
	/** Returns the size */
	size_t getSize() const;
	/** Returns number of parts needed */
	uint32_t getNumberOfParts() const;
	/** Returns the transfer ID */
	uint32_t getTransferID() const;

private:
	/// Transfer ID
	uint32_t mTransferID;
	/// The full path of the file
	std::string mFilepath;
	/// The content filename
	std::string mFilename;
	/// The update key
	std::string mUpdateKey;
	/// File in the hard disk
	FILE* mFile;
	/// The total size of the file
	uint32_t mSize;
	/// The position where we read
	uint32_t mCursor;
	/// Number of parts already sent
	uint32_t mPartsSent;


	/** Open the file if it's not opened, and return whether it could be
	 * opened or not. */
	bool openFile();
};


/** Class representing a transfer (a player with a list of files to download)
 *
 * @author mafm
 */
class SrvContentTransfer
{
public:
	/** Constructor */
	SrvContentTransfer(LoginData* p);

	/** Get the player */
	LoginData* getPlayer() const;
	/** Get the number of files */
	size_t getNumberOfFiles() const;
	/** Add file to be transferred. */
	void addFile(SrvContentFile* file);
	/** Send data.  Returning true means that we finished or that there was
	 * an error, the transfer should be removed because it won't be useful
	 * anymore. */
	bool sendPart();

private:
	/// The player
	LoginData* mPlayer;
	/// The list of files to send
	std::vector<SrvContentFile*> mFileList;
};


/** The server part of the content manager: it receives the content listing from
 * the player (pair of file:updatekey) and sends the data needed
 *
 * @author mafm
 */
class SrvContentMgr : public Singleton<SrvContentMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts
	 * down. */
	void finalize();

	/** Reload the content tree */
	bool reloadContentTree();
	/** Send some more data to clients */
	void sendDataToClients();
	/** Remove a player connection, not sending more data to it */
	void removeConnection(LoginData* loginData);
	/** Handle a message querying for content updates */
	void handleQueryFiles(LoginData* player, 
			      MsgContentQueryUpdate* msg);

private:
	/** Singleton friend access */
	friend class Singleton<SrvContentMgr>;


	/// Contains all the files currently being sent
	std::vector<NameValuePair> mContentTree;
	/// The transfers -- using a list to have valid iterators after removing
	/// elements
	std::list<SrvContentTransfer*> mTransferList;
	/// Maximum transfer id currently allocated
	uint32_t mSerialCounter;
	/// Root real directory
	std::string mRootDirForOS;

	/** Return true if we don't want this file to be considered */
	bool filenameFilter(const std::string& filename);
	/** Load the content tree */
	void loadContentTree();
	/** Clear the content tree */
	void clearContentTree();
	/** Allocate and return a new ID */
	uint32_t getNewTransferID();


	/** Default constructor */
	SrvContentMgr();
};


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8

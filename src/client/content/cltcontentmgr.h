/*
 * cltcontentmgr.h
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

#ifndef __FEARANN_CLIENT_CONTENT_MGR_H__
#define __FEARANN_CLIENT_CONTENT_MGR_H__


#include "common/patterns/singleton.h"


#include <map>
#include <vector>
#include <list>


class MsgContentFilePart;
class MsgContentUpdateList;


/** Listener for the events of the client content manager.
 *
 * @author mafm
 */
class CltContentListener
{
public:
	/** Notification that a part of a file was added (percentages in 0-1
	 * range, so file is completed when filePct = 1.0).  The average speed
	 * is in kBytes/s. */
	virtual void filePartAdded(const char* filename,
				   float filePct,
				   float totalPct,
				   float avgSpeed) = 0;
	/** Notification that the update process was completed. */
	virtual void updateCompleted(float sizeKB,
				     float seconds,
				     float avgSpeed) = 0;
	/** Default destructor */
	virtual ~CltContentListener() { }
};


/** Represents an incoming file (content update in progress).
 *
 * Server sends the data in chunks, typically about 1KB for each part, and we
 * use this class to store the data before writing it on disk when completed.
 *
 * @author mafm
 */
class PartialContentFile
{
public:
	/** Constructor
	 *
	 * @param filename Name of the file
	 *
	 * @param updateKey Key as checksum to know when to update
	 *
	 * @param size Size (in bytes) of the file
	 *
	 * @param parts Number of chunks to be sent
	 */
	PartialContentFile(const char* filename,
			   const char* updateKey,
			   uint32_t size,
			   uint32_t parts);

	/** Add the raw data of the given part to the data that we have so far.
	 *
	 * \returns true if file has been completed, false otherwise
	 */
	bool addPart(uint32_t numPart,
		     const std::vector<char>& buffer);
	/** If everything is OK saves the file to the disk.  If false is
	 * returned, we need to ask the server to resend us the file.  In all
	 * cases, the partial content file needs to be deleted. */
	bool writeToDisk();
	/** Get the filename of the file */
	const char* getFilename() const;
	/** Get the file size */
	size_t getSize() const;
	/** Get progress (in the range 0-1) */
	float getProgress() const;
	/** It checks the content received with the one provided by the
	 * server. */
	bool isComplete() const;

private:
	/// File name
	std::string mFilename;
	/// Update key given by the server
	std::string mUpdateKey;
	/// Size (sent by server, so we know what we should expect)
	size_t mSize;
	/// Number of parts
	uint32_t mPartsTotal;

	/// Raw data of the file
	std::vector<char> mData;
	/// Number of parts currently retrieved
	uint32_t mPartsCurrent;
	/// Completed (whether is complete or not, so we won't allow to write
	/// more, etc)
	bool mCompleted;
};


/** Governs the content updates after connection and before joining the game,
 * sending the update requests and getting the files back.
 *
 * @author mafm
 */
class CltContentMgr : public Singleton<CltContentMgr>
{
public:
	/** Saves a file to the disk.
	 *
	 * \returns false when the contents can be written, whatever the reason
	 * (saves information in the log about that). */
	static bool writeFileToDisk(const std::string& filename,
				    const char* data,
				    size_t size);

	/** Send a query to the server to update the content */
	void sendUpdateQuery();

	/** Purge file tree from outdated files.  This must be called when
	 * receiving the message from the server, telling us which files to
	 * remove. */
	void deleteOutdatedFiles(std::vector<std::string> delete_list);

	/** Handles the message asking to update files */
	void addUpdatedFiles(MsgContentUpdateList* msg);

	/** Handles the message containing a chunk of a file */
	void addFilePart(MsgContentFilePart* msg);

	/** Add a listener for events */
	void addListener(CltContentListener* listener);
	/** Remove a listener */
	void removeListener(CltContentListener* listener);

private:
	/** Singleton friend access */
	friend class Singleton<CltContentMgr>;

	/// List of outdated or non-existent files, that the server is going to
	/// send to us
	std::map<uint32_t, PartialContentFile*> mUpdateList;

	/// List of listeners subscribed to our events
	std::list<CltContentListener*> mListenerList;

	/** Statistics for this class */
	class Statistics {
	public:
		Statistics() {
			totalFiles = 0; completedFiles = 0;
			totalBytes = 0; completedBytes = 0;
			timeDownloadBegin = 0; timeDownloadElapsed = 0;
		}
		/// Total files to download
		uint32_t totalFiles;
		/// Total files completed
		uint32_t completedFiles;
		/// Total kbytes (not accurate, but helps the download window to
		/// be more accurate than estimating based on files)
		uint32_t totalBytes;
		/// Completed kbytes (not accurate)
		uint32_t completedBytes;
		/// Aux variable to help to calculate the rate
		uint32_t timeDownloadBegin;
		/// Aux variable to help to calculate the rate
		uint32_t timeDownloadElapsed;
	} mStats;


	/** Default constructor */
	CltContentMgr();
	/** Destructor */
	~CltContentMgr();

	/** Make the dir hierarchy given, if non-existant and possible.
	 *
	 * \returns false when there's an error that prevents to make the dirs
	 * (or check whether they exist). */
	static bool makeDirsIfNeeded(const std::string& dirpath);

	/** Filter to check for potentially dangerous file (generated in servers
	 * by error or maliciously).
	 * 
	 * @param verbose means wether to echo the file as a log message or not:
	 * it should be done for incoming files from the server, not when
	 * scaning local files to send the update.
	 */
	bool filenameFilter(const std::string& filename, bool verbose);

	/** Performs the actions necessary to finish */
	void finished();
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

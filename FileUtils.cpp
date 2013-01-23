/*
 * FileUtils.cpp
 *
 *  Created on: 10 juil. 2012
 *      Author: sylvain
 */

#include "FileUtils.h"
#include "FileEncoding.h"
#include "Unitex-C++/Unicode.h"
#include "Unitex-C++/Buffer.h"
#include "Unitex-C++/Af_stdio.h"
#include "Utils.h"
#include <sstream>

using namespace std;
using namespace uima;
using namespace boost::filesystem;
using namespace unitex;

namespace unitexcpp {

	/**
	 * Writes a string to a file, converting the ICU Unicode string into something
	 * understandable by Unitex.
	 * The file may be "regular" or abstract (i.e. starting with "$:").
	 */
	bool writeStringToFile(const string& strFilename, const UnicodeStringRef& uString)
	{
		VersatileEncodingConfig cfg = VEC_DEFAULT;
		U_FILE* f = u_fopen(&cfg, strFilename.c_str(), U_WRITE);
		if (f == NULL)
			return false;

		int32_t length = uString.length();
		unichar* uBuffer = new unichar[length + 1];
		for (int32_t i = 0; i < length; i++)
			uBuffer[i] = uString.charAt(i);
		uBuffer[length] = 0;

		u_fwrite(uBuffer, length, f);

		delete[] uBuffer;

		u_fclose(f);
		return true;
	}

	bool writeStringToFile(const path& filename, const UnicodeStringRef& uString)
	{
		return writeStringToFile(filename.string(), uString);
	}

	bool getStringFromFile(const string& strFilename, UnicodeString& uString)
	{
		if (!isPersistedPath(strFilename) && !exists(path(strFilename)))
			return false;

		VersatileEncodingConfig cfg = VEC_DEFAULT;
		U_FILE* f = u_fopen(&cfg, strFilename.c_str(), U_READ);
		buffer* contents = new_buffer_for_file(UNICHAR_BUFFER, f, 0);
		int ok;
		contents->size = u_fread(contents->unichar_buffer, contents->MAXIMUM_BUFFER_SIZE, f, &ok);
		u_fclose(f);

		if (!ok)
			return false;

		uString.remove();
		for (int i = 0; i < contents->size; i++)
			uString.append((UChar)contents->unichar_buffer[i]);
		uString.append(0);
		free_buffer(contents);

		return true;
	}

	bool getStringFromFile(const path& filename, UnicodeString& uString)
	{
		return getStringFromFile(filename.string(), uString);
	}

	path getRelativePathFrom(const path& rootPath, const path& fullPath)
	{
		path offsetPath, currentPath = system_complete(fullPath), stopPath = system_complete(rootPath);

		path::iterator fullIt = currentPath.begin();
		for (path::iterator rootIt = stopPath.begin(); (rootIt != stopPath.end()) && (fullIt != currentPath.end()); ++rootIt, ++fullIt)
			;
		if (fullIt == currentPath.end()) {
			ostringstream oss;
			oss << "Path " << fullPath << " is not a leaf of " << rootPath;
			throw filesystem_error(oss.str(), fullPath, rootPath, boost::system::error_code());
		}

		while (fullIt != currentPath.end())
			offsetPath = offsetPath / *fullIt;

		return offsetPath;
	}

	path createPathRelativeTo(const path& rootPath, const path& offsetPath)
	{
		path fullPath = system_complete(rootPath);
		for (path::iterator it = offsetPath.begin(); it != offsetPath.end(); it++)
			fullPath = fullPath / *it;
		return fullPath;
	}

	/**
	 * Fills a string list with the paths of all virtual files contained in a virtual directory.
	 */
	void getVirtualFilesInDirectory(const string& strDirectory, list<string>& list)
	{
		list.clear();
		char** filenames = af_get_list_file(strDirectory.c_str());
		if (filenames != NULL) {
			int iter = 0;
			while ((*(filenames + iter)) != NULL)
				list.push_back(*(filenames + iter++));
		}
		af_release_list_file(strDirectory.c_str(), filenames);
	}

	/**
	 * Fills a string list with the paths of all virtual files contained in a virtual directory.
	 */
	void getVirtualFilesInDirectory(const path& directory, list<string>& list)
	{
		getVirtualFilesInDirectory(directory.string(), list);
	}

}



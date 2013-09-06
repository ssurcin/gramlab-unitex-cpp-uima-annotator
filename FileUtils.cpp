/*
* FileUtils.cpp
*
*  Created on: 10 juil. 2012
*      Author: sylvain
*/

#include "FileUtils.h"
#include "FileEncoding.h"
#include "Unitex-C++/UnitexLibIO.h"
#include "Unitex-C++/File.h"
#include "Unitex-C++/Unicode.h"
#include "Unitex-C++/Buffer.h"
#include "Unitex-C++/Af_stdio.h"
#include "Utils.h"
#include "UnitexAnnotatorCpp.h"
#include <sstream>

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace uima;
using namespace boost::filesystem;
using namespace unitex;

namespace unitexcpp {

	// A unique instance to represent a file not found
	static const path fileNotFoundPath;

	// A static variable to store only once the VFS prefix
	static string VfsPrefix;
	static bool VfsPrefixInitialized = false;

	string const& getVirtualFilePfx()
	{
		if (!VfsPrefixInitialized) {
			if (UnitexAbstractPathExists("*") != 0)
				VfsPrefix = "*";
			else if (UnitexAbstractPathExists("$:") != 0)
				VfsPrefix= "$:";
			VfsPrefixInitialized = true;
		}
		return VfsPrefix;
	}

	bool isAbsolutePath(const path& path)
	{
		return ::unitex::is_absolute_path(path.string().c_str());
	}

	/**
	* Tests if a path starts with the native Unitex persistence mark.
	*/
	bool isVirtualPath(const path& aPath)
	{
		return boost::starts_with(aPath.string(), VfsPrefix);
	}

	/**
	* Generates and return a new path with the native Unitex persistence mark.
	* If the path already has the mark, returns a copy.
	*/
	path virtualizedPath(const path& aPath)
	{
		if (!isVirtualPath(aPath))
			return path(VfsPrefix + aPath.string());
		return aPath;
	}

	/**
	* Generates and return a new path without the native Unitex persistence mark.
	* If the path already has no mark, returns a copy.
	*/
	path unvirtualizedPath(const path& aPath)
	{
		if (isVirtualPath(aPath)) {
			path p = aPath;
			do {
				p = path(p.string().substr(VfsPrefix.length()));
			} while (isVirtualPath(p));
			return p;
		}
		return aPath;
	}

	string quotePath(const path& path)
	{
		ostringstream oss;
		oss << path;
		return oss.str();
	}

	/// <summary>
	/// Extends UnitexLibIO function to accept boost paths as arguments.
	/// </summary>
	/// <remarks>Do something only if old and new filenames differ.</remarks>
	/// <param name='oldName'>The source file name.</param>
	/// <param name='newName'>The destination file name.</param>
	/// <returns>True if ok, false if failed.</returns>
	bool copyUnitexFile(path const& oldName, path const& newName)
	{
		return (oldName != newName) ? (CopyUnitexFile(oldName.string().c_str(), newName.string().c_str()) == 0) : true;
	}

	/// <summary>
	/// Extends UnitexLibIO function to accept boost path.
	/// </summary>
	/// <param name='fileName'>The path of the destination file.</param>
	/// <param name='buffer'>The buffer to write to file.</param>
	/// <param name='bufferSize'>The buffer's size in bytes.</param>
	/// <returns>True if ok, false if failed.</returns> 
	bool writeUnitexFile(path const& fileName, const void* buffer, size_t bufferSize)
	{
		return WriteUnitexFile(fileName.string().c_str(), buffer, bufferSize, NULL, 0) == 0;
	}

	/// <summary>
	/// Creates a virtual file containing the contents of a Unicode string.
	/// </summary>
	/// <param name='fileName'>The destination file name.</param>
	/// <param name='uString'>A Unicode string.</param>
	/// <returns>True if ok, false if failed.</returns>
	bool writeUnitexFile(path const& fileName, UnicodeStringRef const& uString)
	{
		int32_t uLength = uString.length();
		unichar* uBuffer = new unichar[uLength + 1];
		for (int32_t i = 0; i < uLength; i++)
			uBuffer[i] = uString.charAt(i);
		uBuffer[uLength] = 0;

		const void* buffer = (const void*)uBuffer;
		size_t length = uLength * sizeof(unichar);

		bool result = writeUnitexFile(fileName, uBuffer, length);

		delete[] uBuffer;

		return result;
	}

	/// <summary>
	/// Gets the whole contents of a (virtual) file into a Unicode string.
	/// </summary>
	/// <param name='fileName'>The file name.</param>
	/// <param name='uString'>The Unicode string where to store the file contents.</param>
	/// <returns>True if ok, false if failed.</returns>
	bool getStringFromUnitexFile(path const& fileName, UnicodeString& uString)
	{
		uString.remove();

		if (!isVirtualPath(fileName) && !exists(fileName))return false;

		UNITEXFILEMAPPED* pFileHandle;
		const void* buffer = NULL;
		size_t bufferSize = 0;
		GetUnitexFileReadBuffer(fileName.string().c_str(), &pFileHandle, &buffer, &bufferSize);
		if (buffer != NULL) {
			if (bufferSize > 0) {
				const unsigned char* foo = (const unsigned char*)buffer;
				const UChar* unicharBuffer = (const UChar*)buffer;
				size_t uSize = bufferSize / U_SIZEOF_UCHAR;

				UChar max = 0;
				for (size_t i = 0; i < uSize; i++) {
					UChar uchar = unicharBuffer[i];
					//cout << "uBuffer[" << i << "] = " << unicharBuffer[i] << endl;
					if (uchar > max) max = uchar;
					uString.append(uchar);
				}
				uString.append(0);
			}
			CloseUnitexFileReadBuffer(pFileHandle, buffer, bufferSize);
			return true;
		}
		return false;
	}

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
		uString.remove();

		if (!isVirtualPath(strFilename) && !exists(path(strFilename)))
			return false;

		buffer* contents;

		VersatileEncodingConfig cfg = VEC_DEFAULT;
		U_FILE* f = u_fopen(&cfg, strFilename.c_str(), U_READ);
		contents = new_buffer_for_file(UNICHAR_BUFFER, f, 0);
		int ok;
		contents->size = u_fread(contents->unichar_buffer, contents->MAXIMUM_BUFFER_SIZE, f, &ok);
		u_fclose(f);

		if (!ok)
			return false;

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



/*
* FileUtils.h
*
*  Created on: 10 juil. 2012
*      Author: sylvain
*/

#ifndef FILEUTILS_H_
#define FILEUTILS_H_

#include <boost/filesystem.hpp>

#include <uima/unistrref.hpp>
#include <unicode/ustring.h>

#undef u_fopen
#undef u_fread
#undef u_fwrite
#undef u_fclose
#undef u_feof
#undef u_fgetc

namespace unitexcpp
{
	extern const boost::filesystem::path fileNotFoundPath;

	std::string const& getVirtualFilePfx();

	bool isAbsolutePath(const boost::filesystem::path& path);
	bool isVirtualPath(const boost::filesystem::path& aPath);
	boost::filesystem::path virtualizedPath(const boost::filesystem::path& aPath);
	boost::filesystem::path unvirtualizedPath(const boost::filesystem::path& aPath);
	std::string quotePath(const boost::filesystem::path& path);

	bool copyUnitexFile(boost::filesystem::path const& oldName, boost::filesystem::path const& newName);
	bool writeUnitexFile(boost::filesystem::path const& fileName, const void* buffer, size_t bufferSize);
	bool writeUnitexFile(boost::filesystem::path const& fileName, const uima::UnicodeStringRef& uString);
	bool getUnicodeStringFromUnitexFile(boost::filesystem::path const& fileName, icu::UnicodeString& uString);

	void getVirtualFilesInDirectory(const std::string& strDirectory, std::list<std::string>& list);
	void getVirtualFilesInDirectory(const boost::filesystem::path& directory, std::list<std::string>& list);

	boost::filesystem::path getRelativePathFrom(const boost::filesystem::path& rootPath, const boost::filesystem::path& fullPath);
	boost::filesystem::path createPathRelativeTo(const boost::filesystem::path& rootPath, const boost::filesystem::path& offsetPath);

	void emptyVirtualFileSystem();
}

#endif /* FILEUTILS_H_ */

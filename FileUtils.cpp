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
		UChar uBom = 0xfeff;

		const UChar * uBuffer = uString.getBuffer();
		int32_t uLength = uString.length();

		bool result = WriteUnitexFile(fileName.string().c_str(), &uBom, sizeof(UChar), uBuffer, uLength * sizeof(UChar)) == 0;

		return result;
	}

	//bool writeUnitexFileFastWithBOM(path const& fileName, UnicodeStringRef const& uString)
	//{
	//	int32_t uLength = uString.length();
	//	unsigned char* rawBuffer = new unsigned char[(uLength + 2) * U_SIZEOF_WCHAR_T];
	//	UChar * uBuffer = (UChar*) rawBuffer;
	//	*uBuffer = 0xfeff;
	//	UErrorCode errorCode = U_ZERO_ERROR ;
	//	uString.extract(uBuffer+1,uLength+1,errorCode);

	//	size_t length = (uLength+1) * U_SIZEOF_WCHAR_T;

	//	bool result = writeUnitexFile(fileName, rawBuffer, length);

	//	delete[] rawBuffer;

	//	return result;
	//}

#define GetUtf8_Size(ch)  \
	(((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? 1 : \
	(((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? 2 : \
	(((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? 3 : \
	(((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? 4 : \
	(((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? 5 : \
	(((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? 6 : 001))))))


#define GetUtf8_Mask(ch)  \
	(((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? ((unsigned char)0x7f) : \
	(((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? ((unsigned char)0x1f) : \
	(((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? ((unsigned char)0x0f) : \
	(((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? ((unsigned char)0x07) : \
	(((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? ((unsigned char)0x03) : \
	(((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? ((unsigned char)0x01) : 0))))))

	static size_t unpack_utf8_string(UChar*write_content_walk_buf,size_t nb_unichar_alloc_walk,size_t * p_size_this_string_written,
		const unsigned char*src_walk,size_t buf_size)
	{
		size_t size_this_string_written=0;
		size_t nb_pack_read=0;
		for (;;)
		{
			if ((src_walk==NULL) || (buf_size==0))
			{				
				if (p_size_this_string_written!=NULL)
					*p_size_this_string_written = size_this_string_written;
				return 0;
			}
			unsigned char ch = *(src_walk++);
			buf_size--;
			nb_pack_read++;

			UChar c;



			if ((ch&0x80) == 0)
			{
				c=ch;
			}
			else
			{
				c=ch & GetUtf8_Mask(ch);
				int nbbyte=GetUtf8_Size(ch);
				if (((int)buf_size)+1 < nbbyte)
				{
					if (p_size_this_string_written!=NULL)
						*p_size_this_string_written = size_this_string_written;
					return 0;
				}

				for(;;)
				{
					nbbyte--;
					if (nbbyte==0)
						break;

					c = (c<<6) | ( (*(src_walk++)) & 0x3F);
					buf_size--;
					nb_pack_read++;
				}
			}

			if ((write_content_walk_buf!=NULL) && (size_this_string_written<nb_unichar_alloc_walk))
				*(write_content_walk_buf + size_this_string_written)=c;
			size_this_string_written++;

			if (c==0)
			{
				if (p_size_this_string_written!=NULL)
					*p_size_this_string_written = size_this_string_written;
				return nb_pack_read;
			}
		}
	}

	///// <summary>
	///// Gets the whole contents of a (virtual) file into a Unicode string.
	///// </summary>
	///// <param name='fileName'>The file name.</param>
	///// <param name='uString'>The Unicode string where to store the file contents.</param>
	///// <returns>True if ok, false if failed.</returns>
	//bool getStringFromUnitexFile(path const& fileName, UnicodeString& uString)
	//{
	//	uString.remove();

	//	if (!isVirtualPath(fileName) && !exists(fileName))return false;

	//	UNITEXFILEMAPPED* pFileHandle;
	//	const void* buffer = NULL;
	//	size_t bufferSize = 0;
	//	GetUnitexFileReadBuffer(fileName.string().c_str(), &pFileHandle, &buffer, &bufferSize);

	//	if (buffer != NULL) {
	//		if (bufferSize > 0) {
	//			const unsigned char* bufchar= (const unsigned char*) buffer;
	//			size_t size_bom = 0;
	//			bool is_utf16_native_endianess = false;
	//			bool is_utf16_swap_endianess = false;

	//			if (bufferSize > 1) {
	//				if (((*(bufchar)) == 0xff) && ((*(bufchar + 1)) == 0xfe))
	//				{
	//					// little endian
	//					is_utf16_native_endianess = is_little_endian();
	//					is_utf16_swap_endianess = ! is_utf16_native_endianess;
	//					size_bom = 2;
	//				}
	//			}

	//			if (bufferSize > 1) {
	//				if (((*(bufchar)) == 0xfe) && ((*(bufchar + 1)) == 0xff))
	//				{
	//					// big endian
	//					is_utf16_native_endianess = ! is_little_endian();
	//					is_utf16_swap_endianess = ! is_utf16_native_endianess;
	//					size_bom = 2;
	//				}
	//			}

	//			if (bufferSize > 2) {
	//				if (((*(bufchar)) == 0xef) && ((*(bufchar + 1)) == 0xbb) && ((*(bufchar + 2)) == 0xbf))
	//				{
	//					size_bom = 3;
	//				}
	//			}

	//			if (is_utf16_native_endianess)
	//			{
	//				const UChar* uBuffer = (const UChar*)(bufchar + size_bom);
	//				size_t uSize = (bufferSize - size_bom) / U_SIZEOF_UCHAR;
	//				uString.setTo(uBuffer, uSize);
	//			}
	//			else if (is_utf16_swap_endianess)
	//			{
	//				unsigned char* returnedUTF16buffer = (unsigned char*) malloc(bufferSize);
	//				if (returnedUTF16buffer != NULL)
	//				{
	//					for (size_t i = 0; i<bufferSize; i += 2)
	//					{
	//						unsigned char c1 = *(bufchar + i);
	//						unsigned char c2 = *(bufchar + i + 1);
	//						*(returnedUTF16buffer + i) = c2;
	//						*(returnedUTF16buffer + i + 1) = c1;
	//					}
	//					const UChar* uBuffer = (const UChar*)(returnedUTF16buffer + size_bom);
	//					size_t uSize = (bufferSize - size_bom) / U_SIZEOF_UCHAR;
	//					uString.setTo(uBuffer, uSize);
	//					free(returnedUTF16buffer);
	//				}
	//			}
	//			else
	//			{
	//				char* stringUtf = (char*) malloc(bufferSize + 1);
	//				memcpy(stringUtf, bufchar + size_bom, bufferSize - size_bom);
	//				*(stringUtf + bufferSize - size_bom) = '\0';
	//				uString = UnicodeString(stringUtf);
	//				free(stringUtf);
	//			}
	//		}
	//		CloseUnitexFileReadBuffer(pFileHandle, buffer, bufferSize);
	//		return true;
	//	}

	//	//if (buffer != NULL) {

	//	//	bool isUtf16 = false;
	//	//	
	//	//	if (bufferSize > 0) {
	//	//		const UChar* browseBuffer = (const UChar*)buffer;
	//	//		size_t bufferSizeWithoutBom = bufferSize;
	//	//		if (bufferSize >= sizeof(UChar))
	//	//			if ((*browseBuffer) == 0xfeff)
	//	//			{
	//	//				browseBuffer += 1;
	//	//				bufferSizeWithoutBom -= sizeof(UChar);
	//	//				isUtf16 = true;
	//	//			}
	//	//			uString.setTo(browseBuffer,(bufferSizeWithoutBom/sizeof(UChar)));
	//	//	}

	//	//	if (!isUtf16)
	//	//	{
	//	//		size_t buffer_alloc_size = (bufferSize + 1) * 2;
	//	//		UChar* outBuffer = new UChar[buffer_alloc_size];
	//	//		size_t size_this_string_written = 0;
	//	//		unpack_utf8_string(outBuffer,buffer_alloc_size,&size_this_string_written,
	//	//			(const unsigned char*)buffer,bufferSize);
	//	//		uString.setTo(outBuffer,size_this_string_written);
	//	//		delete[] outBuffer;
	//	//	}

	//	//	CloseUnitexFileReadBuffer(pFileHandle, buffer, bufferSize);
	//	//	return true;
	//	//}

	//	return false;
	//}

	bool getUnicodeStringFromUnitexFile(path const& fileName, UnicodeString& uString)
	{
		if (!exists(fileName)) return false;

		uString.remove();

		UNITEXFILEMAPPED* pFileHandle;
		const void* buffer = NULL;
		size_t bufferSize = 0;
		GetUnitexFileReadBuffer(fileName.string().c_str(), &pFileHandle, &buffer, &bufferSize);

		if (pFileHandle != NULL) {
			if (bufferSize > 0) {
				const unsigned char* bufchar= (const unsigned char*) buffer;
				size_t size_bom = 0;
				bool is_utf16_native_endianess = false;
				bool is_utf16_swap_endianess = false;

				if (bufferSize > 1) {
					UChar UTF16Bom = *((const UChar*)buffer);

					if (UTF16Bom == 0xfeff)
					{
						// native endian
						is_utf16_native_endianess = true;
						size_bom = 2;
					}

					if (UTF16Bom == 0xfffe)
					{
						// reverse endian
						is_utf16_swap_endianess = true;
						size_bom = 2;
					}
				}


				if (bufferSize > 2) {
					if (((*(bufchar)) == 0xef) && ((*(bufchar + 1)) == 0xbb) && ((*(bufchar + 2)) == 0xbf))
					{
						size_bom = 3;
					}
				}

				if (is_utf16_native_endianess)
				{
					const UChar* uBuffer = (const UChar*)(bufchar + size_bom);
					size_t uSize = (bufferSize - size_bom) / U_SIZEOF_UCHAR;
					uString.setTo(uBuffer, uSize);
				}
				else if (is_utf16_swap_endianess)
				{
					unsigned char* returnedUTF16buffer = new unsigned char [bufferSize];
					if (returnedUTF16buffer != NULL)
					{
						for (size_t i = 0; i<bufferSize; i += 2)
						{
							unsigned char c1 = *(bufchar + i);
							unsigned char c2 = *(bufchar + i + 1);
							*(returnedUTF16buffer + i) = c2;
							*(returnedUTF16buffer + i + 1) = c1;
						}
						const UChar* uBuffer = (const UChar*)(returnedUTF16buffer + size_bom);
						size_t uSize = (bufferSize - size_bom) / U_SIZEOF_UCHAR;
						uString.setTo(uBuffer, uSize);
						delete [] returnedUTF16buffer;
					}
				}
				else
				{
					size_t len_buf_UChar = bufferSize+1+1;
					UChar* stringUChar = new UChar[len_buf_UChar + 1];
				

					size_t nb_written = 0;
					unpack_utf8_string(stringUChar,len_buf_UChar,&nb_written,bufchar + size_bom, bufferSize - size_bom);
		

					uString.setTo((const UChar*)stringUChar, nb_written);
					delete [] stringUChar;
				}
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

	//bool getStringFromFile(const string& strFilename, UnicodeString& uString)
	//{
	//	uString.remove();

	//	if (!isVirtualPath(strFilename) && !exists(path(strFilename)))
	//		return false;

	//	buffer* contents;

	//	VersatileEncodingConfig cfg = VEC_DEFAULT;
	//	U_FILE* f = u_fopen(&cfg, strFilename.c_str(), U_READ);
	//	contents = new_buffer_for_file(UNICHAR_BUFFER, f, 0);
	//	int ok;
	//	contents->size = u_fread(contents->unichar_buffer, contents->MAXIMUM_BUFFER_SIZE, f, &ok);
	//	u_fclose(f);

	//	if (!ok)
	//		return false;

	//	uString.setTo((const UChar*)contents->unichar_buffer, contents->size / U_SIZEOF_UCHAR);
	//	//for (int i = 0; i < contents->size; i++)
	//	//	uString.append((UChar)contents->unichar_buffer[i]);
	//	//uString.append(0);
	//	free_buffer(contents);

	//	return true;
	//}

	//bool getStringFromFile(const path& filename, UnicodeString& uString)
	//{
	//	return getStringFromFile(filename.string(), uString);
	//}

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



#include "Utils.h"
#if (defined(_WIN32) || defined(_WIN64))
#define WINDOWS
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "FileUtils.h"

// ICU
#include <unicode/ustream.h>
#include <unicode/ustdio.h>
#include <unicode/regex.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <list>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "UnitexException.h"

// Xerces-C++
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

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
using namespace icu;
using namespace uima;
using namespace boost::filesystem;
using namespace boost::algorithm;
using namespace xercesc;

///////////////////////////////////////////////////////////////////////////////
//
// Native Unitex persistence tools
//
///////////////////////////////////////////////////////////////////////////////

// Use the adequate VFS prefix depending on the preprocessor symbol VFSUFO 
// defined for the project or not.
const string UNITEX_VIRTUAL_PATH_PREFIX = unitexcpp::getVirtualFilePfx();

/**
* Gets the value of an environment variable.
* Returns boost::none if the variable is not defined.
*/
boost::optional<string> get_env(const string& varname)
{
	char* var = getenv(varname.c_str());
	if (var)
		return string(var);
	return boost::none;
}

///////////////////////////////////////////////////////////////////////////////
//
// String utilities
//
///////////////////////////////////////////////////////////////////////////////

static const UnicodeString whiteSpaces = UnicodeString(0x0020) + UnicodeString(0x00a0) + UnicodeString(0x2000) + UnicodeString(0x2001) + UnicodeString(0x2002) + UnicodeString(0x2003)
	+ UnicodeString(0x2004) + UnicodeString(0x2005) + UnicodeString(0x2006) + UnicodeString(0x2007) + UnicodeString(0x2008) + UnicodeString(0x2009) + UnicodeString(0x200a) + UnicodeString(0x200b)
	+ UnicodeString(0x202f) + UnicodeString(0x205f) + UnicodeString(0x3000) + UnicodeString(0xfeff);
//"\u0020\u00a0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u200b\u202f\u205f\u3000\ufeff";

/**
* An external version of isEmpty to be used as a predicate by STL algorithms.
*/
bool isEmpty(const UnicodeString& string)
{
	return string.isEmpty();
}

bool isBlank(const UnicodeString& string)
{
	int32_t pos = 0, len = string.length();
	while (pos < len) {
		UChar32 c = string.char32At(pos);
		if (c && (!u_isUWhiteSpace(c)) && (whiteSpaces.indexOf(c) < 0))
			return false;
		pos++;
	}
	return true;
}

/**
* True only if the string is CR+LF.
*/
bool isCRLF(const UnicodeString& string)
{
	return ((string.length() == 2) && (string.charAt(0) == '\r') && (string.charAt(1) == '\n'));
}

/**
* True only if the string is CR+LF.
*/
bool isCRLF(const UnicodeStringRef string)
{
	return ((string.length() == 2) && (string.charAt(0) == '\r') && (string.charAt(1) == '\n'));
}

/**
* True if the string has the CR+LF sequence at the indicated position.
*/
bool hasCRLF(const UnicodeString& string, int32_t pos)
{
	if ((pos >= 0) && (pos < string.length() - 1))
		return (string.charAt(pos) == '\r') && (string.charAt(1) == '\n');
	return false;
}

/**
* True if the string has the CR+LF sequence at the indicated position.
*/
bool hasCRLF(const UnicodeStringRef string, int32_t pos)
{
	if ((pos >= 0) && (pos < string.length() - 1))
		return (string.charAt(pos) == '\r') && (string.charAt(1) == '\n');
	return false;
}

/**
* True if the string has the LF character at the indicated position
*/
bool hasLF(const UnicodeString& string, int32_t pos)
{
	if ((pos >= 0) && (pos < string.length()))
		return string.charAt(pos) == '\n';
	return false;
}

/**
* True if the string has the LF character at the indicated position
*/
bool hasLF(const UnicodeStringRef string, int32_t pos)
{
	if ((pos >= 0) && (pos < string.length()))
		return string.charAt(pos) == '\n';
	return false;
}

/**
* Builds a new Unicode string from a string that should contain an XML tag.
* The contents of the XML tag is escaped so that its XML-sensitive entities are protected.
* 
* string: the original Unicode string
*/
UnicodeString protectXmlEntities(const UnicodeString& string)
{
	UnicodeString result;
	UErrorCode error = U_ZERO_ERROR;
	RegexMatcher matcher("<(\\S+)([^>]*)/>|<(\\S+)([^>]*)>(.*?)</\\3>", string, 0, error);
	if (U_FAILURE(error))
		throw unitexcpp::UnitexException(u_errorName(error));
	bool matched = false;
	while (matcher.find()) {
		matched = true;
		UnicodeString match = matcher.group(error);
		if (U_FAILURE(error))
			throw unitexcpp::UnitexException(u_errorName(error));
		if (match.endsWith(UNICODE_STRING_SIMPLE("/>"))) {
			UnicodeString tag = matcher.group(1, error);
			UnicodeString attributes = matcher.group(2, error);
			UnicodeString escapedAttributes = escapeXmlAttributes(attributes);
			result += UNICODE_STRING_SIMPLE("<") + tag + escapedAttributes + UNICODE_STRING_SIMPLE("/>");
		} else {
			UnicodeString tag = matcher.group(3, error);
			UnicodeString attributes = matcher.group(4, error);
			UnicodeString escapedAttributes = escapeXmlAttributes(attributes);
			UnicodeString content = matcher.group(5, error);
			result += UNICODE_STRING_SIMPLE("<") + tag + escapedAttributes + UNICODE_STRING_SIMPLE(">");
			result += protectXmlEntities(content);
			result += UNICODE_STRING_SIMPLE("</") + tag + UNICODE_STRING_SIMPLE(">");
		}
	}
	if (matched)
		return result;
	return escapeXml(string);
}

UnicodeString escapeXmlAttributes(const UnicodeString& attributes)
{
	UnicodeString result, val;
	int32_t pos = 0, len = attributes.length();
	bool inQuotes = false;
	while (pos < len) {
		UChar32 c = attributes.char32At(pos);
		if (!inQuotes && (c == '"')) {
			inQuotes = true;
			result += c;
			pos++;
			continue;
		} else if (inQuotes && (c == '"')) {
			if ((pos > 0) && (attributes.char32At(pos - 1) != '\\')) {
				result += escapeXml(val);
				val = UNICODE_STRING_SIMPLE("");
				result += c;
				pos++;
				inQuotes = false;
				continue;
			}
		}
		if (inQuotes)
			val += c;
		else
			result += c;
		pos++;
	}
	return result;
}

UnicodeString escapeXml(const UnicodeString& string)
{
	UnicodeString result;
	int32_t len = string.length();
	for (int32_t i = 0; i < len; i++) {
		UChar32 c = string.char32At(i);
		switch (c) {
		case '&':
			result += UNICODE_STRING_SIMPLE("&amp;");
			break;
		case '<':
			result += UNICODE_STRING_SIMPLE("&lt;");
			break;
		case '>':
			result += UNICODE_STRING_SIMPLE("&gt;");
			break;
		case '"':
			result += UNICODE_STRING_SIMPLE("&quot;");
			break;
		case '\'':
			result += UNICODE_STRING_SIMPLE("&apos;");
			break;
		default:
			if (c > 0x7F) {
				ostringstream oss;
				oss << "&#" << (int32_t)c << ";";
				result += oss.str().c_str();
			}
			else
				result += c;
			break;
		}
	}
	return result;
}

namespace unitexcpp
{
	static const Stringlist _EMPTYSTRINGLIST;
	const Stringlist& EMPTYSTRINGLIST = _EMPTYSTRINGLIST;

	static const UnicodeStringlist _EMPTYUNICODESTRINGLIST;
	const UnicodeStringlist& EMPTYUNICODESTRINGLIST = _EMPTYUNICODESTRINGLIST;

	static const UnicodeStringVector _EMPTYUNICODESTRINGVECTOR;
	const UnicodeStringVector& EMPTYUNICODESTRINGVECTOR = _EMPTYUNICODESTRINGVECTOR;
}

ostream& write(ostream& os, const set<UnicodeString>& items)
{
	os << "[";
	bool first = true;
	for (set<UnicodeString>::const_iterator it = items.begin(); it != items.end(); it++) {
		if (!first)
			os << ", ";
		else
			first = false;
		os << *it;
	}
	os << "]";
	return os;
}

ostream& write(ostream& os, const vector<UnicodeString>& items)
{
	os << "[vector of " << items.size() << " items:" << endl;
	for (size_t i = 0; i < items.size(); i++) {
		os << "\t" << i << ": " << items[i] << endl;
	}
	os << "]" << endl;
	return os;
}

using namespace unitexcpp;

ostream& operator <<(ostream& os, const UnicodeStringlist& list)
{
	os << "UnicodeStringlist with " << list.size() << " items:" << endl;
	size_t i = 0;
	for (UnicodeStringlist::const_iterator it = list.begin(); it != list.end(); it++, i++)
		os << "  " << i << ": '" << *it << "'" << endl;
	return os;
}

ostream& operator <<(ostream& os, const UnicodeStringVector& vector)
{
	os << "UnicodeStringVector with " << vector.size() << " items:" << endl;
	size_t i = 0;
	for (UnicodeStringVector::const_iterator it = vector.begin(); it != vector.end(); it++, i++)
		os << "  " << i << ": '" << *it << "'" << endl;
	return os;
}

UnicodeString toString(const UnicodeStringRef& stringRef)
{
	UnicodeString str;
	stringRef.extract(0, stringRef.length(), str);
	return str;
}

/**
* Converts a Unicode string in to a standard STL string, not caring about character sets.
* This is to be used with precaution and only to extract strings from parameter values we
* know won't include weird characters.
*/
const string convertUnicodeStringToRawString(const UnicodeString& unicodeString)
{
	ostringstream oss;
	oss << unicodeString;
	return oss.str();
}

size_t splitRegex(UnicodeStringlist& l, const UnicodeString& seq, const UnicodeString& regex)
{
	l.clear();

	if (seq.isEmpty())
		return 0;

	UErrorCode error = U_ZERO_ERROR;
	RegexMatcher matcher(regex, seq, 0, error);
	if (U_FAILURE(error)) {
		ostringstream oss;
		oss << "Error in splitRegex(" << regex << ") on '" << seq << "' : " << u_errorName(error);
		throw UnitexException(oss.str());
	}

	size_t count = 1;
	bool matches = matcher.find(0, error);
	while (matches) {
		count++;
		matches = matcher.find();
	}

	UnicodeString* chunks = new UnicodeString[count];
	matcher.reset();
	matcher.split(seq, chunks, count, error);

	for (size_t i = 0; i < count; i++) {
		l.push_back(chunks[i]);
	}

	delete[] chunks;
	return count;
}

size_t splitRegex(UnicodeStringVector& v, const UnicodeString& seq, const UnicodeString& regex)
{
	v.clear();

	if (seq.isEmpty())
		return 0;

	UErrorCode error = U_ZERO_ERROR;
	RegexMatcher matcher(regex, seq, 0, error);
	if (U_FAILURE(error)) {
		ostringstream oss;
		oss << "Error in splitRegex(" << regex << ") on '" << seq << "' : " << u_errorName(error);
		throw UnitexException(oss.str());
	}

	size_t count = 1;
	bool matches = matcher.find(0, error);
	while (matches) {
		count++;
		matches = matcher.find();
	}

	UnicodeString* chunks = new UnicodeString[count];
	matcher.reset();
	matcher.split(seq, chunks, count, error);

	for (size_t i = 0; i < count; i++) {
		v.push_back(chunks[i]);
	}

	delete[] chunks;
	return count;
}

size_t splitLines(UnicodeStringlist& l, const UnicodeString& seq, const bool allowEmpty)
{
	l.clear();
	if (allowEmpty) 
		return splitRegex(l, seq, "\\n+");
	else {
		UnicodeStringlist temp;
		splitRegex(temp, seq, "\\n+");
		BOOST_FOREACH(UnicodeString s, temp) {
			if (!isEmpty(s) && !isBlank(s))
				l.push_back(s);
		}
		return l.size();
	}
}

size_t splitLines(UnicodeStringVector& v, const UnicodeString& seq, const bool allowEmpty)
{
	v.clear();
	if (allowEmpty) 
		return splitRegex(v, seq, "\\n+");
	else {
		UnicodeStringlist temp;
		splitRegex(temp, seq, "\\n+");
		BOOST_FOREACH(UnicodeString s, temp) {
			if (!isEmpty(s) && !isBlank(s))
				v.push_back(s);
		}
		return v.size();
	}
}

UnicodeString join(const UnicodeStringlist& l, const UnicodeString& glue)
{
	UnicodeString result;
	size_t i = 0;
	for (UnicodeStringlist::const_iterator it = l.begin(); it != l.end(); it++, i++) {
		UnicodeString string = *it;
		string.trim();
		if (string.isEmpty())
			continue;
		if (i > 0)
			result.append(glue);
		result.append(string);
	}
	return result;
}

/*!
* Parses a string to read a boolean.
*/
bool parse_bool(const UnicodeString& str)
{
	ostringstream oss;
	oss << str;
	return parse_bool(oss.str());
}

bool parse_bool(const string& str)
{
	if (str.empty() || iequals(str, "false") || iequals(str, "no") || equals(str, "0"))
		return false;
	if (iequals(str, "true") || iequals(str, "yes") || equals(str, "1"))
		return true;

	ostringstream oss;
	oss << "\"" << str << "\" is not a valid boolean";
	throw std::invalid_argument(oss.str());
}

size_t readPropertyFile(map<string, string>& properties, const path& path)
{
	properties.clear();

	ifstream ifs(path.string().c_str());
	if (ifs.is_open()) {
		string line;
		while (ifs.good()) {
			getline(ifs, line);
			trim(line);
			if (!line.empty() && !starts_with(line, "#")) {
				vector<string> components;
				split(components, line, is_any_of("="), token_compress_on);
				if (components.size() == 2) {
					properties[components[0]] = components[1];
				}
			}
		}
		ifs.close();
	} else
		cout << "cannot open " << path << endl;

	return properties.size();
}

char** stringListToCharStarArray(const unitexcpp::Stringlist& stringList)
{
	size_t nb = stringList.size(), i = 0;
	char** cstrArray = new char*[nb];
	for (unitexcpp::Stringlist::const_iterator it = stringList.begin(); it != stringList.end(); it++, i++) {
		const string& str = *it;
		cstrArray[i] = new char[str.length() + 1];
		strcpy(cstrArray[i], str.c_str());
	}
	return cstrArray;
}

void deleteCharStarArray(size_t size, char** array)
{
	for (size_t i = 0; i < size; i++)
		delete[] array[i];
	delete[] array;
}

///////////////////////////////////////////////////////////////////////////////
//
// XML utilities
//
///////////////////////////////////////////////////////////////////////////////

void readXML(SAXParser& parser, const InputSource& inputSource)
{
	parser.setValidationScheme(SAXParser::Val_Always);
	parser.setDoNamespaces(false);
	parser.setDoSchema(false);

	parser.parse(inputSource);
}

void readXMLFile(SAXParser& parser, const string& filename)
{
	UnicodeString ustrFilename(filename.c_str());
	readXMLFile(parser, ustrFilename);
}

void readXMLFile(SAXParser& parser, const UnicodeString& filename)
{
	int32_t uiLen = filename.length();
	UChar* buffer = new UChar[uiLen + 1];
	assert(buffer != NULL);

	filename.extract(0, uiLen, buffer);
	buffer[uiLen] = 0;

	LocalFileInputSource fileIS((XMLCh const *) buffer);
	readXML(parser, fileIS);

	delete[] buffer;
}

void readXMLString(SAXParser& parser, const UnicodeString& xmlString)
{
	UnicodeStringRef uref(xmlString);
	char const *cpszString = uref.asUTF8().c_str();

	MemBufInputSource memIS((XMLByte const *) cpszString, strlen(cpszString), "sysID");
	readXML(parser, memIS);
}

///////////////////////////////////////////////////////////////////////////////
//
// File utilities
//
///////////////////////////////////////////////////////////////////////////////


#ifndef UNITEXAE_UTILS_H
#define UNITEXAE_UTILS_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#include <uima/unistrref.hpp>
#include <unicode/ustring.h>
#include <unicode/ustream.h>

#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/XMLException.hpp>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Native Unitex persistence tools

extern const std::string UNITEX_VIRTUAL_PATH_PREFIX;

boost::optional<std::string> get_env(const std::string& varname);

// String tools

std::ostream& write(std::ostream& os, const std::set<UnicodeString>& items);
std::ostream& write(std::ostream& os, const std::vector<UnicodeString>& items);

namespace unitexcpp
{
	typedef std::list<std::string> Stringlist;
	extern const Stringlist& EMPTYSTRINGLIST;

	typedef std::list<icu::UnicodeString> UnicodeStringlist;
	extern const UnicodeStringlist& EMPTYUNICODESTRINGLIST;
	std::ostream& operator <<(std::ostream& os, const UnicodeStringlist& list);

	typedef std::vector<icu::UnicodeString> UnicodeStringVector;
	extern const UnicodeStringVector& EMPTYUNICODESTRINGVECTOR;
	std::ostream& operator <<(std::ostream& os, const UnicodeStringVector& vector);
}

bool isEmpty(const icu::UnicodeString& string);
bool isBlank(const icu::UnicodeString& string);
bool isCRLF(const icu::UnicodeString& string);
bool isCRLF(const uima::UnicodeStringRef string);
bool hasCRLF(const icu::UnicodeString& string, int32_t pos);
bool hasCRLF(const uima::UnicodeStringRef string, int32_t pos);
bool hasLF(const icu::UnicodeString& string, int32_t pos);
bool hasLF(const uima::UnicodeStringRef string, int32_t pos);

icu::UnicodeString protectXmlEntities(const icu::UnicodeString& string);
icu::UnicodeString escapeXmlAttributes(const icu::UnicodeString& attributes);
icu::UnicodeString escapeXml(const icu::UnicodeString& string);

icu::UnicodeString toString(const uima::UnicodeStringRef& stringRef);
const std::string convertUnicodeStringToRawString(const icu::UnicodeString& unicodeString);

size_t splitRegex(unitexcpp::UnicodeStringlist& l, const icu::UnicodeString& seq, const icu::UnicodeString& regex);
size_t splitRegex(unitexcpp::UnicodeStringVector& v, const icu::UnicodeString& seq, const icu::UnicodeString& regex);
size_t splitLines(unitexcpp::UnicodeStringlist& l, const icu::UnicodeString& seq, const bool allowEmpty =true);
size_t splitLines(unitexcpp::UnicodeStringVector& v, const icu::UnicodeString& seq, const bool allowEmpty =true);
icu::UnicodeString join(const unitexcpp::UnicodeStringlist& l, const icu::UnicodeString& glue);

bool parse_bool(const icu::UnicodeString& str);
bool parse_bool(const std::string& str);
std::size_t readPropertyFile(std::map<std::string, std::string>& properties, const boost::filesystem::path& path);

char** stringListToCharStarArray(const unitexcpp::Stringlist& stringList);
void deleteCharStarArray(std::size_t size, char** array);

// Xerces tools

void readXMLFile(xercesc::SAXParser& parser, const std::string& filename);
void readXMLFile(xercesc::SAXParser& parser, const icu::UnicodeString& filename);
void readXMLString(xercesc::SAXParser& parser, const icu::UnicodeString& xmlString);

#endif // UNITEXAE_UTILS_H

/*
 * Language.cpp
 *
 *  Created on: 29 déc. 2010
 *      Author: sylvainsurcin
 */

#include "Language.h"
#include <unicode/ustream.h>
#include <iostream>
#include <sstream>
#include "UnitexException.h"

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

namespace unitexcpp
{

	Language::MAP* Language::languages = NULL;

	Language::Language(string const& strShort, string const& strNorm, UnicodeString const& utfLocal) :
		m_shortForm(strShort), m_normalizedForm(strNorm), m_localizedForm(utfLocal)
	{
	}

	Language::~Language()
	{
	}

	string const& Language::getShortForm() const
	{
		return m_shortForm;
	}

	string const& Language::getNormalizedForm() const
	{
		return m_normalizedForm;
	}

	UnicodeString const& Language::getLocalizedForm() const
	{
		return m_localizedForm;
	}

	Language const& Language::getLanguage(UnicodeString const& utfNorm)
	{
		ostringstream oss;
		oss << utfNorm;
		return getLanguage(oss.str());
	}

	Language const& Language::getLanguage(string const& strNorm)
	{
		if (languages == NULL)
			initStatic();

		MAP::const_iterator it = languages->find(strNorm);
		if (it == languages->end()) {
			ostringstream oss;
			oss << "Unknown language " << strNorm;
			throw UnitexException(oss.str());
		}
		return *(it->second);
	}

	void Language::initStatic()
	{
		languages = new MAP();
		(*languages)["French"] = new Language("fr", "French", UNICODE_STRING_SIMPLE("français"));
		(*languages)["English"] = new Language("en", "English", UNICODE_STRING_SIMPLE("English"));
	}

	void Language::destroyStatic()
	{
		for (MAP::iterator it = languages->begin(); it != languages->end(); it++)
			delete it->second;
		delete languages;
		languages = NULL;
	}
}

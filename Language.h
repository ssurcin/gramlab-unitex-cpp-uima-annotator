/*
 * Language.h
 *
 *  Created on: 29 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef LANGUAGE_H_
#define LANGUAGE_H_

#include <string>
#include <map>
#include "unicode/unistr.h"

namespace unitexcpp
{

	class Language
	{
		std::string m_shortForm;
		std::string m_normalizedForm;
		icu::UnicodeString m_localizedForm;

		typedef std::map<std::string, const Language*> MAP;
		static MAP* languages;

	protected:
		Language(std::string const& strShort, std::string const& strNorm, icu::UnicodeString const& utfLocal);
		virtual ~Language();

	public:
		std::string const& getShortForm() const;
		std::string const& getNormalizedForm() const;
		icu::UnicodeString const& getLocalizedForm() const;

		static Language const& getLanguage(icu::UnicodeString const& utfNorm);
		static Language const& getLanguage(std::string const& strNorm);
		static void destroyStatic();
	private:
		static void initStatic();
	};

}

#endif /* LANGUAGE_H_ */

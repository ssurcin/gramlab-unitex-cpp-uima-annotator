/*
 * LanguageAreaAnnotation.h
 *
 *  Created on: 30 juil. 2012
 *      Author: sylvain
 */

#ifndef LANGUAGEAREAANNOTATION_H_
#define LANGUAGEAREAANNOTATION_H_

#include "AnnotationWrapper.h"
#include <vector>
#include <list>

namespace unitexcpp
{
	namespace annotation
	{

		class LanguageArea:
				public unitexcpp::annotation::AnnotationWrapper
		{
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);
			static void allLanguageAreasInView(uima::CAS& aCas, std::list<LanguageArea>& languageAreas);

		public:
			LanguageArea();
			LanguageArea(uima::CAS& cas);
			LanguageArea(uima::AnnotationFS fs);
			LanguageArea(const LanguageArea& model);
			virtual ~LanguageArea();

			LanguageArea& operator=(const LanguageArea& model);

			uima::UnicodeStringRef getLanguage() const;
			void setLanguage(const icu::UnicodeString& value);
			void setLanguage(uima::UnicodeStringRef value);

			uima::UnicodeStringRef getAdditionalOpeningTag() const;
			void setAdditionalOpeningTag(const icu::UnicodeString& value);
			void setAdditionalOpeningTag(uima::UnicodeStringRef value);

			uima::UnicodeStringRef getAdditionalClosingTag() const;
			void setAdditionalClosingTag(const icu::UnicodeString& value);
			void setAdditionalClosingTag(uima::UnicodeStringRef value);

			std::vector<uima::UnicodeStringRef> getGraphFilter() const;
			std::vector<uima::UnicodeStringRef> getGraphNegativeFilter() const;

		private:
			static uima::Type tLanguageArea;
			static uima::Feature fLanguage;
			static uima::Feature fAdditionalOpeningTag;
			static uima::Feature fAdditionalClosingTag;
			static uima::Feature fGraphFilter;
			static uima::Feature fGraphNegativeFilter;
		};

	} /* namespace annotation */
} /* namespace unitexcpp */
#endif /* LANGUAGEAREAANNOTATION_H_ */

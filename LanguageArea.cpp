/*
 * LanguageAreaAnnotation.cpp
 *
 *  Created on: 30 juil. 2012
 *      Author: sylvain
 */

#include "LanguageArea.h"
#include "UnitexException.h"

using namespace std;
using namespace icu;
using namespace uima;

namespace unitexcpp
{
	namespace annotation
	{

		Type LanguageArea::tLanguageArea;
		Feature LanguageArea::fLanguage;
		Feature LanguageArea::fAdditionalOpeningTag;
		Feature LanguageArea::fAdditionalClosingTag;
		Feature LanguageArea::fGraphFilter;
		Feature LanguageArea::fGraphNegativeFilter;

		TyErrorId LanguageArea::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			tLanguageArea = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.general.tcas.LanguageArea");
			if (!tLanguageArea.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fLanguage = tLanguageArea.getFeatureByBaseName("language");
			fAdditionalOpeningTag = tLanguageArea.getFeatureByBaseName("additionalOpeningTag");
			fAdditionalClosingTag = tLanguageArea.getFeatureByBaseName("additionalClosingTag");
			fGraphFilter = tLanguageArea.getFeatureByBaseName("graphFilter");
			fGraphNegativeFilter = tLanguageArea.getFeatureByBaseName("graphNegativeFilter");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		void LanguageArea::allLanguageAreasInView(CAS& aCas, list<LanguageArea>& languageAreas)
		{
			languageAreas.clear();

			ANIterator languageAreaIterator = aCas.getAnnotationIndex(tLanguageArea).iterator();
			if (!languageAreaIterator.isValid()) {
				throw UnitexException("Unable to retrieve LanguageAreas");
			}

			languageAreaIterator.moveToFirst();
			while (languageAreaIterator.isValid()) {
				LanguageArea languageArea(languageAreaIterator.get());
				languageAreas.push_back(languageArea);
				languageAreaIterator.moveToNext();
			}
		}

		LanguageArea::LanguageArea()
		: AnnotationWrapper()
		{
		}

		LanguageArea::LanguageArea(CAS& aCas) :
				AnnotationWrapper(aCas)
		{
		}

		LanguageArea::LanguageArea(AnnotationFS fs) :
				AnnotationWrapper(fs)
		{
		}

		LanguageArea::LanguageArea(const LanguageArea& model)
		{
			*this = model;
		}

		LanguageArea::~LanguageArea()
		{
		}

		LanguageArea& LanguageArea::operator =(const LanguageArea& model)
		{
			this->AnnotationWrapper::operator =(model);
			return *this;
		}

		UnicodeStringRef LanguageArea::getLanguage() const
		{
			return annotation.getStringValue(fLanguage);
		}

		void LanguageArea::setLanguage(UnicodeStringRef value)
		{
			annotation.setStringValue(fLanguage, value);
		}

		void LanguageArea::setLanguage(const UnicodeString& value)
		{
			annotation.setStringValue(fLanguage, value);
		}

		UnicodeStringRef LanguageArea::getAdditionalOpeningTag() const
		{
			return annotation.getStringValue(fAdditionalOpeningTag);
		}

		void LanguageArea::setAdditionalOpeningTag(UnicodeStringRef value)
		{
			annotation.setStringValue(fAdditionalOpeningTag, value);
		}

		void LanguageArea::setAdditionalOpeningTag(const UnicodeString& value)
		{
			annotation.setStringValue(fAdditionalOpeningTag, value);
		}

		UnicodeStringRef LanguageArea::getAdditionalClosingTag() const
		{
			return annotation.getStringValue(fAdditionalClosingTag);
		}

		void LanguageArea::setAdditionalClosingTag(UnicodeStringRef value)
		{
			annotation.setStringValue(fAdditionalClosingTag, value);
		}

		void LanguageArea::setAdditionalClosingTag(const UnicodeString& value)
		{
			annotation.setStringValue(fAdditionalClosingTag, value);
		}

		vector<UnicodeStringRef> LanguageArea::getGraphFilter() const
		{
			vector<UnicodeStringRef> result;
			StringArrayFS arrayFS = annotation.getStringArrayFSValue(fGraphFilter);
			if (arrayFS.isValid()) {
				for (size_t i = 0; i < arrayFS.size(); i++)
					result.push_back(arrayFS.get(i));
			}
			return result;
		}

		vector<UnicodeStringRef> LanguageArea::getGraphNegativeFilter() const
		{
			vector<UnicodeStringRef> result;
			StringArrayFS arrayFS = annotation.getStringArrayFSValue(fGraphNegativeFilter);
			if (arrayFS.isValid()) {
				for (size_t i = 0; i < arrayFS.size(); i++)
					result.push_back(arrayFS.get(i));
			}
			return result;
		}

	} /* namespace annotation */
} /* namespace unitexcpp */

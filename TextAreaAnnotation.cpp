/*
 * TextArea.cpp
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#include "TextAreaAnnotation.h"
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

using namespace uima;

namespace unitexcpp
{

	namespace annotation
	{

		uima::Type TextAreaAnnotation::tTextAreaAnnotation;
		uima::Feature TextAreaAnnotation::fPrevious;
		uima::Feature TextAreaAnnotation::fNext;

		TyErrorId TextAreaAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			tTextAreaAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.TextAreaAnnotation");
			if (!tTextAreaAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fPrevious = tTextAreaAnnotation.getFeatureByBaseName("previous");
			fNext = tTextAreaAnnotation.getFeatureByBaseName("next");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		TextAreaAnnotation::TextAreaAnnotation()
		{
		}

		TextAreaAnnotation::TextAreaAnnotation(CAS& aCas) :
			AnnotationWrapper(aCas)
		{
		}

		TextAreaAnnotation::TextAreaAnnotation(AnnotationFS& fs) :
			AnnotationWrapper(fs)
		{
		}

		TextAreaAnnotation::~TextAreaAnnotation()
		{
		}

		TextAreaAnnotation& TextAreaAnnotation::operator =(const TextAreaAnnotation& model)
		{
			this->AnnotationWrapper::operator =(model);
			return *this;
		}

		TextAreaAnnotation TextAreaAnnotation::getPrevious() const
		{
			AnnotationFS fs(annotation.getFSValue(fPrevious));
			return TextAreaAnnotation(fs);
		}

		void TextAreaAnnotation::setPrevious(const TextAreaAnnotation& ta)
		{
			annotation.setFSValue(fPrevious, ta.annotation);
		}

		TextAreaAnnotation TextAreaAnnotation::getNext() const
		{
			AnnotationFS fs(annotation.getFSValue(fNext));
			return TextAreaAnnotation(fs);
		}

		void TextAreaAnnotation::setNext(const TextAreaAnnotation& ta)
		{
			annotation.setFSValue(fNext, ta.annotation);
		}

	}

}

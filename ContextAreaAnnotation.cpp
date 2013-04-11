/*
 * ContextAreaAnnotation.cpp
 *
 *  Created on: 14 janv. 2011
 *      Author: sylvainsurcin
 */

#include "ContextAreaAnnotation.h"
#include "TokenAnnotation.h"

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
using namespace icu;

namespace unitexcpp
{

	namespace annotation
	{

		Type ContextAreaAnnotation::tContextArea;
		Feature ContextAreaAnnotation::fFirstToken;
		Feature ContextAreaAnnotation::fLastToken;

		TyErrorId ContextAreaAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			if (TextAreaAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE)
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			tContextArea = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.ContextAnnotation");
			if (!tContextArea.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fFirstToken = tContextArea.getFeatureByBaseName("firstToken");
			fLastToken = tContextArea.getFeatureByBaseName("lastToken");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		/*!
		 * Default constructor builds an invalid instance.
		 */
		ContextAreaAnnotation::ContextAreaAnnotation()
		{
		}

		/*!
		 * Builds a new empty annotation in a CAS.
		 */
		ContextAreaAnnotation::ContextAreaAnnotation(CAS& aCas) :
			TextAreaAnnotation(aCas)
		{
		}

		/*!
		 * \brief Wrapper constructor
		 *
		 * Wraps an existing annotation into an instance
		 */
		ContextAreaAnnotation::ContextAreaAnnotation(uima::AnnotationFS fs) :
			TextAreaAnnotation(fs)
		{
		}

		/*!
		 * Destructor does nothing.
		 */
		ContextAreaAnnotation::~ContextAreaAnnotation()
		{
		}

		/*!
		 * Shallow copy operator.
		 * Copies the annotation only.
		 */
		ContextAreaAnnotation& ContextAreaAnnotation::operator =(const ContextAreaAnnotation& model)
		{
			pCas = model.pCas;
			annotation = model.annotation;
			return *this;
		}

		void ContextAreaAnnotation::setFirstToken(const TokenAnnotation& token)
		{
			annotation.setFSValue(fFirstToken, token.getAnnotation());
		}

		TokenAnnotation ContextAreaAnnotation::getFirstToken() const
		{
			AnnotationFS anAnnotation(annotation.getFSValue(fFirstToken));
			return TokenAnnotation(anAnnotation);
		}

		void ContextAreaAnnotation::setLastToken(const TokenAnnotation& token)
		{
			annotation.setFSValue(fLastToken, token.getAnnotation());
		}

		TokenAnnotation ContextAreaAnnotation::getLastToken() const
		{
			AnnotationFS anAnnotation(annotation.getFSValue(fLastToken));
			return TokenAnnotation(anAnnotation);
		}

		/*
		void ContextAreaAnnotation::getCoveredText(UnicodeString& ustring) const
		{
			UnicodeStringRef document = pCas->getDocumentText();
			document.extractBetween(getFirstToken().getBegin(), getLastToken().getEnd(), ustring);
		}
		*/
	}

}

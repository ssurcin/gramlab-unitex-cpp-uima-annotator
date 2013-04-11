/*
 * TransductionOutputAnnotation.cpp
 *
 *  Created on: 18 janv. 2011
 *      Author: sylvainsurcin
 */

#include "TransductionOutputAnnotation.h"

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
using namespace icu;

namespace unitexcpp
{

	namespace annotation
	{

		Type TransductionOutputAnnotation::tTransductionOutputAnnotation;
		Feature TransductionOutputAnnotation::fOutput;

		/*!
		 * Initializes the typesystem types and features
		 */
		TyErrorId TransductionOutputAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			tTransductionOutputAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.TransductionOutputAnnotation");
			if (!tTransductionOutputAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fOutput = tTransductionOutputAnnotation.getFeatureByBaseName("output");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		/*!
		 * Builds an invalid instance
		 */
		TransductionOutputAnnotation::TransductionOutputAnnotation()
		{
		}

		/*!
		 * Explicit constructor.
		 */
		TransductionOutputAnnotation::TransductionOutputAnnotation(CAS& aCas, size_t begin, size_t end, const UnicodeString& ustring) :
			AnnotationWrapper(aCas)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tTransductionOutputAnnotation, begin, end);
			setOutput(ustring);
			indexRep.addFS(annotation);
		}

		/*!
		 * Wraps an existing annotation.
		 */
		TransductionOutputAnnotation::TransductionOutputAnnotation(AnnotationFS& fs)
		: AnnotationWrapper(fs)
		{
		}

		TransductionOutputAnnotation::~TransductionOutputAnnotation()
		{
		}

		UnicodeStringRef TransductionOutputAnnotation::getOutput() const
		{
			return annotation.getStringValue(fOutput);
		}

		void TransductionOutputAnnotation::setOutput(const UnicodeString& ustring)
		{
			annotation.setStringValue(fOutput, ustring);
		}

		ostream& operator <<(ostream& os, const TransductionOutputAnnotation& annotation)
		{
			os << "(" << annotation.getBegin() << "," << annotation.getEnd() << ":" << annotation.getOutput() << " over \"" << annotation.getCoveredText() << "\")";
			return os;
		}

	}

}

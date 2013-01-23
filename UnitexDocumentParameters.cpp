/*
 * UnitexDocumentParameters.cpp
 *
 *  Created on: 3 août 2012
 *      Author: sylvain
 */

#include "UnitexDocumentParameters.h"
#include "UnitexException.h"

using namespace uima;
using namespace std;
using namespace icu;

namespace unitexcpp
{
	namespace annotation
	{

		Type UnitexDocumentParameters::tUnitexDocumentParameters;
		Feature UnitexDocumentParameters::fAnalysisStrategy;
		Feature UnitexDocumentParameters::fSkip;
		Feature UnitexDocumentParameters::fUri;

		TyErrorId UnitexDocumentParameters::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			tUnitexDocumentParameters = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.UnitexDocumentParameters");
			if (!tUnitexDocumentParameters.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fAnalysisStrategy = tUnitexDocumentParameters.getFeatureByBaseName("analysisStrategy");
			fSkip = tUnitexDocumentParameters.getFeatureByBaseName("skip");
			fUri = tUnitexDocumentParameters.getFeatureByBaseName("uri");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		UnitexDocumentParameters UnitexDocumentParameters::getUnitexDocumentParameters(CAS& view)
		{
			ANIterator iterator = view.getAnnotationIndex(tUnitexDocumentParameters).iterator();
			if (!iterator.isValid()) {
				throw UnitexException("Unable to retrieve UnitexDocumentParameters");
			}

			iterator.moveToFirst();
			return UnitexDocumentParameters(iterator.get());
		}

		UnitexDocumentParameters::UnitexDocumentParameters()
		{
		}

		UnitexDocumentParameters::UnitexDocumentParameters(AnnotationFS fs) :
				AnnotationWrapper(fs)
		{
		}

		UnitexDocumentParameters::~UnitexDocumentParameters()
		{
		}

		UnicodeStringRef UnitexDocumentParameters::getAnalysisStrategy() const
		{
			return annotation.getStringValue(fAnalysisStrategy);
		}

		bool UnitexDocumentParameters::getSkip() const
		{
			return annotation.getBooleanValue(fSkip);
		}

		UnicodeStringRef UnitexDocumentParameters::getUri() const
		{
			return annotation.getStringValue(fUri);
		}

	} /* namespace annotation */
} /* namespace unitexcpp */

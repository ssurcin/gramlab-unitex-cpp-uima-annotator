#include "AnnotatorPerformanceAnnotation.h"

using namespace std;
using namespace uima;
using namespace icu;

namespace unitexcpp
{
	namespace annotation
	{
		Type AnnotatorPerformanceAnnotation::tAnnotatorPerformanceAnnotation;
		Feature AnnotatorPerformanceAnnotation::fComponentName;
		Feature AnnotatorPerformanceAnnotation::fElapsedTime;

		/*!
		* Initializes the typesystem types and features
		*/
		TyErrorId AnnotatorPerformanceAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			tAnnotatorPerformanceAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.general.tcas.AnnotatorPerformanceAnnotation");
			if (!tAnnotatorPerformanceAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fComponentName = tAnnotatorPerformanceAnnotation.getFeatureByBaseName("componentName");
			fElapsedTime = tAnnotatorPerformanceAnnotation.getFeatureByBaseName("elapsedTime");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		/*!
		* Builds an invalid instance
		*/
		AnnotatorPerformanceAnnotation::AnnotatorPerformanceAnnotation(void)
		{
		}

		/*!
		 * Explicit constructor.
		 */
		AnnotatorPerformanceAnnotation::AnnotatorPerformanceAnnotation(CAS& aCas, const UnicodeString& annotatorName, long elapsedMillis)
			: AnnotationWrapper(aCas)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tAnnotatorPerformanceAnnotation, 0, 0);
			setComponentName(annotatorName);
			setElapsedTime(elapsedMillis);
			indexRep.addFS(annotation);
		}

		/*!
		 * Wraps an existing annotation.
		 */
		AnnotatorPerformanceAnnotation::AnnotatorPerformanceAnnotation(AnnotationFS& fs)
		: AnnotationWrapper(fs)
		{
		}

		AnnotatorPerformanceAnnotation::~AnnotatorPerformanceAnnotation(void)
		{
		}

		UnicodeStringRef AnnotatorPerformanceAnnotation::getComponentName() const
		{
			return annotation.getStringValue(fComponentName);
		}

		void AnnotatorPerformanceAnnotation::setComponentName(const UnicodeString& componentName) 
		{
			annotation.setStringValue(fComponentName, componentName);
		}

		long AnnotatorPerformanceAnnotation::getElapsedTime() const
		{
			return annotation.getLongValue(fElapsedTime);
		}

		void AnnotatorPerformanceAnnotation::setElapsedTime(long elapsedMillis)
		{
			annotation.setLongValue(fElapsedTime, elapsedMillis);
		}

		ostream& operator <<(ostream& os, const AnnotatorPerformanceAnnotation& annotation)
		{
			os << "(Time spent in " << annotation.getComponentName() << "=" << annotation.getElapsedTime() << "ms)";
			return os;
		}
	}
}

#include "AutomatonLocatePerformanceAnnotation.h"

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
		Type AutomatonLocatePerformanceAnnotation::tAutomatonLocatePerformanceAnnotation;
		Feature AutomatonLocatePerformanceAnnotation::fComponentName;
		Feature AutomatonLocatePerformanceAnnotation::fElapsedTime;

		/*!
		* Initializes the typesystem types and features
		*/
		TyErrorId AutomatonLocatePerformanceAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			tAutomatonLocatePerformanceAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.general.tcas.AutomatonLocatePerformanceAnnatation");
			if (!tAutomatonLocatePerformanceAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fComponentName = tAutomatonLocatePerformanceAnnotation.getFeatureByBaseName("componentName");
			fElapsedTime = tAutomatonLocatePerformanceAnnotation.getFeatureByBaseName("elapsedTime");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		/*!
		* Builds an invalid instance
		*/
		AutomatonLocatePerformanceAnnotation::AutomatonLocatePerformanceAnnotation(void)
		{
		}

		/*!
		 * Explicit constructor.
		 */
		AutomatonLocatePerformanceAnnotation::AutomatonLocatePerformanceAnnotation(CAS& aCas, const UnicodeString& annotatorName, long elapsedMillis)
			: AnnotationWrapper(aCas)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tAutomatonLocatePerformanceAnnotation, 0, 0);
			setComponentName(annotatorName);
			setElapsedTime(elapsedMillis);
			indexRep.addFS(annotation);
		}

		/*!
		 * Wraps an existing annotation.
		 */
		AutomatonLocatePerformanceAnnotation::AutomatonLocatePerformanceAnnotation(AnnotationFS& fs)
		: AnnotationWrapper(fs)
		{
		}

		AutomatonLocatePerformanceAnnotation::~AutomatonLocatePerformanceAnnotation(void)
		{
		}

		UnicodeStringRef AutomatonLocatePerformanceAnnotation::getComponentName() const
		{
			return annotation.getStringValue(fComponentName);
		}

		void AutomatonLocatePerformanceAnnotation::setComponentName(const UnicodeString& componentName) 
		{
			annotation.setStringValue(fComponentName, componentName);
		}

		long AutomatonLocatePerformanceAnnotation::getElapsedTime() const
		{
			return annotation.getLongValue(fElapsedTime);
		}

		void AutomatonLocatePerformanceAnnotation::setElapsedTime(long elapsedMillis)
		{
			annotation.setLongValue(fElapsedTime, elapsedMillis);
		}

		ostream& operator <<(ostream& os, const AutomatonLocatePerformanceAnnotation& annotation)
		{
			os << "(" << annotation.getView().getViewName() << ": Time spent in automaton " << annotation.getComponentName() << "=" << annotation.getElapsedTime() << "ms)";
			return os;
		}
	}
}

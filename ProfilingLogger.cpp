#include "ProfilingLogger.h"
#include "AnnotatorPerformanceAnnotation.h"
#include "AutomatonLocatePerformanceAnnotation.h"
#include "Utils.h"
#include "UnitexEngine.h"
#include <map>
#include <sstream>
#include <boost/foreach.hpp>

using namespace uima;
using namespace std;

namespace unitexcpp
{

	ProfilingLogger::ProfilingLogger(CAS& cas, UnitexAnnotatorCpp& annotator)
		: m_cas(cas), m_annotator(annotator)
	{
	}


	ProfilingLogger::~ProfilingLogger(void)
	{
		storeProfilingInformation();
	}

	void ProfilingLogger::storeProfilingInformation()
	{
		double elapsedTime = m_timer.elapsed();
		m_annotator.logMessage("Time spent in UnitexAnnotatorCpp: %f ms", elapsedTime);
		annotation::AnnotatorPerformanceAnnotation annotatorPerformanceAnnotation(m_cas, "UnitexAnnotatorCpp", elapsedTime * 1000);
		storeAutomataProfilingInformation();
	}

	void ProfilingLogger::storeAutomataProfilingInformation()
	{
		map<string, map<string, long> > performanceMap;

		UnitexAnnotatorCpp::UnicodeStringPair langStrategy;
		engine::UnitexEngine* pEngine;
		BOOST_FOREACH(boost::tie(langStrategy, pEngine), m_annotator.getUnitexInstances()) {
			string language = convertUnicodeStringToRawString(langStrategy.first);
			const map<string, long>& perf = pEngine->getTextProcessor().getLocatePerformanceMap();
			if (perf.size() > 0) {
				map<string, long> cumulatedPerformances;

				map<string, map<string, long> >::iterator it = performanceMap.find(language);
				if (it != performanceMap.end()) {
					cumulatedPerformances = it->second;
				}

				string automaton;
				long duration;
				BOOST_FOREACH(boost::tie(automaton, duration), perf) {
					map<string, long>::iterator jt = cumulatedPerformances.find(automaton);
					if (jt != cumulatedPerformances.end())
						duration += jt->second;
					cumulatedPerformances[automaton] = duration;
				}
				performanceMap[language] = cumulatedPerformances;
			}
		}

		string language;
		map<string, long> langPerformances;
		BOOST_FOREACH(boost::tie(language, langPerformances), performanceMap) {
			string automaton;
			long elapsedTime;
			BOOST_FOREACH(boost::tie(automaton, elapsedTime), langPerformances) {
				ostringstream oss;
				oss << language << ":" << automaton;
				UnicodeString ustring = oss.str().c_str();
				annotation::AutomatonLocatePerformanceAnnotation annotation(m_annotator.getView(), ustring, elapsedTime);
			}
		}
	}
}

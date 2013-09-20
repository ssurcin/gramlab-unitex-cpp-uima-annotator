
#include "ProfilingLogger.h"
#include "AnnotatorPerformanceAnnotation.h"
#include "AutomatonLocatePerformanceAnnotation.h"
#include "Utils.h"
#include "UnitexEngine.h"
#include <map>
#include <sstream>
#include <boost/foreach.hpp>

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

#ifdef DEBUG_UIMA_CPP
	void dumpAutomatonPerfMap(const map<string, long>& perfMap, int indentation =0)
	{
		string automaton;
		long elapsedMs;
		string indent;
		for (int i = 0; i < indentation; i++) indent += "  ";
		BOOST_FOREACH(boost::tie(automaton, elapsedMs), perfMap) {
			cout << indent << automaton << " : " << elapsedMs << " ms" << endl;
		}
	}
#endif

	/// <remarks>
	/// This is called at the end of each UnitexAnnotatorCpp::process, i.e. after each document.
	/// </remarks>
	void ProfilingLogger::storeAutomataProfilingInformation()
	{
#ifdef DEBUG_UIMA_CPP
		cout << "Enter ProfilingLogger::storeAutomataProfilingInformation" << endl;
		cout << "Build an empty performance map for each language " << endl;
#endif
		Language2AutomatonPerformanceMap performanceMap;

		UnitexAnnotatorCpp::UnicodeStringPair langStrategy;
		engine::UnitexEngine* pEngine;
		BOOST_FOREACH(boost::tie(langStrategy, pEngine), m_annotator.getUnitexInstances()) {
			string language = convertUnicodeStringToRawString(langStrategy.first);
			string strategy = convertUnicodeStringToRawString(langStrategy.second);
#ifdef DEBUG_UIMA_CPP
		cout << "(" << language << ", " << strategy << ")" << endl;
#endif

			const map<string, long>& perf = pEngine->getTextProcessor().getLocatePerformanceMap();
#ifdef DEBUG_UIMA_CPP
			cout << "  Performances for this engine's text processor:" << endl;
			dumpAutomatonPerfMap(perf, 2);
#endif
			if (perf.size() > 0) {
				map<string, long> cumulatedPerformances;

#ifdef DEBUG_UIMA_CPP
				cout << "  Looking for the cumulated performances for " << language << endl;
#endif
				map<string, map<string, long> >::iterator it = performanceMap.find(language);
				if (it != performanceMap.end()) {
					cumulatedPerformances = it->second;
#ifdef DEBUG_UIMA_CPP
					cout << "  found" << endl;
#endif
				}
#ifdef DEBUG_UIMA_CPP
				else
					cout << "  not found" << endl;
#endif

#ifdef DEBUG_UIMA_CPP
				cout << "  Adding current engine's results to cumulated performances" << endl;
#endif
				string automaton;
				long duration;
				BOOST_FOREACH(boost::tie(automaton, duration), perf) {
#ifdef DEBUG_UIMA_CPP
					cout << "    Current performances of " << automaton << " = " << duration << " ms" << endl;
					cout << "    Looking for cumulated performances of " << automaton << endl;
#endif
					map<string, long>::iterator jt = cumulatedPerformances.find(automaton);
					if (jt != cumulatedPerformances.end()) {
#ifdef DEBUG_UIMA_CPP
						cout << "      found" << endl;
#endif
						duration += jt->second;
#ifdef DEBUG_UIMA_CPP
						cout << "      new duration = " << duration << endl;
#endif
					}
					cumulatedPerformances[automaton] = duration;
#ifdef DEBUG_UIMA_CPP
					cout << "    Set cumulated performance of " << automaton << " = " << duration << endl;
#endif
				}
#ifdef DEBUG_UIMA_CPP
				cout << "  Storing cumulated performances for " << language << endl;
#endif
				performanceMap[language] = cumulatedPerformances;
			}
		}

		createCumulatedPerformanceAutomatonAnnotations(performanceMap);
	}

	void ProfilingLogger::createCumulatedPerformanceAutomatonAnnotations(const Language2AutomatonPerformanceMap& performanceMap) 
	{
#ifdef DEBUG_UIMA_CPP
		cout << "Creating cumulated performances annotations " << endl;
#endif
		string language;
		map<string, long> langPerformances;
		BOOST_FOREACH(boost::tie(language, langPerformances), performanceMap) {
#ifdef DEBUG_UIMA_CPP
			cout << "  For language " << language << endl;
#endif
			string automaton;
			long elapsedTime;
			BOOST_FOREACH(boost::tie(automaton, elapsedTime), langPerformances) {
				ostringstream oss;
				oss << language << ":" << automaton;
#ifdef DEBUG_UIMA_CPP
				cout << "    " << automaton << " :" << elapsedTime << " ms" << endl;
#endif
				UnicodeString ustring = oss.str().c_str();
				annotation::AutomatonLocatePerformanceAnnotation annotation(m_annotator.getView(), ustring, elapsedTime);
			}
		}
	}
}

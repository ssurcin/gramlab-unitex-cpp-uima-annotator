#ifndef PROFILINGLOGGER_H_
#define PROFILINGLOGGER_H_

#include <uima/api.hpp>
#include <boost/timer.hpp>
#include "UnitexAnnotatorCpp.h"

namespace unitexcpp
{
	class ProfilingLogger
	{
	public:
		ProfilingLogger(uima::CAS& cas, uima::UnitexAnnotatorCpp& annotator);
		virtual ~ProfilingLogger(void);

	private:
		void storeProfilingInformation();
		void storeAutomataProfilingInformation();

		typedef std::map<std::string, long> AutomatonPerformanceMap;
		typedef std::map<std::string, AutomatonPerformanceMap> Language2AutomatonPerformanceMap;

		void createCumulatedPerformanceAutomatonAnnotations(const Language2AutomatonPerformanceMap& performanceMap);

	private:
		uima::CAS& m_cas;
		uima::UnitexAnnotatorCpp& m_annotator;
		boost::timer m_timer;
	};

}
#endif // PROFILINGLOGGER_H_
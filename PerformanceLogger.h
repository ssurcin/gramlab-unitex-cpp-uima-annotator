#ifndef PERFORMANCELOGGER_H_
#define PERFORMANCELOGGER_H_

#include <boost/timer.hpp>
#include "UnitexAnnotatorCpp.h"

namespace unitexcpp
{

	class PerformanceLogger
	{
	public:
		typedef std::map<std::string, long> AutomatonPerformanceMap;
		typedef std::map<std::string, AutomatonPerformanceMap> Language2AutomatonPerformanceMap;

	private:
		uima::UnitexAnnotatorCpp& m_annotator;

	public:
		PerformanceLogger(uima::UnitexAnnotatorCpp& annotator);
		virtual ~PerformanceLogger();

		void reset();
	};

}
#endif // PERFORMANCELOGGER_H_

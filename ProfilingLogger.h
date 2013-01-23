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

	private:
		uima::CAS& m_cas;
		uima::UnitexAnnotatorCpp& m_annotator;
		boost::timer m_timer;
	};

}
#endif // PROFILINGLOGGER_H_
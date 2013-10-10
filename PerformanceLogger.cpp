#include "PerformanceLogger.h"

using namespace uima;
using namespace std;

namespace unitexcpp
{

	PerformanceLogger::PerformanceLogger(UnitexAnnotatorCpp& annotator)
		: m_annotator(annotator)
	{
	}


	PerformanceLogger::~PerformanceLogger(void)
	{
	}

}
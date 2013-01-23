/*
 * UnitexSubengine.cpp
 *
 *  Created on: 28 déc. 2010
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "UnitexSubengine.h"
#include "UnitexEngine.h"
#include <string>

using namespace std;

namespace unitexcpp
{
	namespace engine
	{

		UnitexSubengine::UnitexSubengine(UnitexEngine& engine) :
			m_refEngine(engine)
		{
		}

		UnitexSubengine::~UnitexSubengine()
		{
		}

		UnitexEngine& UnitexSubengine::getEngine() const
		{
			return m_refEngine;
		}

		const UnitexAnnotatorCpp& UnitexSubengine::getAnnotator() const
		{
			return getEngine().getAnnotator();

		}
		const AnnotatorContext& UnitexSubengine::getAnnotatorContext() const
		{
			return getAnnotator().getAnnotatorContext();
		}

		/////////////////////////////////////////////////////////////////////////
		//
		// Logging
		//
		/////////////////////////////////////////////////////////////////////////

		void UnitexSubengine::logMessage(const char* szFormat, ...) const
		{
			va_list ap;
			va_start(ap, szFormat);
			getAnnotator().logMessage(szFormat, ap);
			va_end(ap);
		}

		void UnitexSubengine::logMessage(const string& format, ...) const
		{
			va_list ap;
			va_start(ap, format);
			getAnnotator().logMessage(format, ap);
			va_end(ap);
		}

		void UnitexSubengine::logWarning(const char* szFormat, ...) const
		{
			va_list ap;
			va_start(ap, szFormat);
			getAnnotator().logWarning(szFormat, ap);
			va_end(ap);
		}

		void UnitexSubengine::logWarning(const string& format, ...) const
		{
			va_list ap;
			va_start(ap, format);
			getAnnotator().logWarning(format, ap);
			va_end(ap);
		}

		void UnitexSubengine::logError(const char* szFormat, ...) const
		{
			va_list ap;
			va_start(ap, szFormat);
			getAnnotator().logError(szFormat, ap);
			va_end(ap);
		}

		void UnitexSubengine::logError(const string& format, ...) const
		{
			va_list ap;
			va_start(ap, format);
			getAnnotator().logError(format, ap);
			va_end(ap);
		}

	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

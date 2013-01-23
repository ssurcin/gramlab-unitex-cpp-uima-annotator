/*
 * UnitexSubengine.h
 *
 *  Created on: 28 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef UNITEXSUBENGINE_H_
#define UNITEXSUBENGINE_H_

#include "UnitexAnnotatorCpp.h"

namespace unitexcpp
{
	namespace engine
	{
		class UnitexEngine;

		class UnitexSubengine
		{
		private:
			UnitexEngine& m_refEngine;

		public:
			UnitexSubengine(UnitexEngine& engine);
			virtual ~UnitexSubengine();

			UnitexEngine& getEngine() const;
			const uima::UnitexAnnotatorCpp& getAnnotator() const;
			const uima::AnnotatorContext& getAnnotatorContext() const;

			void logMessage(const char* szFormat, ...) const;
			void logMessage(const std::string& format, ...) const;
			void logWarning(const char* szFormat, ...) const;
			void logWarning(const std::string& format, ...) const;
			void logError(const char* szFormat, ...) const;
			void logError(const std::string& format, ...) const;
		};

	}
}

#endif /* UNITEXSUBENGINE_H_ */

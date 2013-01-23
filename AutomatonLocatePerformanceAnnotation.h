#ifndef AUTOMATONLOCATEPERFORMANCEANNOTATION_H_
#define AUTOMATONLOCATEPERFORMANCEANNOTATION_H_

#include "AnnotationWrapper.h"
#include <uima/api.hpp>

namespace unitexcpp
{
	namespace annotation
	{
		/**
		* This class is the C++ representation of an AutomatonLocatePerformanceAnnotation in the output CAS.
		*/
		class AutomatonLocatePerformanceAnnotation : public AnnotationWrapper
		{
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		public:
			AutomatonLocatePerformanceAnnotation(void);
			AutomatonLocatePerformanceAnnotation(uima::CAS& cas, const icu::UnicodeString& automatonName, long elapsedMillis);
			AutomatonLocatePerformanceAnnotation(uima::AnnotationFS& fs);
			virtual ~AutomatonLocatePerformanceAnnotation(void);

			uima::UnicodeStringRef getComponentName() const;
			void setComponentName(const icu::UnicodeString& ustring);

			long getElapsedTime() const;
			void setElapsedTime(long elapsedMillis);

			friend std::ostream& operator <<(std::ostream& os, const AutomatonLocatePerformanceAnnotation& annotation);

		private:
			static uima::Type tAutomatonLocatePerformanceAnnotation;
			static uima::Feature fComponentName;
			static uima::Feature fElapsedTime;
		};
	}
}

#endif

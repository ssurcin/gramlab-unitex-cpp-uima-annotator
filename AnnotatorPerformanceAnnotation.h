#ifndef ANNOTATORPERFORMANCEANNOTATION_H_
#define ANNOTATORPERFORMANCEANNOTATION_H_

#include "AnnotationWrapper.h"
#include <uima/api.hpp>

namespace unitexcpp
{
	namespace annotation
	{
		/**
		 * This class is the C++ representation of an AnnotatorPerformanceAnnotation in the output CAS.
		 */
		class AnnotatorPerformanceAnnotation : public AnnotationWrapper
		{		
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		public:
			AnnotatorPerformanceAnnotation(void);
			AnnotatorPerformanceAnnotation(uima::CAS& cas, const icu::UnicodeString& annotatorName, long elapsedMillis);
			AnnotatorPerformanceAnnotation(uima::AnnotationFS& fs);
			virtual ~AnnotatorPerformanceAnnotation(void);

			uima::UnicodeStringRef getComponentName() const;
			void setComponentName(const icu::UnicodeString& ustring);

			long getElapsedTime() const;
			void setElapsedTime(long elapsedMillis);

			friend std::ostream& operator <<(std::ostream& os, const AnnotatorPerformanceAnnotation& annotation);

		private:
			static uima::Type tAnnotatorPerformanceAnnotation;
			static uima::Feature fComponentName;
			static uima::Feature fElapsedTime;
		};
	}
}

#endif //ANNOTATORPERFORMANCEANNOTATION_H_

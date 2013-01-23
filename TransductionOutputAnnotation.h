/*
 * TransductionOutputAnnotation.h
 *
 *  Created on: 18 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TRANSDUCTIONOUTPUTANNOTATION_H_
#define TRANSDUCTIONOUTPUTANNOTATION_H_

#include "AnnotationWrapper.h"
#include <uima/api.hpp>

namespace unitexcpp
{

	namespace annotation
	{

		class TransductionOutputAnnotation: public unitexcpp::annotation::AnnotationWrapper
		{
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		public:
			TransductionOutputAnnotation();
			TransductionOutputAnnotation(uima::CAS& aCas, size_t begin, size_t end, const icu::UnicodeString& ustring);
			TransductionOutputAnnotation(uima::AnnotationFS& fs);
			virtual ~TransductionOutputAnnotation();

			TransductionOutputAnnotation& operator =(const TransductionOutputAnnotation& model);

			uima::UnicodeStringRef getOutput() const;
			void setOutput(const icu::UnicodeString& ustring);

			friend std::ostream& operator <<(std::ostream& os, const TransductionOutputAnnotation& annotation);

		private:
			static uima::Type tTransductionOutputAnnotation;
			static uima::Feature fOutput;
		};

	}

}

#endif /* TRANSDUCTIONOUTPUTANNOTATION_H_ */

/*
 * AnnotationWrapper.h
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef ANNOTATIONWRAPPER_H_
#define ANNOTATIONWRAPPER_H_

#include <uima/api.hpp>
#include <iostream>

namespace unitexcpp
{

	namespace annotation
	{

		class AnnotationWrapper
		{
		protected:
			AnnotationWrapper();
			AnnotationWrapper(uima::CAS& aCas);
			AnnotationWrapper(uima::CAS* aCasPtr);
			AnnotationWrapper(uima::AnnotationFS& anAnnotation);
			virtual ~AnnotationWrapper();

		public:
			AnnotationWrapper& operator =(const AnnotationWrapper& model);
			void setAnnotation(uima::AnnotationFS& anAnnotation);

			bool isValid() const;

			uima::CAS& getView() const;
			int32_t getBegin() const;
			int32_t getEnd() const;
			uima::AnnotationFS getAnnotation() const;
			uima::UnicodeStringRef getCoveredText() const;
			void getCoveredText(icu::UnicodeString& ustring) const;

			int32_t distanceTo(const AnnotationWrapper& other) const;
			bool isBefore(const AnnotationWrapper& other) const;
			bool isAfter(const AnnotationWrapper& other) const;

			friend std::ostream& operator <<(std::ostream& os, const AnnotationWrapper& annotation);

		protected:
			uima::CAS* pCas;
			uima::AnnotationFS annotation;
			bool valid;
		};

	}

}

#endif /* ANNOTATIONWRAPPER_H_ */

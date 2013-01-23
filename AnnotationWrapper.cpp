/*
 * AnnotationWrapper.cpp
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "AnnotationWrapper.h"
#include "UnitexAnnotatorCpp.h"
#include <unicode/ustream.h>

using namespace std;
using namespace uima;
using namespace icu;

namespace unitexcpp
{

	namespace annotation
	{

		/*!
		 * Default constructor builds an invalid wrapper.
		 */
		AnnotationWrapper::AnnotationWrapper()
		{
			valid = false;
		}

		AnnotationWrapper::AnnotationWrapper(CAS& aCas) :
				pCas(&aCas)
		{
			valid = true;
		}

		AnnotationWrapper::AnnotationWrapper(CAS* aCasPtr) :
				pCas(aCasPtr)
		{
			valid = true;
		}

		AnnotationWrapper::AnnotationWrapper(AnnotationFS& anAnnotation)
		{
			setAnnotation(anAnnotation);
			valid = true;
		}

		AnnotationWrapper::~AnnotationWrapper()
		{
		}

		void AnnotationWrapper::setAnnotation(AnnotationFS& anAnnotation)
		{
			annotation = anAnnotation;
			pCas = anAnnotation.getView();
		}

		/**
		 * Shallow copy operator.
		 * Only the annotation is copied, no new annotation is created.
		 */
		AnnotationWrapper& AnnotationWrapper::operator =(const AnnotationWrapper& model)
		{
			annotation = model.annotation;
			pCas = model.pCas;
			return *this;
		}

		bool AnnotationWrapper::isValid() const
		{
			return valid;
		}

		CAS& AnnotationWrapper::getView() const
		{
			return *pCas;
		}

		int32_t AnnotationWrapper::getBegin() const
		{
			return annotation.getBeginPosition();
		}

		int32_t AnnotationWrapper::getEnd() const
		{
			return annotation.getEndPosition();
		}

		AnnotationFS AnnotationWrapper::getAnnotation() const
		{
			return annotation;
		}

		UnicodeStringRef AnnotationWrapper::getCoveredText() const
		{
			return annotation.getCoveredText();
		}

		void AnnotationWrapper::getCoveredText(UnicodeString& ustring) const
		{
			ustring.remove();
			UnicodeStringRef rString = annotation.getCoveredText();
			rString.extract(0, rString.length(), ustring);
		}

		int32_t AnnotationWrapper::distanceTo(const AnnotationWrapper& other) const
		{
			return other.getBegin() - getBegin();
		}

		bool AnnotationWrapper::isBefore(const AnnotationWrapper& other) const
		{
			return getBegin() < other.getBegin();
		}

		bool AnnotationWrapper::isAfter(const AnnotationWrapper& other) const
		{
			return getBegin() > other.getBegin();
		}

		ostream& operator <<(ostream& os, const AnnotationWrapper& annotation)
		{
			os << "(" << annotation.getBegin() << "," << annotation.getEnd() << ":" << annotation.getCoveredText() << ")";
			return os;
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

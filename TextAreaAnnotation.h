/*
 * TextArea.h
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TEXTAREAANNOTATION_H_
#define TEXTAREAANNOTATION_H_

#include "AnnotationWrapper.h"

namespace unitexcpp
{

	namespace annotation
	{

		class TextAreaAnnotation: public unitexcpp::annotation::AnnotationWrapper
		{
		protected:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		protected:
			TextAreaAnnotation();
			TextAreaAnnotation(uima::CAS& aCas);
			TextAreaAnnotation(uima::AnnotationFS& fs);
			virtual ~TextAreaAnnotation();

		public:
			TextAreaAnnotation& operator =(const TextAreaAnnotation& model);

			TextAreaAnnotation getPrevious() const;
			void setPrevious(const TextAreaAnnotation& ta);
			TextAreaAnnotation getNext() const;
			void setNext(const TextAreaAnnotation& ta);

		private:
			static uima::Type tTextAreaAnnotation;
			static uima::Feature fPrevious;
			static uima::Feature fNext;
		};

	}

}

#endif /* TEXTAREAANNOTATION_H_ */

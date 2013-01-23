/*
 * ParagraphAnnotation.h
 *
 *  Created on: 14 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef PARAGRAPHANNOTATION_H_
#define PARAGRAPHANNOTATION_H_

#include "ContextAreaAnnotation.h"

namespace unitexcpp
{

	namespace annotation
	{

		class TokenAnnotation;

		class ParagraphAnnotation: public unitexcpp::annotation::ContextAreaAnnotation
		{
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		public:
			ParagraphAnnotation();
			ParagraphAnnotation(uima::CAS& aCas, int begin, int end, const TokenAnnotation& firstToken, const TokenAnnotation& lastToken);
			ParagraphAnnotation(uima::AnnotationFS& anAnnotation);
			virtual ~ParagraphAnnotation();

			ParagraphAnnotation& operator =(const ParagraphAnnotation& model);

			template <typename ParagraphContainer>
			static void getAllParagraphsInView(uima::CAS& aCas, ParagraphContainer& aContainer);

			static ParagraphAnnotation getParagraphCovering(const AnnotationWrapper& annotationWrapper, uima::CAS& aCas);

		private:
			static uima::Type tParagraphAnnotation;
			static uima::Feature fSentences;
		};

	}

}

#endif /* PARAGRAPHANNOTATION_H_ */

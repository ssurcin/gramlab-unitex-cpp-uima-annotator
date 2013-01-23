/*
 * SentenceAnnotation.h
 *
 *  Created on: 15 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef SENTENCEANNOTATION_H_
#define SENTENCEANNOTATION_H_

#include "ContextAreaAnnotation.h"
#include "ParagraphAnnotation.h"
#include "TokenAnnotation.h"

namespace unitexcpp
{

	namespace annotation
	{

		class SentenceAnnotation: public ContextAreaAnnotation
		{
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		public:
			SentenceAnnotation();
			SentenceAnnotation(uima::CAS& aCas, size_t begin, size_t end, const TokenAnnotation& firstToken, const TokenAnnotation& lastToken);
			SentenceAnnotation(uima::CAS& aCas, size_t begin, size_t end, const TokenAnnotation& firstToken, const TokenAnnotation& lastToken, const ParagraphAnnotation& paragraph);
			SentenceAnnotation(uima::AnnotationFS& anAnnotation);
			virtual ~SentenceAnnotation();

			SentenceAnnotation& operator =(const SentenceAnnotation& model);

			void setParagraph(const ParagraphAnnotation& paragraph);
			ParagraphAnnotation getParagraph() const;

			static SentenceAnnotation findSentenceContainingToken(TokenAnnotation& token);

			friend std::ostream& operator <<(std::ostream& os, const SentenceAnnotation& sentence);

		private:
			static uima::Type tSentenceAnnotation;
			static uima::Feature fParagraph;
		};

	}

}

#endif /* SENTENCEANNOTATION_H_ */

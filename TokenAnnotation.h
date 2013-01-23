/*
 * Token.h
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TOKEN_H_
#define TOKEN_H_

#include "TextAreaAnnotation.h"
#include <list>

namespace unitexcpp
{

	namespace annotation
	{

		class TokenAnnotation: public unitexcpp::annotation::TextAreaAnnotation
		{
		public:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		public:
			TokenAnnotation();
			TokenAnnotation(uima::CAS& aCas, int32_t begin, int32_t end, const icu::UnicodeString& text, size_t nIndex);
			TokenAnnotation(uima::AnnotationFS& anAnnotation);
			virtual ~TokenAnnotation();

			TokenAnnotation& operator =(const TokenAnnotation& model);

			uima::UnicodeStringRef token() const;
			void token(const icu::UnicodeString& text);
			size_t getIndex() const;
			void setIndex(size_t n);

			static TokenAnnotation tokenStartingAtOffsetInView(uint32_t offset, uima::CAS& view);
			static TokenAnnotation tokenEndingAtOffsetInView(uint32_t offset, uima::CAS& view);
			static void getTokensInView(uima::CAS& view, std::list<TokenAnnotation>& tokens);

			friend std::ostream& operator <<(std::ostream& os, const TokenAnnotation& token);

		private:
			bool m_isEmpty;

			static uima::Type tTokenAnnotation;
			static uima::Feature fToken;
			static uima::Feature fIndex;
		};

	}

}

#endif /* TOKEN_H_ */

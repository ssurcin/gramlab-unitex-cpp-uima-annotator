/*
 * KwagaTokenizer.h
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef KWAGATOKENIZER_H_
#define KWAGATOKENIZER_H_

#include "AbstractTokenizer.h"
#include <map>
#include <vector>
#include <uima/api.hpp>
#include <boost/tuple/tuple.hpp>
#include "TokenAnnotation.h"
#include "ParagraphAnnotation.h"
#include "SentenceAnnotation.h"
#include "UnitexOutputOffsetConverter.h"
#include "UnitexEngine.h"

namespace uima
{
	class UnitexAnnotatorCpp;
}

namespace unitexcpp
{

	namespace tokenize
	{

		/**
		 * Private class implementing a simple tokenization algorithm, while
		 * waiting for Unitex to be enhanced to provide its own tokenization.
		 */
		class StringTokenizer
		{
		public:
			class TokenLocation
			{
				icu::UnicodeString const* pString;
				int32_t begin;
				int32_t end;

			public:
				TokenLocation(const icu::UnicodeString& aString, int32_t nBegin, int32_t nEnd):
					pString(&aString), begin(nBegin), end(nEnd)
				{}
				virtual ~TokenLocation()
				{}

				TokenLocation& operator =(const TokenLocation& model)
				{
					pString = model.pString;
					begin = model.begin;
					end = model.end;
					return *this;
				}

				const int32_t getBegin() const { return begin; }
				const int32_t getEnd() const { return end; }
				const int32_t size() const { return end - begin + 1; }
				icu::UnicodeString getString() const
				{
					icu::UnicodeString str;
					pString->extract(begin, size(), str);
					return str;
				}

				friend std::ostream& operator <<(std::ostream& os, const TokenLocation& tl);
			};

			StringTokenizer(const icu::UnicodeString& uString);
			virtual ~StringTokenizer();

			const std::vector<TokenLocation>& getTokens() const;
			const std::vector<int32_t>& getTokenIndexes() const;

		private:
			int32_t strpbrk(int32_t offset);
			void buildTokens();
			void buildTokenIndexes();

		private:
			const icu::UnicodeString& material;
			static icu::UnicodeString separators;
			static icu::UnicodeString tokenseparators;
			int32_t tokensReturned;
			std::vector<TokenLocation> tokens;
			std::vector<int32_t> tokenIndexes;
		};

		std::ostream& operator <<(std::ostream& os, const StringTokenizer::TokenLocation& tl);

		/**
		 * This class is a delegate used by UnitexAnnotator in order to tokenize the
		 * real mail body view and the body view. It also marks sentences and paragraphs
		 * in both views.
		 *
		 * By default, the marking of tokens, sentences and paragraphs are performed
		 * only in the real mail body view. But if the system property
		 * com.kwaga.pulse.mode=debug then the marking is also performed in the body
		 * view, so that we can visually track the annotations in debug tools such as
		 * the CAS Visual Debugger or the XMI Annotation Viewer.
		 *
		 * @author surcin@kwaga.com
		 *
		 */
		class KwagaTokenizer: public unitexcpp::tokenize::AbstractTokenizer
		{
		public:
			KwagaTokenizer(const uima::UnitexAnnotatorCpp& anAnnotator, const unitexcpp::engine::UnitexEngine& anEngine);
			virtual ~KwagaTokenizer();

			const icu::UnicodeString& getNormalizedText() const;
			size_t convertOffset(size_t offset, bool askEnd =false) const;

		private:
			void extractTokens();
			void setNormalizedText();
			void setOriginalText(uima::UnicodeStringRef aStringRef);
			bool isEmptyToken(const StringTokenizer::TokenLocation& aTokenLocation) const;
			void createParagraphAnnotations();
			void createSentenceAnnotations(unitexcpp::annotation::ParagraphAnnotation& rmbPar, int32_t normBegin, int32_t normEnd);

			unitexcpp::annotation::TokenAnnotation findTokenStartingAt(int32_t offset) const;
			unitexcpp::annotation::TokenAnnotation findTokenEndingAt(int32_t offset) const;

		private:
			const uima::UnitexAnnotatorCpp& annotator;
			const unitexcpp::engine::UnitexEngine& unitexEngine;

			// The normalized text and its tokens
			icu::UnicodeString ustrNormalizedText;
			std::vector<StringTokenizer::TokenLocation> sntTokenLocations;
			std::vector<int32_t> sntTokenIndexes;

			// The original text and its tokens
			icu::UnicodeString ustrOriginalText;
			std::vector<StringTokenizer::TokenLocation> originalTokenLocations;
			std::vector<int32_t> originalTokenIndexes;

			std::vector<unitexcpp::annotation::ParagraphAnnotation> rmbParagraphs;

			// This table maps a token in the original view with its offsets in the
			// "normalized text" produced by Unitex preprocessing.
			typedef std::map<uima::AnnotationFS, std::pair<int32_t, int32_t> > MapToken2Offsets;
			MapToken2Offsets mapOrigToken2NormToken;

			typedef std::map<int32_t, uima::AnnotationFS> MapOffset2Token;
			MapOffset2Token mapSntStartOffset2OrigToken;
			MapOffset2Token mapSntEndOffset2OrigToken;

			UnitexOutputOffsetConverter* pOffsetConverter;
		};

	}

}

#endif /* KWAGATOKENIZER_H_ */

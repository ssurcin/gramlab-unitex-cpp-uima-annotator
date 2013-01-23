/*
 * UnitexTokenizer.h
 *
 *  Created on: 12 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef UNITEXTOKENIZER_H_
#define UNITEXTOKENIZER_H_

#include "UnitexEngine.h"
#include "TokenAnnotation.h"
#include "ParagraphAnnotation.h"
#include "SentenceAnnotation.h"
#include <boost/tuple/tuple.hpp>

namespace uima
{
	class UnitexAnnotatorCpp;
}

namespace unitexcpp
{

	namespace tokenize
	{

		class SntToken
		{
		private:
			bool m_empty;
			int32_t m_tokenIndex;
			int32_t m_formIndex;
			UnicodeString m_form;
			int32_t m_start;
			int32_t m_end;

		private:
			SntToken();

		public:
			static SntToken EmptyToken;

			SntToken(int32_t tokenIndex, int32_t formIndex, int32_t start, int32_t end, uima::UnicodeStringRef form);
			SntToken(const SntToken& model);
			virtual ~SntToken();

			SntToken& operator=(const SntToken& model);

			bool isEmpty() const;
			int32_t tokenIndex() const;
			int32_t formIndex() const;
			int32_t start() const;
			int32_t end() const;
			const icu::UnicodeString& form() const;

			int32_t length() const;

			friend std::ostream& operator<<(std::ostream& os, const SntToken& token);
		};

		/**
		 * A class implementing algorithms to create UIMA token annotations from Unitex generated tokens.
		 */
		class UnitexTokenizer
		{
		public:
			UnitexTokenizer(
					const uima::UnitexAnnotatorCpp& anAnnotator,
					const unitexcpp::UnicodeStringVector& fakeTokenParameter,
					const unitexcpp::engine::UnitexEngine& unitexEngine,
					size_t offsetInDocument,
					UnicodeStringRef text);
			virtual ~UnitexTokenizer();

			size_t tokenize(size_t tokenIndexOffset);
			size_t countTokens() const;

			size_t convertOffset(size_t offset, bool askEnd = false) const;

			unitexcpp::annotation::TokenAnnotation* getTokenByIndex(int32_t index);

		private:
			void readTokenList();
			void readSntTokens();

			boost::tuple<int32_t, int32_t, int32_t> tokenCoordinates(const icu::UnicodeString& line);
			int32_t fakeTokenStartingAtLine(const unitexcpp::UnicodeStringVector& lines, const int32_t n);
			void concatenateWithTokenFormAtLine(icu::UnicodeString& dest, const unitexcpp::UnicodeStringVector& lines, const int32_t n);
			size_t isFakeTokenAt(UnicodeStringRef rustr, size_t position);

			void createParagraphAnnotations();
			void createSentenceAnnotations();

		private:
			/**
			 * A reference to the annotator using this tokenizer.
			 */
			const uima::UnitexAnnotatorCpp& m_annotator;
			/**
			 * The Unitex instance to use for tokenization.
			 */
			const unitexcpp::engine::UnitexEngine& m_unitexEngine;
			/**
			 * A reference to the UIMA view containing the document.
			 */
			uima::CAS& m_view;
			/**
			 * An array of token sequences that must not be taken into account.
			 */
			const unitexcpp::UnicodeStringVector& m_fakeTokens;
			/**
			 * An offset to be added to all tokens, representing the offset of the
			 * current tokenized area wrt to the document's actual beginning.
			 */
			const std::size_t m_offset;
			/**
			 * The input text being tokenized.
			 */
			const uima::UnicodeStringRef m_text;
			/**
			 * The text normalized and split into sentences by Unitex.
			 */
			icu::UnicodeString m_sntText;

			/**
			 * Euh...
			 */
			std::vector<size_t> m_inputWithoutFakeTokens;
			/**
			 * All token forms found in this current tokenization.
			 */
			std::vector<UnicodeString> m_tokenForms;
			/**
			 * The data of tokens found in this current tokenization.
			 */
			std::vector<SntToken> m_tokens;
			/**
			 * A way to track sentence frontiers by remembering the last token of a sentence,
			 * and the first token of the next one.
			 */
			std::vector<std::pair<SntToken, SntToken> > m_sentenceMarkers;

			/**
			 * The index of the first line to keep...
			 */
			int32_t m_firstLineToKeep;
			/**
			 * The number of tokens created.
			 */
			size_t m_nbTokens;
			/**
			 * A way to keep track of all Token annotations created, against their token index.
			 */
			std::map<int32_t, unitexcpp::annotation::TokenAnnotation> m_tokensByIndex;
		};

	}

}

#endif /* UNITEXTOKENIZER_H_ */

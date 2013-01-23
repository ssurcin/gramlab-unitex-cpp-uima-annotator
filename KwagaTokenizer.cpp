/*
 * KwagaTokenizer.cpp
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#include "KwagaTokenizer.h"
#include "UnitexAnnotatorCpp.h"
#include "UnitexEngine.h"
#include "UnitexException.h"
#include "TokenAnnotation.h"
#include "ParagraphAnnotation.h"
#include "SentenceAnnotation.h"
#include "TextArea.h"
#include "Utils.h"

#include <uima/api.hpp>
#include <unicode/regex.h>
#include <unicode/ustream.h>
#include <vector>

using namespace std;
using namespace uima;
using namespace icu;
using namespace boost::tuples;
using namespace unitexcpp;
using namespace unitexcpp::annotation;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace tokenize
	{

		KwagaTokenizer::KwagaTokenizer(const UnitexAnnotatorCpp& anAnnotator, const UnitexEngine& anEngine) :
			annotator(anAnnotator), unitexEngine(anEngine)
		{
			pOffsetConverter = NULL;
		}

		KwagaTokenizer::~KwagaTokenizer()
		{
			if (pOffsetConverter)
				delete pOffsetConverter;
		}

		const UnicodeString& KwagaTokenizer::getNormalizedText() const
		{
			return ustrNormalizedText;
		}

		void KwagaTokenizer::extractTokens()
		{
			bool debug = false;
			CAS& view = annotator.getView();

			// Get the SNT normalized text
			setNormalizedText();

			// Get the original document text
			setOriginalText(view.getDocumentText());

			if (pOffsetConverter)
				delete pOffsetConverter;
			pOffsetConverter = new UnitexOutputOffsetConverter(getNormalizedText(), view);

			if (originalTokenLocations.size() == 0)
				return;

			vector<annotation::TokenAnnotation> tokens;

			const int WINDOW_SIZE = 5;
			const int NB_SNT = sntTokenLocations.size();
			const int NB_ORIG = originalTokenLocations.size();

			int i = 0, j = 0;
			while ((i < NB_SNT) && (j < NB_ORIG)) {
				if (isEmptyToken(sntTokenLocations[i])) {
					i++;
				}
				else {
					// If the original token and the SNT token are not equal, it is
					// an effect of Replace
					if (!UnitexAnnotatorCpp::equalsModuloUnitexSpecialCharacterReplacements(originalTokenLocations[j].getString(), sntTokenLocations[i].getString())) {
						if (debug) {
							cout << "SNT offset " << i << "='" << sntTokenLocations[i].getString() << "' differs from original token " << j << "='" << originalTokenLocations[j].getString() << "'" << endl;
						}
						int iCommon = -1, jCommon = -1;
						bool breakLoop = false;
						// First we look in a range of windowSize tokens to the right if we
						// can find a common token
						int origStart = j + 1, sntStart = i + 1;
						int origLength = MIN(NB_ORIG, j + WINDOW_SIZE), sntLength = MIN(NB_SNT, i + WINDOW_SIZE);
						while ((MIN(NB_ORIG, NB_SNT) == NB_SNT) ? (sntStart < NB_SNT) : (origStart < NB_ORIG)) {
							// Loop on the tokens (SNT first, then original) to try
							// and find a common string
							for (int jj = origStart; jj < origLength; jj++) {
								breakLoop = false;
								for (int ii = sntStart; ii < sntLength; ii++) {
									//System.out.printf("jj=%d/%d, ii=%d/%d\n", jj, origLength, ii, sntLength);
									if (UnitexAnnotatorCpp::equalsModuloUnitexSpecialCharacterReplacements(originalTokenLocations[jj].getString(), sntTokenLocations[ii].getString())) {
										if (debug) {
											cout << "First common index for SNT=" << ii << " and orig=" << jj << " -> '" << originalTokenLocations[jj].getString() << "'" << endl;
										}
										iCommon = ii;
										jCommon = jj;
										breakLoop = true;
										break;
									}
								}
								if (breakLoop)
									break;
							}
							// If we haven't found a common token, we had windowSize more
							// tokens to the window
							if (iCommon == -1) {
								origStart = origLength;
								sntStart = sntLength;
								origLength = MIN(NB_ORIG, origLength + WINDOW_SIZE);
								sntLength = MIN(NB_SNT, sntLength + WINDOW_SIZE);
							}
							else {
								break;
							}
						}

						// If we have not found a common token, assume that the end
						// of the inconsistent
						// area is the last token.
						if (iCommon == -1) {
							iCommon = NB_SNT;
							jCommon = NB_ORIG;
						}

						int fakeTokenBegin = originalTokenLocations[j].getBegin() + offset;
						int fakeTokenEnd = originalTokenLocations[jCommon - 1].getEnd() + 1 + offset;

						UnicodeString originalToken;
						ustrOriginalText.extractBetween(originalTokenLocations[j].getBegin(), originalTokenLocations[jCommon - 1].getEnd() + 1, originalToken);
						TokenAnnotation token(annotator.getView(), fakeTokenBegin, fakeTokenEnd, originalToken, j);

						mapOrigToken2NormToken[token.getAnnotation()] = make_pair(sntTokenLocations[i].getBegin(), sntTokenLocations[iCommon - 1].getEnd() + 1);

						mapSntStartOffset2OrigToken[sntTokenLocations[i].getBegin()] =  token.getAnnotation();
						mapSntEndOffset2OrigToken[sntTokenLocations[iCommon - 1].getEnd()] = token.getAnnotation();

						i = iCommon;
						j = jCommon;
						if (debug)
							cout << "Now SNT index=" << i << " and orig index=" << j << endl;
					}
					else {
						int32_t tokenBegin = originalTokenLocations[j].getBegin();
						int32_t tokenEnd = originalTokenLocations[j].getEnd();

						TokenAnnotation token(view, tokenBegin, tokenEnd + 1, originalTokenLocations[j].getString(), j);
						tokens.push_back(token);

						/*
						 ostringstream oss;
						 oss << "Created token (" << tokenBegin << ", " << tokenEnd << ")=\"" << rmbToken.token() << "\"";
						 annotator.getLogger().logMessage(oss.str());
						 */

						if (tokens.size() > 1) {
							annotation::TokenAnnotation previousRmbToken = tokens[tokens.size() - 2];
							token.setPrevious(previousRmbToken);
							previousRmbToken.setNext(token);
						}

						int32_t sntBegin = sntTokenLocations[i].getBegin();
						int32_t sntEnd = sntTokenLocations[i].getEnd() + 1;
						mapOrigToken2NormToken[token.getAnnotation()] = make_pair(sntBegin, sntEnd);
						//cout << "map RMB token " << rmbToken << " -> (" << sntBegin << "," << sntEnd << ")" << endl;

						mapSntStartOffset2OrigToken[sntBegin] = token.getAnnotation();
						mapSntEndOffset2OrigToken[sntEnd] = token.getAnnotation();
						i++;
						j++;
					}
				}
			}

			// Mark paragraphs and sentences
			createParagraphAnnotations();
		}

		void KwagaTokenizer::setNormalizedText()
		{
			unitexEngine.getNormalizedText(ustrNormalizedText);
			StringTokenizer sntTokenizer(ustrNormalizedText);
			sntTokenLocations = sntTokenizer.getTokens();
			sntTokenIndexes = sntTokenizer.getTokenIndexes();
		}

		void KwagaTokenizer::setOriginalText(const UnicodeStringRef aStringRef)
		{
			aStringRef.extract(0, aStringRef.length(), ustrOriginalText);
			StringTokenizer tokenizer(ustrOriginalText);
			originalTokenLocations = tokenizer.getTokens();
			originalTokenIndexes = tokenizer.getTokenIndexes();
		}

		/**
		 * Test whether a token is considered as empty, i.e. it consists only of
		 * spaces or it is Unitex's sentence marker {S}
		 *
		 * @param aTokenLocation
		 *            a token location
		 * @return true if it is considered empty
		 */
		bool KwagaTokenizer::isEmptyToken(const StringTokenizer::TokenLocation& aTokenLocation) const
		{
			static RegexPattern* pSpaceRE = NULL;
			static UErrorCode error;

			if (!pSpaceRE) {
				pSpaceRE = RegexPattern::compile("\\s*|\\{S\\}|@@@", UREGEX_MULTILINE + UREGEX_DOTALL, error);
				if (U_FAILURE(error))
					throw UnitexException(u_errorName(error));
			}

			RegexMatcher* pMatcher = pSpaceRE->matcher(aTokenLocation.getString(), error);
			bool result = pMatcher->matches(error);
			delete pMatcher;

			return result;
		}

		/**
		 * Creates the paragraph annotations in the normalized view.
		 *
		 * \throws UnitexException
		 */
		void KwagaTokenizer::createParagraphAnnotations()
		{
			static RegexPattern* pParagraphRE = NULL;
			static RegexPattern* pBlankRE = NULL;
			static UErrorCode error;

			if (!pParagraphRE) {
				pParagraphRE = RegexPattern::compile("(\\s*\\n){2,}|\\n\\s*\\*\\s*\\n", UREGEX_MULTILINE, error);
				if (U_FAILURE(error))
					throw UnitexException(u_errorName(error));
				pBlankRE = RegexPattern::compile("(\\s*\\n*)*", UREGEX_MULTILINE, error);
				if (U_FAILURE(error))
					throw UnitexException(u_errorName(error));
			}

			TextArea area(ustrOriginalText);
			vector<TextArea> paragraphAreas = area.counterpart(pParagraphRE);

			CAS& view = annotator.getView();

			rmbParagraphs.clear();
			for (vector<TextArea>::const_iterator it = paragraphAreas.begin(); it != paragraphAreas.end(); it++) {
				const TextArea& paragraph = *it;

				// Don't process empty paragraphs
				RegexMatcher* pMatcher = pBlankRE->matcher(paragraph.getText(), error);
				if (U_FAILURE(error))
					throw UnitexException(u_errorName(error));
				if (pMatcher->matches(error))
					continue;

				annotation::TokenAnnotation firstToken = annotation::TokenAnnotation::tokenStartingAtOffsetInView(paragraph.getBegin(), view);
				if (!firstToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find token starting at " << paragraph.getBegin() << " in RMB";
					throw UnitexException(oss.str());
				}
				if (firstToken.getBegin() > paragraph.getEnd()) {
					annotator.getLogger().logMessage("First token found after current paragraph boundaries => skipping");
					continue;
				}

				annotation::TokenAnnotation lastToken = annotation::TokenAnnotation::tokenEndingAtOffsetInView(paragraph.getEnd(), view);
				if (!lastToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find token ending at " << paragraph.getEnd() << " in RMB";
					throw UnitexException(oss.str());
				}

				annotation::ParagraphAnnotation rmbPar(view, firstToken.getBegin(), lastToken.getEnd(), firstToken, lastToken);
				rmbParagraphs.push_back(rmbPar);

				// annotator.getLogger().logMessage("Created paragraph annotation:\n" + rmbPar.toString());

				// Create sentence annotations within this paragraph
				pair<int32_t, int32_t> normCoord1 = mapOrigToken2NormToken[firstToken.getAnnotation()];
				pair<int32_t, int32_t> normCoord2 = mapOrigToken2NormToken[lastToken.getAnnotation()];
				createSentenceAnnotations(rmbPar, normCoord1.first, normCoord2.second);
			}

		}

		/**
		 * Builds a version of a text without initial mail quoting signs (typically
		 * ">" but it can be different).
		 *
		 * \param result a reference to the string where to store the result
		 * \param text the text containing the quoting signs
		 * \param quoteSign the quote sign to remove
		 */
		static UnicodeString removeLeadingQuoteSigns(const UnicodeString& text, const UnicodeString& quoteSign)
		{
			UnicodeStringlist lines, cleanLines;
			splitRegex(lines, text, "\\n");
			for (UnicodeStringlist::const_iterator it = lines.begin(); it != lines.end(); it++) {
				UnicodeString pattern = UNICODE_STRING_SIMPLE("^(").append(quoteSign).append("\\s*)*");
				UErrorCode error;
				RegexMatcher matcher(pattern, *it, 0, error);
				UnicodeString cleanLine = matcher.replaceFirst("", error).trim();
				if (cleanLine.length() > 0)
					cleanLines.push_back(cleanLine);
			}
			return join(lines, "\n");
		}

		/**
		 * Tests whether we are on a Unitex mark (sentence mark of subject line
		 * mark) and returns the length of the mark if so, or 0 it we are not on a
		 * mark.
		 *
		 * \param text the reference text
		 * \param offset the current offset
		 * \param limit a limit to the right offset
		 * \return the number of characters to skip when finding a Unitex specific mark
		 */
		static int32_t isUnitexMark(const UnicodeString& text, int32_t offset, int32_t limit)
		{
			int32_t length = MIN((int32_t)text.length(), limit + 1);
			if (offset + 3 < length) {
				UnicodeString substring;
				text.extract(offset, 3, substring);
				if (substring.compare("{S}") == 0)
					return 3;
			}
			if (offset + 9 < length) {
				UnicodeString substring;
				text.extract(offset, 9, substring);
				if (substring.compare("[SUBJECT]") == 0)
					return 9;
			}
			return 0;
		}

		/**
		 * Finds the offset of the next line start in a text.
		 *
		 * \param text the reference text
		 * \param currentOffset the current offset
		 * \param limit a limit to the right offset
		 * \return the offset of the 1st character after an EOL or a Unitex sentence marker
		 */
		static int32_t skipToNextLine(const UnicodeString& text, int32_t currentOffset, int32_t limit)
		{
			static RegexMatcher* pMatcher = NULL;
			UErrorCode error = U_ZERO_ERROR;

			if (currentOffset >= text.length())
				return limit + 1;

			if (!pMatcher) {
				pMatcher = new RegexMatcher("(\\n(\\{S\\})?)+", 0, error);
				if (U_FAILURE(error)) {
					ostringstream oss;
					oss << "skipToNextLine error building regex " << u_errorName(error);
					throw UnitexException(oss.str());
				}
			}

			pMatcher->reset(text);
			bool found = pMatcher->find(currentOffset, error);
			if (U_FAILURE(error)) {
				ostringstream oss;
				oss << "skipToNextLine error finding newline " << u_errorName(error);
				throw UnitexException(oss.str());
			}

			if (!found)
				return limit + 1;

			int32_t offset = pMatcher->end(error);
			if (offset > limit)
				return limit + 1;

			return offset;
		}

		/**
		 * Finds the offset immediately after the next Unitex sentence mark.
		 *
		 * \param text the reference text
		 * \param currentOffset the current offset
		 * \param limit a limit to the right offset
		 * \return the offset of the 1st character after a Unitex sentence marker
		 */
		static int32_t skipUnitexMark(const UnicodeString& text, int32_t currentOffset, int32_t limit)
		{
			if (currentOffset >= text.length())
				return limit + 1;

			int32_t offset = currentOffset;
			int32_t unitexMark = isUnitexMark(text, offset, limit);
			if (unitexMark > 0)
				return offset + unitexMark;
			return offset;
		}

		/**
		 * Finds the offset immediately after the next a new line followed by a
		 * Unitex sentence mark.
		 *
		 * \param text the reference text
		 * \param currentOffset the current offset
		 * \param limit a limit to the right offset
		 * \return the offset of the 1st character after a Unitex sentence marker
		 */
		static int32_t skipToNextLineAndUnitexMark(const UnicodeString& text, int32_t currentOffset, int32_t limit)
		{
			return skipUnitexMark(text, skipToNextLine(text, currentOffset, limit), limit);
		}

		/**
		 * Finds the last offset of a line before an EOL.
		 *
		 * \param text the reference text
		 * \param currentOffset the current offset
		 * \param limit a limit to the right offset
		 * \return the offset of the last character before the next EOL or Unitex sentence marker
		 */
		static int32_t skipToNextLineBeforeEOL(const UnicodeString& text, int32_t currentOffset, int32_t limit)
		{
			static RegexMatcher* pMatcher = NULL;
			UErrorCode error = U_ZERO_ERROR;

			if (currentOffset >= text.length())
				return limit + 1;

			if (!pMatcher) {
				pMatcher = new RegexMatcher("\\s*\\n+", 0, error);
				if (U_FAILURE(error)) {
					ostringstream oss;
					oss << "skipToNextLineBeforeEOL error building regex " << u_errorName(error);
					throw UnitexException(oss.str());
				}
			}

			pMatcher->reset(text);
			bool found = pMatcher->find(currentOffset, error);
			if (U_FAILURE(error)) {
				ostringstream oss;
				oss << "skipToNextLine error finding newline " << u_errorName(error);
				throw UnitexException(oss.str());
			}

			if (!found)
				return limit + 1;

			int32_t offset = pMatcher->start(error) - 1;
			if (offset > limit)
				return limit + 1;

			return offset;
		}

		/**
		 * Creates the sentence annotations in the normalized view and in the body
		 * view inside an already marked paragraph. To do that, we use the sentence
		 * marks {S} introduced by Unitex's Sentence.grf preprocessing.
		 *
		 * \param rmbPar the current paragraph in the RMB
		 * \param normBegin the paragraph's begin offset in the normalized text
		 * \param normEnd the paragraph's end offset in the normalized text
		 * \throws UnitexException
		 */
		void KwagaTokenizer::createSentenceAnnotations(annotation::ParagraphAnnotation& rmbPar, int32_t normBegin, int32_t normEnd)
		{
			const int32_t endParagraph = normEnd;

			CAS& view = annotator.getView();

			UnicodeString para;
			ustrNormalizedText.extractBetween(normBegin, normEnd, para);

			// Split the paragraph into sentences
			UnicodeStringlist sentences;
			splitRegex(sentences, para, "\\{S\\}|\\[SUBJECT\\]");

			int32_t nCurPos = normBegin;

			vector<annotation::SentenceAnnotation> rmbSentences;
			UErrorCode error = U_ZERO_ERROR;
			RegexMatcher paraMatcher("\\[Para\\]", 0, error);
			if (U_FAILURE(error))
				throw UnitexException(u_errorName(error));

			for (UnicodeStringlist::const_iterator it = sentences.begin(); it != sentences.end(); it++) {
				const UnicodeString& sentence = *it;

				// Do not process empty sentences but increase current offset
				UnicodeString trimmedSentence = removeLeadingQuoteSigns(sentence, ">");
				paraMatcher.reset(trimmedSentence);
				trimmedSentence = paraMatcher.replaceAll("", error).trim();
				if (trimmedSentence.isEmpty()) {
					nCurPos = skipToNextLineAndUnitexMark(ustrNormalizedText, nCurPos, endParagraph);
					continue;
				}

				UnicodeStringlist lines;
				splitRegex(lines, sentence, "\\n");

				int32_t nSentBegin = nCurPos;

				//TokenAnnotation firstToken = annotation::TokenAnnotation::tokenStartingAtOffsetInView(rmbSentBegin, rmbView);
				TokenAnnotation firstToken = findTokenStartingAt(nSentBegin);
				if (!firstToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find RMB token starting at offset " << nSentBegin;
					throw UnitexException(oss.str());
				}

				int32_t currentOffset = nSentBegin;

				// Skip intermediary lines
				if (lines.size() > 1) {
					currentOffset = skipToNextLineAndUnitexMark(ustrNormalizedText, nSentBegin, endParagraph);
					if (lines.size() > 2) {
						for (size_t i = 1; i < lines.size() - 1; i++)
							currentOffset = skipToNextLineAndUnitexMark(ustrNormalizedText, currentOffset, endParagraph);
					}
				}

				int32_t nSentEnd = currentOffset;
				nSentEnd = skipToNextLineBeforeEOL(ustrNormalizedText, nSentEnd, endParagraph);

				//annotation::TokenAnnotation lastToken = annotation::TokenAnnotation::tokenEndingAtOffsetInView(rmbSentEnd, rmbView);
				TokenAnnotation lastToken = findTokenEndingAt(nSentEnd);
				if (!lastToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find RMB token ending at offset " << nSentEnd;
					throw UnitexException(oss.str());
				}

				nCurPos = skipToNextLineAndUnitexMark(ustrNormalizedText, nSentEnd, endParagraph);

				annotation::SentenceAnnotation rmbSentence(view, firstToken.getBegin(), lastToken.getEnd(), firstToken, lastToken, rmbPar);
				rmbSentences.push_back(rmbSentence);
				/*
				 cout << "created sentence " << rmbSentence << endl;
				 cout << "  firstToken " << firstToken << " index=" << firstToken.getIndex() << endl;
				 cout << "  lastToken " << lastToken << " index=" << lastToken.getIndex() << endl;
				 */
			}
		}

		size_t KwagaTokenizer::convertOffset(size_t offset, bool askEnd) const
		{
			return pOffsetConverter->convert(offset);
		}

		/**
		 * Retrieve the token starting at the specified offset among the tokens of
		 * the RMB view. The offset is given in the so-called "normalized" view
		 * (i.e. the output of Unitex's preprocessing).
		 *
		 * \param offset
		 *            the offset in Unitex's preprocessing view
		 * \return the offset starting at the corresponding offset in the RMB view
		 *         (or an invalid token if not found)
		 */
		TokenAnnotation KwagaTokenizer::findTokenStartingAt(int32_t offset) const
		{
			list<TokenAnnotation> tokens;
			TokenAnnotation::getTokensInView(annotator.getView(), tokens);

			for (list<TokenAnnotation>::const_iterator it = tokens.begin(); it != tokens.end(); it++) {
				MapToken2Offsets::const_iterator jt = mapOrigToken2NormToken.find(it->getAnnotation());
				if (jt == mapOrigToken2NormToken.end()) {
					ostringstream oss;
					oss << "KwagaTokenizer::findTokenStartingAt cannot find coords for token " << *it;
					throw UnitexException(oss.str());
				}
				int32_t begin = jt->second.first;
				int32_t end = jt->second.second;
				if ((begin >= offset) || ((begin <= offset) && (end >= offset)))
					return TokenAnnotation(*it);
			}
			return TokenAnnotation();
		}

		/**
		 * Retrieve the token ending at the specified offset among the tokens of the
		 * RMB view. The offset is given in the so-called "normalized" view (i.e.
		 * the output of Unitex's preprocessing).
		 *
		 * \param offset
		 *            the offset in Unitex's preprocessing view
		 * \return the offset starting at the corresponding offset in the RMB view
		 *         (or an invalid token if not found)
		 */
		TokenAnnotation KwagaTokenizer::findTokenEndingAt(int32_t offset) const
		{
			list<TokenAnnotation> tokens;
			TokenAnnotation::getTokensInView(annotator.getView(), tokens);

			for (list<TokenAnnotation>::const_reverse_iterator it = tokens.rbegin(); it != tokens.rend(); it++) {
				MapToken2Offsets::const_iterator jt = mapOrigToken2NormToken.find(it->getAnnotation());
				if (jt == mapOrigToken2NormToken.end()) {
					ostringstream oss;
					oss << "KwagaTokenizer::findTokenEndingAt cannot find coords for token " << *it;
					throw UnitexException(oss.str());
				}
				// We return the token containing the offset, or the token ending
				// immediately
				// before the offset
				int32_t begin = jt->second.first;
				int32_t end = jt->second.second;
				if ((end <= offset) || ((begin <= offset) && (end >= offset)))
					return TokenAnnotation(*it);
			}
			return TokenAnnotation();
		}

		///////////////////////////////////////////////////////////////////////
		//
		// StringTokenizer
		//
		///////////////////////////////////////////////////////////////////////

		UnicodeString StringTokenizer::separators;
		UnicodeString StringTokenizer::tokenseparators;

		StringTokenizer::StringTokenizer(const UnicodeString& uString) :
			material(uString)
		{
			if (separators.isEmpty()) {
				separators = UNICODE_STRING_SIMPLE("*\t\n\r ,;?!:\"'`.");
				UChar _separators[] = { 0x00a0, 0x00ab, 0x00bb, 0x00bf, 0x201c, 0x201d, 0x2019, 0x2026, 0x2009, 0x2020, 0x2030 };
				separators.append(_separators, 11);

				tokenseparators = UNICODE_STRING_SIMPLE(",;?!:\"'`.");
				UChar _tokenseparators[] = { 0x00ab, 0x00bb, 0x00bf, 0x201c, 0x201d, 0x2019, 0X2026, 0x2020, 0x2030 };
				tokenseparators.append(_tokenseparators, 9);
			}
			tokensReturned = 0;
			buildTokens();
			buildTokenIndexes();
		}

		StringTokenizer::~StringTokenizer()
		{
		}

		const vector<StringTokenizer::TokenLocation>& StringTokenizer::getTokens() const
		{
			return tokens;
		}

		const vector<int32_t>& StringTokenizer::getTokenIndexes() const
		{
			return tokenIndexes;
		}

		int32_t StringTokenizer::strpbrk(int32_t offset)
		{
			int32_t length = material.length();
			int32_t patterns = separators.length();
			int32_t character, where = offset;

			while (where < length) {
				character = 0;
				while (character < patterns) {
					if (separators.char32At(character) == material.char32At(where)) {
						return where;
					}
					else {
						++character;
					}
				}
				++where;
			}
			return -1;
		}

		void StringTokenizer::buildTokens()
		{
			int32_t start = 0, end = 0, length = material.length();

			do {
				// skip Unitex sentence tag
				if (start + 2 < length) {
					UnicodeString first3;
					material.extract(start, 3, first3);
					if (first3 == UNICODE_STRING_SIMPLE("{S}")) {
						start += 3;
						continue;
					}
				}

				// skip beginning and end of line marks
				if (start + 8 < length) {
					UnicodeString first9;
					material.extract(start, 9, first9);
					if ((first9 == UNICODE_STRING_SIMPLE("[BEGLINE]")) || (first9 == UNICODE_STRING_SIMPLE("[ENDLINE]"))) {
						start += 9;
						continue;
					}
					// skip subject tag
					if (first9 == UNICODE_STRING_SIMPLE("[SUBJECT]")) {
						start += 9;
						continue;
					}
				}

				end = strpbrk(start);

				if (end == start) {
					if (tokenseparators.indexOf(material.char32At(end)) < 0) {
						++start;
						continue;
					}
					else {
						tokens.push_back(TokenLocation(material, start, end));
						start = end + 1;
						continue;
					}
				}
				if (end >= 0) {
					tokens.push_back(TokenLocation(material, start, end - 1));

					//if (this.material.charAt(end) == '\'') { tokens.add(new
					//TokenLocation(material, end, end)); }

					// start = end + 1;
					start = end;
				}
			} while ((end >= 0) && (start < length));

			if ((end < 0) && (start + 1 < length)) {
				tokens.push_back(TokenLocation(material, start, length - 1));
			}
		}

		void StringTokenizer::buildTokenIndexes()
		{
			tokenIndexes.clear();

			if (tokens.size() == 0)
				return;

			size_t tokenId = 0;
			TokenLocation* pCurrentToken = &tokens[tokenId];

			for (int32_t character = 0; character < material.length(); ++character) {

				if (character > pCurrentToken->getBegin()) {
					if (tokenId + 1 == tokens.size()) {
						tokenIndexes.push_back(-1);
						continue;
					}
					else
						pCurrentToken = &tokens[++tokenId];
				}

				if (character < pCurrentToken->getBegin())
					tokenIndexes.push_back(-1);
				else if (character <= pCurrentToken->getEnd())
					tokenIndexes.push_back(tokenId);
			}
		}

		ostream& operator <<(ostream& os, const StringTokenizer::TokenLocation& tl)
		{
			os << "('" << tl.getString() << "', " << tl.getBegin() << ", " << tl.getEnd() << ")";
			return os;
		}

	}
}

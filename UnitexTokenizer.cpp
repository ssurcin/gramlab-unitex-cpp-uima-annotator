/*
* UnitexTokenizer.cpp
*
*  Created on: 12 janv. 2011
*      Author: sylvainsurcin
*/

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "UnitexTokenizer.h"
#include "UnitexAnnotatorCpp.h"
#include "UnitexEngine.h"
#include "UnitexException.h"
#include "TokenAnnotation.h"
#include "ParagraphAnnotation.h"
#include "TextArea.h"
#include "Utils.h"
#include "FileUtils.h"
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/foreach.hpp>
#include <unicode/ustream.h>
#include <unicode/regex.h>
#include <algorithm>

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace uima;
using namespace unitexcpp;
using namespace unitexcpp::engine;
using namespace unitexcpp::annotation;
using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::tuples;
using namespace icu;

namespace unitexcpp
{

	namespace tokenize
	{

		///////////////////////////////////////////////////////////////////////
		//
		// class Token
		//
		///////////////////////////////////////////////////////////////////////

		SntToken SntToken::EmptyToken;

		SntToken::SntToken()
		{
			m_empty = true;
		}

		SntToken::SntToken(int32_t tokenIndex, int32_t formIndex, int32_t start, int32_t end, UnicodeStringRef form)
		{
			m_empty = false;
			m_tokenIndex = tokenIndex;
			m_formIndex = formIndex;
			m_start = start;
			m_end = end;
			form.extract(0, form.length(), m_form);
		}

		SntToken::SntToken(const SntToken& model)
		{
			*this = model;
		}

		SntToken::~SntToken()
		{
		}

		SntToken& SntToken::operator=(const SntToken& model)
		{
			if (model.m_empty)
				m_empty = true;
			else {
				m_empty = false;
				m_tokenIndex = model.m_tokenIndex;
				m_formIndex = model.m_formIndex;
				m_start = model.m_start;
				m_end = model.m_end;
				m_form = model.m_form;
			}
			return *this;
		}

		bool SntToken::isEmpty() const
		{
			return m_empty;
		}

		int32_t SntToken::tokenIndex() const
		{
			return m_tokenIndex;
		}

		int32_t SntToken::formIndex() const
		{
			return m_formIndex;
		}

		int32_t SntToken::start() const
		{
			return m_start;
		}

		int32_t SntToken::end() const
		{
			return m_end;
		}

		const UnicodeString& SntToken::form() const
		{
			return m_form;
		}

		int32_t SntToken::length() const
		{
			return end() - start();
		}

		ostream& operator<<(ostream& os, const SntToken& token)
		{
			if (token.isEmpty())
				os << "EMPTY_TOKEN";
			else
				os << "<tokenIndex=" << token.m_tokenIndex << ", formIndex=" << token.m_formIndex << ", form=\"" << token.m_form << "\", start=" << token.m_start << ", end=" << token.m_end << ">";
			return os;
		}

		///////////////////////////////////////////////////////////////////////
		//
		// class UnitexTokenizer
		//
		///////////////////////////////////////////////////////////////////////

		UnitexTokenizer::UnitexTokenizer(const UnitexAnnotatorCpp& anAnnotator, const UnicodeStringVector& fakeTokenParameter, const UnitexEngine& unitexEngine, size_t offsetInDocument,
			UnicodeStringRef text) :
		m_annotator(anAnnotator), m_unitexEngine(unitexEngine), m_view(m_annotator.getView()), m_fakeTokens(fakeTokenParameter), m_offset(offsetInDocument), m_text(text)
		{
			m_unitexEngine.getNormalizedText(m_sntText);

			// To each character of the input text, associates the index of its
			// corresponding position in the input text where all fake tokens (e.g.
			// [SDOC]) have been removed.
			// The characters inside the fake tokens are all associated to the last
			// character in a non fake token on the left.
			UnicodeString inputText;
			m_unitexEngine.getInputText(inputText);
			for (size_t i = 0; i < (size_t) inputText.length(); i++) {
				size_t delta = isFakeTokenAt(inputText, i);
				if (delta > 0) {
					for (size_t j = 0; j < delta; j++)
						m_inputWithoutFakeTokens.push_back(i);
				} else
					m_inputWithoutFakeTokens.push_back(i);
			}
		}

		UnitexTokenizer::~UnitexTokenizer()
		{
		}

		/**
		* Reads the list of tokens as determined by Unitex.
		*/
		void UnitexTokenizer::readTokenList()
		{
#ifdef _DEBUG
			cout << "Reading token forms..." << endl;
#endif

			path sntPath(m_unitexEngine.getSntDirectory());

			// Read the file contents
			path tokensPath = sntPath / "tokens.txt";
			UnicodeString ustrTokens;
			getStringFromFile(tokensPath.string(), ustrTokens);
#ifdef _DEBUG
			cout << "tokens.txt=" << endl << ustrTokens << endl << endl;
#endif
			// Split it into lines
			UnicodeStringlist lines;
			splitRegex(lines, ustrTokens, "\\n+");

			// Fill in the token vector
			m_tokenForms.clear();
			// Skip the first line which contains only the number of lines
			UnicodeStringlist::const_iterator it = lines.begin();
			for (++it; it != lines.end(); it++) {
				UnicodeString str = *it;
				m_tokenForms.push_back(str.trim());
			}
#ifdef _DEBUG
			cout << "End of reading token forms" << endl;
#endif
		}

		/**
		* Checks whether there is a fake token (cf. list of fake tokens passed during instantiation) starting at the given position in the string.
		* \return the length of the fake token if found, or 0 otherwise
		*/
		size_t UnitexTokenizer::isFakeTokenAt(UnicodeStringRef rustr, size_t position)
		{
			for (UnicodeStringVector::const_iterator it = m_fakeTokens.begin(); it != m_fakeTokens.end(); it++) {
				const UnicodeString& fakeToken = *it;
				if (!rustr.compare(position, position + fakeToken.length(), fakeToken))
					return fakeToken.length();
			}
			return 0;
		}

		/**
		*
		*/
		int32_t UnitexTokenizer::fakeTokenStartingAtLine(const UnicodeStringVector& lines, const int32_t n)
		{
			for (UnicodeStringVector::const_iterator it = m_fakeTokens.begin(); it != m_fakeTokens.end(); it++) {
				UnicodeString ustr;
				int32_t i = n;
				while ((i < lines.size()) && (ustr.length() < (*it).length())) {
					concatenateWithTokenFormAtLine(ustr, lines, i);
					i++;
				}
				if (ustr == *it) {
					m_firstLineToKeep = i;
					return (*it).length();
				}
			}
			m_firstLineToKeep = -1;
			return 0;
		}

		/**
		* Concatenates an indexed token form to a Unicode string.
		*
		* \param dest
		* 				the Unicode string the will expand
		* \param lines
		* 				a collection of token representations (as in tokens.txt file), one per line
		* \param n
		* 				the index of the token form in the lines collection
		*/
		void UnitexTokenizer::concatenateWithTokenFormAtLine(UnicodeString& dest, const UnicodeStringVector& lines, const int32_t n)
		{
			tuple<int32_t, int32_t, int32_t> coord = tokenCoordinates(lines[n]);
			dest += m_tokenForms[coord.get<0>()];
		}

		/**
		* Parses a line from the snttoken.txt file to extract the "token coordinates", i.e. the
		* 3 first integers at the beginning of the line.
		*/
		tuple<int32_t, int32_t, int32_t> UnitexTokenizer::tokenCoordinates(const UnicodeString& line)
		{
			ostringstream oss;
			oss << line;
			istringstream iss(oss.str());
			int32_t index, start, end;
			iss >> index >> start >> end;
			tuple<int32_t, int32_t, int32_t> result(index, start, end);
			return result;
		}

		/**
		* Reads the snttoken.txt file produced by Unitex and stores its contents.
		* The tokens' begin and end limits are modified in various situations:
		*/
		void UnitexTokenizer::readSntTokens()
		{
#ifdef DEBUG_UIMA_CPP
			cout << "Reading SNT tokens" << endl;
#endif

			path sntPath(m_unitexEngine.getSntDirectory());
			UnicodeStringRef rmb = m_annotator.getView().getDocumentText();
#ifdef DEBUG_UIMA_CPP
			cout << "RMB characters:" << endl;
			int32_t max = 100;
			if (max > rmb.length())
				max = rmb.length();
			for (int32_t i = m_offset; i < m_offset + max; i++) {
				UChar32 uc = rmb.char32At(i);
				UnicodeString us = uc;
				cout << "  rmb[" << i << "] = '" << us << "' (" << uc << ")" << endl;
			}
#endif

			// Read the file contents
			path tokensPath = sntPath / "snttokens.txt";
			UnicodeString ustrTokens;
			getStringFromFile(tokensPath.string(), ustrTokens);
#ifdef DEBUG_UIMA_CPP
			cout << "snttokens.txt=" << endl << ustrTokens << endl;
#endif

			// Split it into lines without blank or empty lines
			UnicodeStringVector lines;
			splitLines(lines, ustrTokens, false);

			// Fill in the token vector
			m_tokens.clear();
			m_sentenceMarkers.clear();

			size_t offsetWithoutFakeTokens = 0;
			SntToken firstTokenInSentence = SntToken::EmptyToken;
			SntToken lastNonDropToken = SntToken::EmptyToken;
			m_firstLineToKeep = -1;

			bool panic = false;
			int32_t panicEndOfLastToken = -1;

			// We keep an offset for (erroneous?) tokens coordinates given by Unitex
			// where the end is shorter than the form's length.
			int32_t offsetTooShortForms = 0;

			// Iterate over the lines of snttokens.txt
			int32_t nbLines = lines.size();
			for (int32_t tokenIndex = 0; tokenIndex < nbLines; tokenIndex++) {
#ifdef DEBUG_UIMA_CPP
				cout << "reading token " << tokenIndex << "/" << nbLines - 1 << ": \"" << lines[tokenIndex] << "\"" << endl;
#endif

				// Parse the current line to extract:
				//   - the token index in the list of token forms
				//   - the offset of the token beginning in the text
				//   - the offset of the token ending in the text
				tuple<int32_t, int32_t, int32_t> currentCoord = tokenCoordinates(lines[tokenIndex]);
				int32_t formIndex = currentCoord.get<0>();
				int32_t start = currentCoord.get<1>();
				int32_t end = currentCoord.get<2>();

				// Fix for too short forms found until now
#ifdef DEBUG_UIMA_CPP
				cout << "offsetTooShortForms=" << offsetTooShortForms << endl;
#endif
				if (offsetTooShortForms > 0) {
					start += offsetTooShortForms;
					end += offsetTooShortForms;
#ifdef DEBUG_UIMA_CPP
					cout << "fixed start=" << start << endl;
					cout << "fixed end=" << end << endl;
#endif
				}

				// If something has gone wrong in Unitex Tokenizer, we can detect it
				// here.
				if (!panic && tokenIndex > 0) {
					tuple<int32_t, int32_t, int32_t> previousCoord = tokenCoordinates(lines[tokenIndex - 1]);
					UnicodeString previousForm = m_tokenForms[previousCoord.get<0>()];
					if ((previousForm == UNICODE_STRING_SIMPLE("{S}")) && (start > previousCoord.get<1>())) {
						panic = true;
						if (!lastNonDropToken.isEmpty())
							panicEndOfLastToken = lastNonDropToken.end();
						m_annotator.logWarning("UnitexTokenizer is entering panic mode at pos %d!", panicEndOfLastToken);
					}
				}

				UnicodeString form = m_tokenForms[formIndex];
				bool drop = (tokenIndex < m_firstLineToKeep) || (form == "{S}");
				if (isBlank(form.trim())) {
					drop = true;
					// If the token detected by Unitex is a CRLF (i.e. it is a blank of 2 characters while the RMB is a LF)
					// then we increment the offset between Unitex's count and the RMB
					int32_t tokenLength = end - start;
					if (tokenLength % 2 == 0) {
						int32_t pos = start - offsetWithoutFakeTokens, nbLfInRmb = 0, rmbLen = rmb.length();
#ifdef DEBUG_UIMA_CPP
						cout << "Investigating possible CRLF / LF mismatch starting at " << pos << endl;
#endif
						while ((pos < rmbLen) && (rmb.char32At(pos) == 10)) {
							pos++;
							nbLfInRmb++;
						}
						if (nbLfInRmb == tokenLength / 2) {
							int32_t offsetIncrement = 0;
							offsetWithoutFakeTokens += nbLfInRmb;
#ifdef DEBUG_UIMA_CPP
							cout << "increment offset to RMB by " << nbLfInRmb << " because it has LF instead of CRLF" << endl;
#endif
						}
					}
				}

				if (!drop) {
					// The token's form is not blank.
					// Check if end is correct corresponding to the token form's length
					if (end < start + form.length()) {
#ifdef DEBUG_UIMA_CPP
						cout << "The end=" << end << " differs from expected " << start + form.length() << "!" << endl;
#endif
						offsetTooShortForms += start + form.length() - end;
						end += offsetTooShortForms;
#ifdef DEBUG_UIMA_CPP
						cout << "Fix end=" << end << endl;
						cout << "offsetTooShortForms=" << offsetTooShortForms << endl;
#endif
					}
				}

#ifdef DEBUG_UIMA_CPP
				if (drop) {
					cout << "drop token '" << form << "' because ";
					if (tokenIndex < m_firstLineToKeep)
						cout << "line " << tokenIndex << " is before first line to keep " << m_firstLineToKeep << endl;
					else if (form == UNICODE_STRING_SIMPLE("{S}"))
						cout << "is is a sentence marker" << endl;
					else
						cout << "the form is blank" << endl;
				}
#endif

				// In panic mode we will start making string matching to retrieve
				// the tokens.
				// It is slower but more resilient to tokenization errors (and
				// shouldn't occur too often anyway).
				if (panic) {
					if (!drop) {
						int pos = m_text.indexOf(form, panicEndOfLastToken);
						if (pos == -1) {
							if (m_annotator.isLoggingEnabled(LogStream::EnWarning)) {
								LogStream& ls = m_annotator.getLogStream(LogStream::EnWarning);
								ls << "Reach end of input text without finding token form \"" << form << "\" after pos " << panicEndOfLastToken << "!" << endl;
								ls.flush();
							}
						} else {
							start = pos + offsetWithoutFakeTokens;
							end = start + form.length();
							panicEndOfLastToken = end;
						}
					}
				}

				int deltaOffset = 0;
				if (!drop) 
					deltaOffset = fakeTokenStartingAtLine(lines, tokenIndex);

				if (deltaOffset > 0) {
					offsetWithoutFakeTokens += deltaOffset;
					drop = true;
#ifdef DEBUG_UIMA_CPP
					cout << "drop token '" << form << "' because it is a fake token of length " << deltaOffset << endl;
#endif
				}
#ifdef DEBUG_UIMA_CPP
				cout << "(" << start << ", " << end << ") =\t\"" << form << "\" offset =" << offsetWithoutFakeTokens << " firstLineToKeep=" << m_firstLineToKeep << " ";
#endif

				if (!drop) {
					start -= offsetWithoutFakeTokens;
					end -= offsetWithoutFakeTokens;
					if (panic)
						panicEndOfLastToken -= offsetWithoutFakeTokens;

					SntToken token(tokenIndex, formIndex, start, end, form);
					m_tokens.push_back(token);
#ifdef DEBUG_UIMA_CPP
					cout << "=> (" << start << ", " << end << ") " << (drop ? "drop" : "keep") << " ";
					UnicodeString substring;
					rmb.extract(start + m_offset, end - start, substring);
					cout << "in RMB=\"" << substring << "\"" << endl;
					if (form != substring)
						cout << "Divergence!" << endl;

					cout << "first token in sentence = " << firstTokenInSentence << endl;
#endif
					if (firstTokenInSentence.isEmpty()) {
						firstTokenInSentence = token;
#ifdef DEBUG_UIMA_CPP
						cout << "firstTokenInSentence = " << firstTokenInSentence << endl;
#endif
					}
					lastNonDropToken = token;
				} 
#ifdef DEBUG_UIMA_CPP
				else
					cout << "drop" << endl;
#endif

				if ((form == UNICODE_STRING_SIMPLE("{S}")) || (tokenIndex == lines.size() - 1)) {
					pair<SntToken, SntToken> sentenceMarker = make_pair(firstTokenInSentence, lastNonDropToken);
					m_sentenceMarkers.push_back(sentenceMarker);
					if (!firstTokenInSentence.isEmpty()) {
#ifdef DEBUG_UIMA_CPP
						cout << "added sentence marker (" << sentenceMarker.first << ", " << sentenceMarker.second << ")" << endl;
#endif
						firstTokenInSentence = SntToken::EmptyToken;
					}
				}
			}

#ifdef DEBUG_UIMA_CPP
			cout << "Leaving read SNT tokens" << endl;
#endif
		}

		/**
		* Gets the number of tokens created.
		*/
		size_t UnitexTokenizer::countTokens() const
		{
			return m_tokens.size();
		}

		/**
		* Tokenizes a string (and creates the underlying token annotations in the UIMA view).
		*
		* \param tokenIndexOffset
		* 				the number of tokens already created before processing this string
		*/
		size_t UnitexTokenizer::tokenize(size_t tokenIndexOffset)
		{
#ifdef DEBUG_UIMA_CPP
			ostringstream oss;
			m_annotator.logMessage("Creating token annotations... starting with tokenIndexOffset = %d", tokenIndexOffset);
#endif
			m_tokens.clear();

			readTokenList();
			readSntTokens();

#ifdef DEBUG_UIMA_CPP
			cout << "There are " << m_tokens.size() << " tokens in SNT" << endl;
#endif

			vector<TokenAnnotation> rmbTokens;
			m_tokensByIndex.clear();

			for (size_t i = 0; i < m_tokens.size(); i++) {
				SntToken& sntToken = m_tokens[i];
				const UnicodeString& tokenForm = sntToken.form();
#ifdef DEBUG_UIMA_CPP
				cout << "Processing token[" << i << "]=" << sntToken << endl;
#endif

				int32_t tokenBegin = sntToken.start() + m_offset;
				int32_t tokenEnd = sntToken.end() + m_offset;

				TokenAnnotation rmbToken(m_view, tokenBegin, tokenEnd, tokenForm, sntToken.tokenIndex() + tokenIndexOffset);
				rmbTokens.push_back(rmbToken);
#ifdef DEBUG_UIMA_CPP
				cout << "create RMB token " << rmbToken << endl;
#endif

				m_tokensByIndex[rmbToken.getIndex()] = rmbToken;

				if (rmbTokens.size() > 1) {
					TokenAnnotation previousRmbToken = rmbTokens[rmbTokens.size() - 2];
					rmbToken.setPrevious(previousRmbToken);
					previousRmbToken.setNext(rmbToken);
				}
			}

			// Mark paragraphs and sentences
			if (m_tokens.size() > 0) {
				createParagraphAnnotations();
				createSentenceAnnotations();
			}

			return countTokens();
		}

		void UnitexTokenizer::createParagraphAnnotations()
		{
#ifdef _DEBUG
			cout << "Creating paragraph annotations" << endl;
#endif

			CAS& view = m_annotator.getView();

			// Split the RMB into non empty paragraphs
			vector<TextArea> paragraphAreas;
			TextArea::getParagraphAreas(m_text, paragraphAreas);

			vector<ParagraphAnnotation> rmbParagraphs;
			for (vector<TextArea>::const_iterator it = paragraphAreas.begin(); it != paragraphAreas.end(); it++) {
				const TextArea& paragraph = *it;
#ifdef _DEBUG
				cout << "Processing paragraph " << paragraph << endl;
#endif

				// Don't process empty paragraphs
				UnicodeString paragraphText;
				paragraph.getText(paragraphText);
				if (isBlank(paragraphText))
					continue;
#ifdef _DEBUG
				cout << "Paragraph is not blank" << endl;
#endif

				int32_t start = paragraph.getBegin() + m_offset;
				int32_t end = paragraph.getEnd() + m_offset;
#ifdef _DEBUG
				cout << "start=" << start << ", end=" << end << endl;
#endif

				TokenAnnotation firstToken = TokenAnnotation::tokenStartingAtOffsetInView(start, view);
				if (!firstToken.isValid()) {
					ostringstream oss;
					oss << "Cannot create paragraph because cannot find token starting at " << paragraph.getBegin() << " in RMB";
					throw UnitexException(oss.str());
				}
#ifdef _DEBUG
				cout << "First token = " << firstToken << endl;
#endif

				if (firstToken.getBegin() > start) {
					bool skip = true;
					// Special case of BOM
					if (start == 0) {
						UChar32 c = paragraphText.char32At(0);
						if (firstToken.getBegin() == 1 && ((c == 0xFEFF) || (c == 0xFFFE)))
							skip = false;
					}
					if (skip) {
						m_annotator.getLogger().logMessage("First token found after current paragraph boundaries => skipping");
						continue;
					}
				}

				TokenAnnotation lastToken = TokenAnnotation::tokenEndingAtOffsetInView(end, view);
				if (!lastToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find token ending at " << end << " in RMB";
					throw UnitexException(oss.str());
				}
#ifdef _DEBUG
				cout << "Last token = " << lastToken << endl;
#endif

				ParagraphAnnotation rmbPar(view, firstToken.getBegin(), lastToken.getEnd(), firstToken, lastToken);
				rmbParagraphs.push_back(rmbPar);
#ifdef _DEBUG
				cout << "Create paragraph " << rmbPar << endl;
#endif
			}
		}

		size_t UnitexTokenizer::convertOffset(size_t offset, bool askEnd) const
		{
			if (askEnd)
				return m_tokens[offset].end();
			return m_tokens[offset].start();
		}

		TokenAnnotation* UnitexTokenizer::getTokenByIndex(int32_t index)
		{
			map<int32_t, TokenAnnotation>::iterator it = m_tokensByIndex.find(index);
			if (it == m_tokensByIndex.end())
				return NULL;
			return &(it->second);
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
				UnicodeString line = *it;
				UnicodeString pattern = UNICODE_STRING_SIMPLE("^(").append(quoteSign).append("\\s*)*");
				UErrorCode error = U_ZERO_ERROR;
				RegexMatcher matcher(pattern, line, 0, error);
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
			static UnicodeString unitexMarks[] =
			{ "{S}", "[SUBJECT]", "[SDOC]", "[EDOC]" };
			static const size_t nbMarks = 4;

			const int32_t length = MIN((int32_t)text.length(), limit + 1);
			UnicodeString substring(text, offset);
			for (size_t i = 0; i < nbMarks; i++) {
				if ((offset + unitexMarks[i].length() <= length) && (substring.startsWith(unitexMarks[i]))) {
					return unitexMarks[i].length();
				}
			}
			return 0;
		}

		/**
		* Tests whether we are on the end of a Unitex mark (sentence mark of subject line
		* mark) and returns the length of the mark if so, or 0 it we are not on a
		* mark.
		*
		* \param text the reference text
		* \param offset the current offset
		* \param limit a limit to the left offset
		* \return the number of characters to skip when finding a Unitex specific mark
		*/
		static int32_t isEndOfUnitexMark(const UnicodeString& text, int32_t offset, int32_t limit)
		{
			static UnicodeString unitexMarks[] =
			{ "{S}", "[SUBJECT]", "[SDOC]", "[EDOC]" };
			static const size_t nbMarks = 4;

			for (size_t i = 0; i < nbMarks; i++) {
				if (offset - unitexMarks[i].length() - 1 >= limit) {
					UnicodeString substring(text, offset - unitexMarks[i].length() - 1);
					if (substring.startsWith(unitexMarks[i]))
						return unitexMarks[i].length();
				}
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
			int32_t offset = currentOffset;
			const int32_t length = MIN((int32_t)text.length(), limit + 1);
			while ((offset < length) && (text.charAt(offset) != '\r') && (text.charAt(offset) != '\n')) {
				int32_t unitexMark = isUnitexMark(text, offset, limit);
				if (unitexMark > 0)
					return offset + unitexMark;
				else
					offset++;
			}
			while ((offset < length) && ((text.charAt(offset) == '\r') || (text.charAt(offset) == '\n')))
				offset++;
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

		static int32_t skipToLastNonEmptyCharacterBefore(const UnicodeString& text, int32_t currentOffset, int32_t limit)
		{
			static UnicodeString blank("\r\n \t");

			assert(limit <= currentOffset);

			int32_t offset = currentOffset, unitexMark = 0;
			if (offset >= text.length())
				offset = text.length() - 1;
			if (isUnitexMark(text, offset, text.length()) > 0)
				offset--;
			do {
				while ((offset >= limit) && (blank.indexOf(text.charAt(offset)) >= 0)) {
					UnicodeString rest(text, offset);
					offset--;
				}
				unitexMark = isEndOfUnitexMark(text, offset, limit);
				if (unitexMark > 0) {
					offset -= unitexMark;
				}
			} while (unitexMark);
			UnicodeString rest(text, offset);
			return offset;
		}

		void UnitexTokenizer::createSentenceAnnotations()
		{
#ifdef _DEBUG
			cout << "Creating sentence annotations..." << endl;
#endif

			for (vector<pair<SntToken, SntToken> >::const_iterator it = m_sentenceMarkers.begin(); it != m_sentenceMarkers.end(); it++) {
				const pair<SntToken, SntToken>& limits = *it;
#ifdef _DEBUG
				cout << "sentence marker [" << limits.first << ", " << limits.second << "]" << endl;
#endif
				if (limits.first.isEmpty() || limits.second.isEmpty())
					continue;

				int32_t start = limits.first.start() + m_offset;
				int32_t end = limits.second.end() + m_offset;

				TokenAnnotation firstToken = TokenAnnotation::tokenStartingAtOffsetInView(start, m_view);
				if (!firstToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find RMB token starting at offset " << start;
					throw UnitexException(oss.str());
				}
#ifdef _DEBUG
				cout << "first token = " << firstToken << endl;
#endif

				TokenAnnotation lastToken = TokenAnnotation::tokenEndingAtOffsetInView(end, m_view);
				if (!lastToken.isValid()) {
					ostringstream oss;
					oss << "Cannot find RMB token ending at offset " << end;
					throw UnitexException(oss.str());
				}
#ifdef _DEBUG
				cout << "last token = " << lastToken << endl;
#endif

				ParagraphAnnotation paragraph = ParagraphAnnotation::getParagraphCovering(firstToken, m_view);
				if (!paragraph.isValid()) {
					ostringstream oss;
					oss << "Cannot retrieve paragraph covering token " << firstToken << endl;
					throw UnitexException(oss.str());
				}
#ifdef _DEBUG
				cout << "paragraph = " << paragraph << endl;
#endif
				SentenceAnnotation rmbSentence(m_view, firstToken.getBegin(), lastToken.getEnd(), firstToken, lastToken, paragraph);
#ifdef _DEBUG
				cout << "Create sentence " << rmbSentence << endl;
#endif
			}
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

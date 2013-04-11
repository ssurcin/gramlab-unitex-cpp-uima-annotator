/*
 * TextArea.cpp
 *
 *  Created on: 14 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "TextArea.h"
#include "UnitexException.h"
#include <unicode/regex.h>
#include <unicode/ustream.h>
#include "Utils.h"

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
using namespace icu;
using namespace std;

namespace unitexcpp
{

	namespace tokenize
	{

		TextArea::TextArea()
		{
			m_begin = m_end = 0;
		}

		TextArea::TextArea(const UnicodeStringRef aStringRef, int32_t nStart, int32_t nEnd)
		{
			m_stringRef = aStringRef;
			m_begin = nStart;
			m_end = nEnd;
		}

		TextArea::TextArea(const UnicodeStringRef aStringRef)
		{
			m_stringRef = aStringRef;
			m_begin = 0;
			m_end = aStringRef.length();
		}

		TextArea::TextArea(const TextArea& model, int32_t nStart, int32_t nEnd)
		{
			*this = model;
			m_begin = nStart;
			m_end = nEnd;
		}

		TextArea::~TextArea()
		{
		}

		TextArea& TextArea::operator =(const TextArea& model)
		{
			m_stringRef = model.m_stringRef;
			m_begin = model.m_begin;
			m_end = model.m_end;
			return *this;
		}

		void TextArea::getParagraphAreas(const UnicodeStringRef string, vector<TextArea>& paragraphs)
		{
			static RegexPattern* pParagraphRE = NULL;
			static RegexPattern* pInitialNewLineRE = NULL;
			if (!pParagraphRE) {
					UErrorCode error = U_ZERO_ERROR;
					pParagraphRE = RegexPattern::compile("(\\s*\\n){2,}|\\n\\s*\\*\\s*\\n", UREGEX_MULTILINE, error);
					if (U_FAILURE(error)) {
						ostringstream oss;
						oss << "Error building paragraph regex: " << u_errorName(error);
						throw UnitexException(oss.str());
					}
					error = U_ZERO_ERROR;
					pInitialNewLineRE = RegexPattern::compile("^\\s*\\n", 0, error);
					if (U_FAILURE(error)) {
						ostringstream oss;
						oss << "Error building paragraph regex: " << u_errorName(error);
						throw UnitexException(oss.str());
					}
			}

			UnicodeString ustring = toString(string);
			UErrorCode status = U_ZERO_ERROR;
			RegexMatcher* pMatcher = pInitialNewLineRE->matcher(ustring, status);
			if (U_FAILURE(status)) {
				ostringstream oss;
				oss << "Error applying regex for initial new line: " << u_errorName(status);
				throw UnitexException(oss.str());
			}
			bool hasInitialNewLine = pMatcher->find();

			paragraphs.clear();
			TextArea area(string);
			UnicodeString text;
			area.getText(text);
			vector<TextArea> counterparts = area.counterpart(pParagraphRE);
			for (size_t i = 0; i < counterparts.size(); i++) {
				// If the first paragraph starts with a single newline, it will not be catched
				// by the regular expression, we must fix it manually
				if ((i == 0) && hasInitialNewLine) {
#ifdef _DEBUG
					cout << "Text starts with initial new line, fix first paragraph" << endl;
#endif
					TextArea first(counterparts[i], counterparts[i].getBegin() + 1, counterparts[i].getEnd());
					paragraphs.push_back(first);
				}
				else
					paragraphs.push_back(counterparts[i]);
			}
		}

		/*!
		 * Read-only accessor to the Unicode string reference.
		 */
		UnicodeStringRef TextArea::getReference() const
		{
			return m_stringRef;
		}

		/*!
		 * Read-only accessor to the begin offset.
		 */
		int32_t TextArea::getBegin() const
		{
			return m_begin;
		}

		/*!
		 * Read-only accessor to the end offset.
		 */
		int32_t TextArea::getEnd() const
		{
			return m_end;
		}

		/*!
		 * Length of the area.
		 */
		int32_t TextArea::length() const
		{
			return m_end - m_begin;
		}

		/*!
		 * Read-only accessor to the text covered by the area.
		 */
		void TextArea::getText(UnicodeString& result) const
		{
			m_stringRef.extractBetween(getBegin(), getEnd(), result);
		}

		/*!
		 * Output a readable representation to an output stream.
		 */
		ostream& operator <<(ostream& os, const TextArea& textArea)
		{
			UnicodeString text;
			textArea.getText(text);
			os << "(" << textArea.getBegin() << "," << textArea.getEnd() << ")=[" << text << "]";
			return os;
		}

		/*!
		 * Equality operator.
		 */
		bool TextArea::operator ==(const TextArea& other) const
		{
			if (m_stringRef == other.m_stringRef) {
				return ((m_begin == other.m_begin) && (m_end == other.m_end));
			}
			return false;
		}

		/*!
		 * Total order less operator.
		 */
		bool TextArea::operator <(const TextArea& other) const
		{
			if (m_stringRef == other.m_stringRef) {
				if (m_begin < other.m_begin)
					return true;
				else if (m_begin == other.m_begin)
					return (m_end < other.m_end);
			}
			return false;
		}

		/*!
		 * Get all the sub-areas in the current area, that match the given pattern.
		 */
		vector<TextArea> TextArea::select(RegexPattern* pPattern) const
		{
			static UErrorCode error;

			vector<TextArea> result;

			UnicodeString text = toString(m_stringRef);
			RegexMatcher* pMatch = pPattern->matcher(text, error);
			if (U_FAILURE(error))
				throw UnitexException(u_errorName(error));

			bool matches = pMatch->find(0, error);
			if (U_FAILURE(error))
				throw UnitexException(u_errorName(error));

			while (matches) {
				int32_t start = pMatch->start(error);
				if (U_FAILURE(error))
					throw UnitexException(u_errorName(error));
				if ((start < getBegin()) || (start >= getEnd()))
					break;
				int32_t end = pMatch->end(error);
				if (U_FAILURE(error))
					throw UnitexException(u_errorName(error));
				result.push_back(TextArea(getReference(), start, end));
				matches = pMatch->find();
			}
			delete pMatch;

			return result;
		}

		/*!
		 * Get all the sub-areas in the current area, that DO NOT match the given pattern.
		 */
		vector<TextArea> TextArea::counterpart(RegexPattern* pPattern) const
		{
			return counterpart(select(pPattern));
		}

		/*!
		 * Return the counterpart of a collection of areas in a given text area. The
		 * areas are in fact sub-areas of the text area, and we build the collection
		 * of the other sub-areas.
		 */
		vector<TextArea> TextArea::counterpart(const vector<TextArea>& selectedAreas) const
		{
			vector<TextArea> result;

			vector<TextArea> areas = selectedAreas;
			sort(areas.begin(), areas.end());

			int32_t start = getBegin(), end = getEnd();
			for (size_t i = 0; i < areas.size(); i++) {
				if ((areas[i].getBegin() > start) && (areas[i].getBegin() < end))
					result.push_back(TextArea(getReference(), start, areas[i].getBegin()));
				start = areas[i].getEnd();
			}
			if (start < end)
				result.push_back(TextArea(getReference(), start, end));

			return result;
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/*
 * UnitexOutputOffsetConverter.cpp
 *
 *  Created on: 18 janv. 2011
 *      Author: sylvainsurcin
 */

#include "UnitexOutputOffsetConverter.h"
#include "UnitexException.h"
#include "unicode/regex.h"
#include "unicode/uchar.h"

using namespace std;
using namespace uima;
using namespace icu;

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace unitexcpp
{

	/*!
	 * Helper function checks whether we match the Subject line mark.
	 * \param ustring the reference string
	 * \param pos the current position in the string
	 * \param length the rightmost limit where to look
	 * \return true if the current postion holds the subject mark
	 */
	bool matchesSubjectMark(const UnicodeString& ustring, size_t pos, size_t length)
	{
		if (pos + 8 < length) {
			UnicodeString sub(ustring, pos, 9);
			return (sub == UNICODE_STRING_SIMPLE("[SUBJECT]"));
		}
		return false;
	}

	/*!
	 * Helper function skips the subject mark in a string
	 * \param ustring the reference string
	 * \param pos the current position in the string (will be modified after the mark)
	 * \param length the rightmost limit where to look
	 */
	void skipSubjectMark(const UnicodeString& ustring, size_t& pos, size_t length)
	{
		while (matchesSubjectMark(ustring, pos, length))
			pos += 9;
	}

	/*!
	 * Helper function checks whether we match the Sentence line mark.
	 * \param ustring the reference string
	 * \param pos the current position in the string
	 * \param length the rightmost limit where to look
	 * \return true if the current postion holds the sentence mark
	 */
	bool matchesSentenceMark(const UnicodeString& ustring, size_t pos, size_t length)
	{
		if (pos + 2 < length) {
			UnicodeString sub(ustring, pos, 3);
			return (sub == UNICODE_STRING_SIMPLE("{S}"));
		}
		return false;
	}

	/*!
	 * Helper function skips the sentence mark in a string
	 * \param ustring the reference string
	 * \param pos the current position in the string (will be modified after the mark)
	 * \param length the rightmost limit where to look
	 */
	void skipSentenceMark(const UnicodeString& ustring, size_t& pos, size_t length)
	{
		while (matchesSentenceMark(ustring, pos, length))
			pos += 3;
	}

	/*!
	 * Builds the converter for a normalized version of the RMB.
	 * \param rustrNormalizedText the normalized RMB with sentence (and other) marks
	 * \param aCas the SOFA containing the actual RMB
	 */
	UnitexOutputOffsetConverter::UnitexOutputOffsetConverter(UnicodeStringRef rustrNormalizedText, CAS& aCas) :
		rmbView(aCas)
	{

		UnicodeString rmb;
		rmbView.getDocumentText().extract(0, rmbView.getDocumentText().length(), rmb);

		UnicodeString normalizedText;
		rustrNormalizedText.extract(0, rustrNormalizedText.length(), normalizedText);

		UErrorCode error = U_ZERO_ERROR;
		RegexMatcher sentenceMatcher("\\{S\\}", normalizedText, 0, error);
		if (U_FAILURE(error)) {
			ostringstream oss;
			oss << "UnitexOutputOffsetConverter ctor error building sentenceMatcher " << u_errorName(error);
			throw UnitexException(oss.str());
		}
		error = U_ZERO_ERROR;
		UnicodeString textWithoutMarks = sentenceMatcher.replaceAll("", error);
		if (U_FAILURE(error)) {
			ostringstream oss;
			oss << "UnitexOutputOffsetConverter ctor error replacing sentenceMatcher " << u_errorName(error);
			throw UnitexException(oss.str());
		}

		count = textWithoutMarks.length();
		offsets = new int[count];

		size_t i = 0, j = 0;
		size_t noMarksLength = textWithoutMarks.length();
		size_t rmbLength = rmb.length();

		/*
		for (size_t k = 0; k < std::max(noMarksLength, rmbLength); k++) {
			if (k < noMarksLength)
				cout << "norm[" << k << "]='" << UnicodeString(textWithoutMarks.charAt(k)) << "'";
			else
				cout << "norm[" << k << "]     ";
			if (k < rmbLength)
				cout << "rmb[" << k << "]='" << UnicodeString(rmb.charAt(k)) << "'" << endl;
			else
				cout << "rmb[" << k << "]" << endl;
		}
		*/

		while (i < noMarksLength) {
			skipSubjectMark(textWithoutMarks, i, noMarksLength);
			if (i >= noMarksLength)
				break;

			// skip sentence marks present in RMB
			skipSentenceMark(rmb, j, rmbLength);

			// It is possible that the original RMB contains repetitions of blank lines. They are
			// not repeated in the normalized text output by Unitex. We must take them into account
			// if we want the indexes to be right.
			// To do that, we will increment the index in original text when we find a blank character
			// and no blank in the corresponding normalized text character.
			bool isWhite1 = u_isWhitespace(textWithoutMarks.char32At(i));
			bool isWhite2 = u_isWhitespace(rmb.char32At(j));
			while (isWhite1 != isWhite2) {
				if (isWhite1) {
					i++;
					if (i >= noMarksLength)
						break;
				}
				else {
					j++;
					if (j >= rmbLength)
						break;
				}
				isWhite1 = u_isWhitespace(textWithoutMarks.char32At(i));
				isWhite2 = u_isWhitespace(rmb.char32At(j));
			}
			if (i < count) {
				offsets[i++] = j++;
				//System.out.printf("t[%d]='%c' -> rmb[%d]='%c'\n", i-1, textWithoutMarks.charAt(i-1), j-1, rmb.charAt(j-1));
				//if (textWithoutMarks.charAt(i-1) != rmb.charAt(j-1))
				//	System.out.println(">>>> CA CHANGE <<<<");
			}
		}
	}

	UnitexOutputOffsetConverter::~UnitexOutputOffsetConverter()
	{
		delete[] offsets;
	}

	/*!
	 * Converts the offset as provided by Unitex "UIMA" output into an offset in
	 * the RMB view.
	 * \param offset the "UIMA" offset provided by Unitex
	 * \return the corresponding offset in the RMB view
	 */
	size_t UnitexOutputOffsetConverter::convert(size_t offset) const
	{
		if (offset > count)
			offset = count - 1;

		if (offset == count)
			return rmbView.getDocumentText().length();
		else
			return offsets[offset];
	}
}

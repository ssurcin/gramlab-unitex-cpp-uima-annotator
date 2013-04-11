/*
 * QualifiedString.cpp
 *
 *  Created on: 18 janv. 2011
 *      Author: sylvainsurcin
 */

#include "QualifiedString.h"
#include <unicode/ustream.h>

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace boost;
using namespace icu;
using namespace std;

namespace unitexcpp
{

	namespace engine
	{

		/**
		 * Builds a qualified string specifying all available information.
		 *
		 * @param start
		 *            the start offset of the matched text
		 * @param end
		 *            the end offset of the matched text
		 * @param aString
		 *            the transducer's output string
		 */
		QualifiedString::QualifiedString(int32_t start, int32_t end, const UnicodeString& aString) :
			nStart(start), nEnd(end), strString(aString)
		{
		}

		QualifiedString::QualifiedString(const QualifiedString& model)
		{
			*this = model;
		}

		QualifiedString::~QualifiedString()
		{
		}

		int32_t QualifiedString::getStart() const
		{
			return nStart;
		}

		int32_t QualifiedString::getEnd() const
		{
			return nEnd;
		}

		UnicodeString& QualifiedString::getString()
		{
			return strString;
		}

		void QualifiedString::setString(const UnicodeString& aString)
		{
			strString = aString;
		}

		QualifiedString& QualifiedString::operator =(const QualifiedString& model)
		{
			nStart = model.nStart;
			nEnd = model.nEnd;
			strString = model.strString;
			return *this;
		}

		bool QualifiedString::operator ==(const QualifiedString& other) const
		{
			return ((nStart == other.nStart) && (nEnd == other.nEnd) && (strString.compare(other.strString) == 0));
		}

		bool QualifiedString::operator <(const QualifiedString& other) const
		{
			if (nStart < other.nStart)
				return true;
			else if (nStart == other.nStart) {
				if (nEnd < other.nEnd)
					return true;
				else if (nEnd == other.nEnd)
					return (strString.compare(other.strString) < 0);
			}
			return false;
		}

		ostream& operator <<(ostream& os, const QualifiedString& qstring)
		{
			os << "(" << qstring.nStart << "," << qstring.nEnd << ":'" << qstring.strString << "')";
			return os;
		}
	}

}

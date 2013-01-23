/*
 * QualifiedString.h
 *
 *  Created on: 18 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef QUALIFIEDSTRING_H_
#define QUALIFIEDSTRING_H_

#include <iostream>
#include <boost/tuple/tuple.hpp>
#include <unicode/unistr.h>

namespace unitexcpp
{

	namespace engine
	{

		/**
		 * This class binds together the input and output of a transducer's match.
		 * It contains the start and end offset of the matched text, and the
		 * corresponding transducer's output.
		 *
		 * @author surcin@kwaga.com
		 *
		 */
		class QualifiedString
		{
		public:
			QualifiedString(int32_t start, int32_t end, const icu::UnicodeString& aString);
			QualifiedString(const QualifiedString& model);
			virtual ~QualifiedString();

			int32_t getStart() const;
			int32_t getEnd() const;
			icu::UnicodeString& getString();
			const icu::UnicodeString& getString() const { return strString; }
			void setString(const icu::UnicodeString& aString);

			QualifiedString& operator =(const QualifiedString& other);

			bool operator ==(const QualifiedString& other) const;
			bool operator <(const QualifiedString& other) const;

			friend std::ostream& operator <<(std::ostream& os, const QualifiedString& qstring);

		private:
			int32_t nStart;
			int32_t nEnd;
			icu::UnicodeString strString;
		};

	}

}

#endif /* QUALIFIEDSTRING_H_ */

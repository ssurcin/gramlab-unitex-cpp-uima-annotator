/*
 * TextArea.h
 *
 *  Created on: 14 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TEXTAREA_H_
#define TEXTAREA_H_

#include <uima/api.hpp>
#include <unicode/regex.h>
#include <vector>

namespace unitexcpp
{

	namespace tokenize
	{

		/**
		 * This utility class represents an area of a reference text, this way we keep
		 * track of its offset and length in the reference text.
		 *
		 * @author surcin@kwaga.com
		 *
		 */
		class TextArea
		{
		public:
			TextArea();
			TextArea(const uima::UnicodeStringRef aStringRef, int32_t nStart, int32_t nEnd);
			TextArea(const uima::UnicodeStringRef aStringRef);
			TextArea(const TextArea& model, int32_t nStart, int32_t nEnd);
			virtual ~TextArea();

			TextArea& operator =(const TextArea& model);

			static void getParagraphAreas(const uima::UnicodeStringRef string, std::vector<TextArea>& paragraphs);

			uima::UnicodeStringRef getReference() const;
			int32_t getBegin() const;
			int32_t getEnd() const;
			int32_t length() const;
			void getText(icu::UnicodeString& result) const;

			bool operator <(const TextArea& other) const;
			bool operator ==(const TextArea& other) const;

			std::vector<TextArea> select(icu::RegexPattern* pPattern) const;

			std::vector<TextArea> counterpart(icu::RegexPattern* pPattern) const;
			std::vector<TextArea> counterpart(const std::vector<TextArea>& selectedAreas) const;

		private:
			uima::UnicodeStringRef m_stringRef;
			int32_t m_begin;
			int32_t m_end;

			friend std::ostream& operator <<(std::ostream& os, const TextArea& textArea);
		};

	}

}

#endif /* TEXTAREA_H_ */

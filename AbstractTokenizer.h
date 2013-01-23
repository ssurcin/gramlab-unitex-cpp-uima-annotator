/*
 * AbstractTokenizer.h
 *
 *  Created on: 12 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef ABSTRACTTOKENIZER_H_
#define ABSTRACTTOKENIZER_H_

#include <uima/api.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>

namespace unitexcpp
{

	namespace tokenize
	{

		class SntToken: public boost::tuple<int32_t, int32_t, uima::UnicodeStringRef>
		{
		public:
			SntToken(int32_t startOffset, int32_t endOffset, uima::UnicodeStringRef content);
			virtual ~SntToken();

			int32_t start() const;
			int32_t end() const;
			uima::UnicodeStringRef content() const;

			int32_t length() const;
		};

		class AbstractTokenizer
		{
		public:
			AbstractTokenizer();
			virtual ~AbstractTokenizer();

			void tokenize(std::size_t anOffset, const uima::UnicodeStringRef refUnicodeString);
			const std::vector<SntToken>& getTokens() const;

			virtual std::size_t convertOffset(std::size_t offset, bool askEnd =false) const =0;

		protected:
			virtual void extractTokens() =0;

		protected:
			uima::UnicodeStringRef rustrText;
			std::size_t offset; // The offset of the original text in the document
			std::vector<SntToken> vecTokens;
		};

	}

}

#endif /* ABSTRACTTOKENIZER_H_ */

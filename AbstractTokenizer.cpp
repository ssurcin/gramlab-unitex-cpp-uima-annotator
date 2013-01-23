/*
 * AbstractTokenizer.cpp
 *
 *  Created on: 12 janv. 2011
 *      Author: sylvainsurcin
 */

#include "AbstractTokenizer.h"

using namespace std;
using namespace boost;
using namespace uima;
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

		SntToken::SntToken(int32_t startOffset, int32_t endOffset, UnicodeStringRef content) :
			tuple<int32_t, int32_t, UnicodeStringRef> (startOffset, endOffset, content)
		{
		}

		SntToken::~SntToken()
		{
		}

		int32_t SntToken::start() const
		{
			return this->get<0>();
		}

		int32_t SntToken::end() const
		{
			return this->get<1>();
		}

		UnicodeStringRef SntToken::content() const
		{
			return this->get<2>();
		}

		int32_t SntToken::length() const
		{
			return end() - start();
		}

		///////////////////////////////////////////////////////////////////////
		//
		// class AbstractTokenizer
		//
		///////////////////////////////////////////////////////////////////////

		AbstractTokenizer::AbstractTokenizer()
		{
		}

		AbstractTokenizer::~AbstractTokenizer()
		{
		}

		void AbstractTokenizer::tokenize(size_t anOffset, UnicodeStringRef refUnicodeString)
		{
			rustrText = refUnicodeString;
			offset = anOffset;
			vecTokens.clear();
			extractTokens();
		}

		const vector<SntToken>& AbstractTokenizer::getTokens() const
		{
			return vecTokens;
		}

	}

}

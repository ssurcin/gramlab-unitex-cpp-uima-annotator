/*
 * TokenizeCommand.cpp
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "TokenizeCommand.h"
#include "UnitexEngine.h"

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace unitexcpp;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace command
	{

		TokenizeCommand::TokenizeCommand(UnitexEngine& unitexEngine, const string& inputName, const string& alphabet, bool bCharByChar, const string& inputOffsetsName, const string& outputOffsetsName) :
			UnitexCommand(unitexEngine, "Tokenize"), m_inputFile(inputName), m_alphabetFile(alphabet), m_charByChar(bCharByChar), m_inputOffsets(inputOffsetsName), m_outputOffsets(outputOffsetsName)
		{
		}

		TokenizeCommand::~TokenizeCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		void TokenizeCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Tokenize");
			arguments.push_back("-a");
			arguments.push_back(absolutePathnameOf(m_alphabetFile));
			if (m_charByChar)
				arguments.push_back("-c");
			if (!m_inputOffsets.empty()) {
				arguments.push_back("--input_offsets");
				arguments.push_back(m_inputOffsets);
			}
			if (!m_outputOffsets.empty()) {
				arguments.push_back("--output_offsets");
				arguments.push_back(m_outputOffsets);
			}
			arguments.push_back(absolutePathnameOf(m_inputFile));
		}
	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

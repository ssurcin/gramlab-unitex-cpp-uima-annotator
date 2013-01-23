/*
 * Fst2CheckCommand.cpp
 *
 *  Created on: 7 août 2012
 *      Author: sylvain
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "Fst2CheckCommand.h"
#include "UnitexEngine.h"
#include "Unitex-C++/Fst2Check.h"

using namespace unitexcpp::engine;
using namespace std;
using namespace boost::filesystem;

namespace unitexcpp
{
	namespace command
	{

		Fst2CheckCommand::Fst2CheckCommand(
				UnitexEngine& unitexEngine,
				const path& inputPath,
				bool sentenceCheck,
				bool loopCheck,
				bool emptyGraphWarning,
				bool appendMode,
				bool statistics,
				const string inputEncoding,
				const path outputPath,
				const string outputEncoding) :
				UnitexCommand(unitexEngine, "Fst2Check")
		{
			m_inputPath = inputPath;
			m_inputEncoding = inputEncoding;
			m_outputPath = outputPath;
			m_outputEncoding = outputEncoding;
			m_loopCheck = loopCheck;
			m_sentenceCheck = sentenceCheck;
			m_emptyGraphWarnings = emptyGraphWarning;
			m_appendOutputMode = appendMode;
			m_showStatistics = statistics;
		}

		Fst2CheckCommand::~Fst2CheckCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand Fst2CheckCommand::getUnitexCommandFunction() const
		{
			return &unitex::main_Fst2Check;
		}

		void Fst2CheckCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getCommandName());

			arguments.push_back(m_inputPath.string());
			arguments.push_back("--input_encoding");
			arguments.push_back(m_inputEncoding);
			if (!m_outputPath.empty()) {
				arguments.push_back("-o");
				arguments.push_back(m_outputPath.string());
				arguments.push_back("--output_encoding");
				arguments.push_back(m_outputEncoding);
			}
			if (m_showStatistics)
				arguments.push_back("-s");
			arguments.push_back(m_loopCheck ? "-y" : "-n");
			if (m_appendOutputMode)
				arguments.push_back("--append");
			if (m_sentenceCheck)
				arguments.push_back("-t");
			if (!m_emptyGraphWarnings)
				arguments.push_back("-e");
		}

	} /* namespace command */
} /* namespace unitexcpp */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

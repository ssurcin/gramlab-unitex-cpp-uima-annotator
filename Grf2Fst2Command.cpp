/*
 * Grf2Fst2Command.cpp
 *
 *  Created on: 6 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "Grf2Fst2Command.h"
#include "UnitexEngine.h"
#include "Unitex-C++/Grf2Fst2.h"
#include <boost/filesystem.hpp>
#include "FileUtils.h"

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <iostream>

using namespace std;
using namespace unitexcpp::engine;
using namespace boost::filesystem;
using namespace unitex;

namespace unitexcpp
{

	namespace command
	{

		Grf2Fst2Command::Grf2Fst2Command(UnitexEngine& unitexEngine, const path& graph, const path& target, const path& alphabet, const path& repositoryFolder, bool flagCheckLoop, bool flagCharByChar,
				bool flagNoEmptyGraphWarning, bool flagCheckValidSentence, bool checkVariableOutput, bool silentMode) :
				UnitexCommand(unitexEngine, "Grf2Fst2")
		{
			m_graphSourcePath = absolutePathnameOf(graph);

			if (!target.empty())
				m_graphTargetPath = target;
			else
				m_graphTargetPath = change_extension(graph, ".fst2");

			if (!alphabet.empty())
				m_alphabetPath = absolutePathnameOf(alphabet);
			else
				m_alphabetPath = virtualizedPath(getUnitexEngine().getAlphabetFile());

			if (!repositoryFolder.empty())
				m_graphRepositoryPath = absolutePathnameOf(repositoryFolder);
			else
				m_graphRepositoryPath = getUnitexEngine().getGraphsDir();

			m_checkLoops = flagCheckLoop;
			m_charByChar = flagCharByChar;
			m_noEmptyGraphWarning = flagNoEmptyGraphWarning;
			m_checkValidForSentence = flagCheckValidSentence;
			m_checkVariableOutput = checkVariableOutput;
			m_silentMode = silentMode;
		}

		Grf2Fst2Command::~Grf2Fst2Command()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand Grf2Fst2Command::getUnitexCommandFunction() const
		{
			return &unitex::main_Grf2Fst2;
			//return NULL;
		}

		void Grf2Fst2Command::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getCommandName());

			arguments.push_back(m_graphSourcePath.string());

			if (m_checkLoops)
				arguments.push_back("-y");

			if (!m_graphRepositoryPath.empty()) {
				arguments.push_back("-d");
				arguments.push_back(m_graphRepositoryPath.string());
			}

			if (!m_alphabetPath.empty()) {
				arguments.push_back("-a");
				arguments.push_back(m_alphabetPath.string());
				}

			if (m_charByChar)
				arguments.push_back("-c");

			if (m_noEmptyGraphWarning)
				arguments.push_back("-e");

			if (m_checkValidForSentence)
				arguments.push_back("-t");

			if (m_silentMode)
				arguments.push_back("-s");

			if (m_checkVariableOutput)
				arguments.push_back("-v");

			arguments.push_back("-o");
			arguments.push_back(m_graphTargetPath.string());

		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

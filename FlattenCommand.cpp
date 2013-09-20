/*
 * FlattenCommand.cpp
 *
 *  Created on: 6 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "FlattenCommand.h"
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
using namespace boost::filesystem;

namespace unitexcpp
{

	namespace command
	{

		FlattenCommand::FlattenCommand(UnitexEngine& unitexEngine, const path& automaton, bool unfold, bool recursive, int depth) :
			UnitexCommand(unitexEngine, "Flatten"), m_automaton(automaton)
		{
			m_unfold = unfold;
			m_recursive = recursive;
			m_depth = depth;
		}

		FlattenCommand::~FlattenCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		void FlattenCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getCommandName());

			arguments.push_back(m_automaton.string());

			if (m_unfold)
				arguments.push_back("-f");
			if (m_recursive) {
				arguments.push_back("-r");
				arguments.push_back("-d");
				ostringstream oss;
				oss << m_depth;
				arguments.push_back(oss.str());
			}
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

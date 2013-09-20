/*
 * CompressCommand.cpp
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "CompressCommand.h"
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

		CompressCommand::CompressCommand(UnitexEngine& unitexEngine, string const& dictName, bool swapInflectedAndLemmaForms) :
			UnitexCommand(unitexEngine, "Compress"), m_dictionaryName(dictName), m_swapForms(swapInflectedAndLemmaForms)
		{
		}

		CompressCommand::~CompressCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		void CompressCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getCommandName());
			if (m_swapForms)
				arguments.push_back("--flip");
			arguments.push_back(absolutePathnameOf(m_dictionaryName));
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

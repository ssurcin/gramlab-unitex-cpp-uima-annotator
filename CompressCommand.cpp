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
#include "Unitex-C++/Compress.h"

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

		UnitexCommand::fnUnitexMainCommand CompressCommand::getUnitexCommandFunction() const {
			return &unitex::main_Compress;
		}

		void CompressCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();

			Stringlist subargs;
			if (m_swapForms)
				subargs.push_back("--flip");
			subargs.push_back(absolutePathnameOf(m_dictionaryName));
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

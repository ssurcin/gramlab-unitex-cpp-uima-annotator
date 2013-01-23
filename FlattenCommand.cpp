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
#include "Unitex-C++/Flatten.h"

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

		UnitexCommand::fnUnitexMainCommand FlattenCommand::getUnitexCommandFunction() const {
			return &unitex::main_Flatten;
		}

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

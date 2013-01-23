/*
 * UnitexToolCommand.cpp
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#include "UnitexToolCommand.h"
#include "UnitexEngine.h"
#include "UnitexTool.h"

using namespace std;
using namespace unitexcpp::engine;

namespace unitexcpp
{
	namespace command
	{

		UnitexToolCommand::UnitexToolCommand(UnitexEngine& unitexEngine) :
			UnitexCommand(unitexEngine, "UnitexTool")
		{
		}

		UnitexToolCommand::~UnitexToolCommand()
		{
		}

		void UnitexToolCommand::addSubcommand(const UnitexCommand& aCommand)
		{
			Stringlist arguments;
			aCommand.buildArguments(arguments);
			subcommands.push_back(boost::tie(aCommand.getCommandName(), arguments));
		}

		/*!
		 * Adds a subcommand manually specifying the subcommand name and arguments.
		 * For internal use only.
		 */
		void UnitexToolCommand::addSubcommand(const string& strName, const Stringlist& arguments)
		{
			subcommands.push_back(boost::tie(strName, arguments));
		}

		void UnitexToolCommand::clear()
		{
			subcommands.clear();
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		void UnitexToolCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getExecutablePath().string());
			for (SubcommandList::const_iterator it = subcommands.begin(); it != subcommands.end(); it++) {
				const Subcommand& subcommand = *it;
				arguments.push_back("{");
				const string& subName = boost::tuples::get<0>(subcommand);
				arguments.push_back(subName);
				const Stringlist& subArguments = boost::tuples::get<1>(subcommand);
				Stringlist::const_iterator it = subArguments.begin();
				if (subArguments.front().find(subName) != string::npos)
					it++;
				for (; it != subArguments.end(); it++) {
					arguments.push_back(*it);
				}
				arguments.push_back("}");
			}
		}

	}
}

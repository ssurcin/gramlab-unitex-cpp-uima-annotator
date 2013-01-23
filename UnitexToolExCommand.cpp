/*
 * UnitexToolExCommand.cpp
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#include "UnitexToolExCommand.h"
#include "UnitexEngine.h"

using namespace unitexcpp::engine;
using namespace std;

namespace unitexcpp
{
	namespace command
	{

		UnitexToolExCommand::UnitexToolExCommand(UnitexEngine& unitexEngine) :
			UnitexToolCommand(unitexEngine)
		{
		}

		UnitexToolExCommand::~UnitexToolExCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Extra information
		//
		///////////////////////////////////////////////////////////////////////////

		const int UnitexToolExCommand::getNumberOfSubcommandsDone() const
		{
			return number_done;
		}

		const int UnitexToolExCommand::getErrorCode() const
		{
			return ptia.ret;
		}

		const int UnitexToolExCommand::getFaultySubcommand() const
		{
			return ptia.tool_number;
		}

		const string UnitexToolExCommand::getFaultySubcommandName() const
		{
			Stringlist args;
			buildArguments(args);

			Stringlist::const_iterator it = args.begin();
			int faulty = getFaultySubcommand();
			for (int i = 0; i < faulty; i++, it++)
				;
			return *it;
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		bool UnitexToolExCommand::executeInLibrary(const Stringlist& arguments)
		{
			ptia.argcpos = 0;
			ptia.nbargs = 0;
			ptia.ret = 0;
			ptia.tool_number = 0;

			number_done = 0;

			int argc = arguments.size();
			char** argv = stringListToCharStarArray(arguments);
			clock_t now = clock();
			int ret = UnitexTool_several_info(argc, argv, &number_done, &ptia);
			UnitexEngine::spendTimeInUnitex(clock() - now);
			deleteCharStarArray(argc, argv);
			return (ret == 0);
		}

	}
}

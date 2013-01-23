/*
 * UnitexToolExCommand.h
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef UNITEXTOOLEXCOMMAND_H_
#define UNITEXTOOLEXCOMMAND_H_

#include "UnitexToolCommand.h"
#include "UnitexTool.h"

namespace unitexcpp
{
	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class UnitexToolExCommand: public UnitexToolCommand
		{
			int number_done;
			struct pos_tools_in_arg ptia;

		public:
			UnitexToolExCommand(unitexcpp::engine::UnitexEngine& unitexEngine);
			virtual ~UnitexToolExCommand();

			const int getNumberOfSubcommandsDone() const;
			const int getErrorCode() const;
			const int getFaultySubcommand() const;
			const std::string getFaultySubcommandName() const;

		protected:
			bool executeInLibrary(const unitexcpp::Stringlist& arguments);
		};

	}
}

#endif /* UNITEXTOOLEXCOMMAND_H_ */

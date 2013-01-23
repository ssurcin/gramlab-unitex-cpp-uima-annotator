/*
 * CheckDicCommand.h
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef CHECKDICCOMMAND_H_
#define CHECKDICCOMMAND_H_

#include "UnitexCommand.h"
#include <string>

namespace unitexcpp
{
	class DictionaryType;

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class CheckDicCommand: public UnitexCommand
		{
		public:
			CheckDicCommand(unitexcpp::engine::UnitexEngine& unitexEngine, const std::string& dictName, const unitexcpp::DictionaryType& dictType);
			virtual ~CheckDicCommand();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			std::string dictionaryName;
			const unitexcpp::DictionaryType& dictionaryType;
		};

	}

}

#endif /* CHECKDICCOMMAND_H_ */

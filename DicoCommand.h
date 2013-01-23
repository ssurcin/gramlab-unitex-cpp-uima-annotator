/*
 * DicoCommand.h
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef DICOCOMMAND_H_
#define DICOCOMMAND_H_

#include "UnitexCommand.h"

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class DicoCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			DicoCommand(unitexcpp::engine::UnitexEngine& unitexEngine,
						const std::string& inputName,
						const std::string& alphabet = "",
						const unitexcpp::Stringlist& binDicNames = EMPTYSTRINGLIST,
						const unitexcpp::Stringlist& lstDictionaries = EMPTYSTRINGLIST);
			virtual ~DicoCommand();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			std::string m_inputFile;
			std::string m_alphabetFile;
			unitexcpp::Stringlist m_mainDictionaries;
			unitexcpp::Stringlist m_dynamicDictionaries;

		};

	}

}

#endif /* DICOCOMMAND_H_ */

/*
 * TokenizeCommand.h
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TOKENIZECOMMAND_H_
#define TOKENIZECOMMAND_H_

#include "UnitexCommand.h"

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class TokenizeCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			TokenizeCommand(unitexcpp::engine::UnitexEngine& unitexEngine,
							const std::string& inputName,
							const std::string& alphabet,
							bool bCharByChar,
							const std::string& inputOffsetsName ="",
							const std::string& outputOffsetsName ="");
			virtual ~TokenizeCommand();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			const std::string m_inputFile;
			const std::string m_alphabetFile;
			const bool m_charByChar;
			const std::string m_inputOffsets;
			const std::string m_outputOffsets;

		};

	}

}

#endif /* TOKENIZECOMMAND_H_ */

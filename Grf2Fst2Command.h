/*
 * Grf2Fst2Command.h
 *
 *  Created on: 6 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef GRF2FST2COMMAND_H_
#define GRF2FST2COMMAND_H_

#include "UnitexCommand.h"
#include <string>

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class Grf2Fst2Command: public unitexcpp::command::UnitexCommand
		{
		public:
			Grf2Fst2Command(unitexcpp::engine::UnitexEngine& unitexEngine,
							const boost::filesystem::path& graph,
							const boost::filesystem::path& target,
							const boost::filesystem::path& alphabet,
							const boost::filesystem::path& repositoryFolder,
							bool loopCheck,
							bool charByChar,
							bool noEmptyGraphWarning,
							bool checkValidSentence,
							bool checkVariableOutput,
							bool silentMode);
			virtual ~Grf2Fst2Command();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			boost::filesystem::path m_graphSourcePath;
			boost::filesystem::path m_graphTargetPath;
			boost::filesystem::path m_alphabetPath;
			boost::filesystem::path m_graphRepositoryPath;
			bool m_checkLoops;
			bool m_charByChar;
			bool m_noEmptyGraphWarning;
			bool m_checkValidForSentence;
			bool m_checkVariableOutput;
			bool m_silentMode;
		};

	}

}

#endif /* GRF2FST2COMMAND_H_ */

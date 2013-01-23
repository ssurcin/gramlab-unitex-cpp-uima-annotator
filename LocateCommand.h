/*
 * LocateCommand.h
 *
 *  Created on: 10 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef LOCATECOMMAND_H_
#define LOCATECOMMAND_H_

#include "UnitexCommand.h"
#include "UnitexTypes.h"
#include "Utils.h"

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class LocateCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			LocateCommand(	unitexcpp::engine::UnitexEngine& unitexEngine,
							const std::string& automaton,
							const std::string& inputFilename,
							const unitexcpp::MatchMode& matchMode = unitexcpp::MatchMode::LONGEST,
							const unitexcpp::TransductionMode& transductionMode = unitexcpp::TransductionMode::REPLACE,
							bool protectDicChars = false,
							int maxMatchNumber = -1,
							const std::string& alphabet ="",
							const unitexcpp::Stringlist& morphoDics = EMPTYSTRINGLIST,
							bool startOnSpace = false,
							bool noStartOnSpaceMatching = true,
							bool charByChar = false,
							bool wordByWord = true,
							const std::string& sntDir = "",
							bool korean = false,
							const std::string& arabicTypoRulesFile = "",
							const unitexcpp::NegationOperator& negOperator = unitexcpp::NegationOperator::TILDE,
							bool allowsAmbiguousOutput = true,
							const unitexcpp::LocateVariableErrorBehaviour& errorBehaviour = unitexcpp::LocateVariableErrorBehaviour::IGNORE,
							int warnTokenCount = -1,
							int stopTokenCount = -1);
			virtual ~LocateCommand();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

			bool getStartOnSpace() const { return bStartOnSpace; }
			void setStartOnSpace(bool flag) { bStartOnSpace = flag; }

		private:
			const std::string automatonName;
			const std::string inputName;
			const unitexcpp::MatchMode& mode;
			const unitexcpp::TransductionMode& transMode;
			const bool bProtectDicChars;
			const int maxNumberOfMatches;
			const std::string alphabetName;
			const unitexcpp::Stringlist morphoBinDics;
			bool bStartOnSpace;
			const bool bNoStartOnSpaceMatching;
			const bool bCharByChar;
			const bool bWordByWord;
			const std::string outputSntDir;
			const bool bKorean;
			const std::string arabicTypoName;
			const unitexcpp::NegationOperator& negationOperator;
			const bool bAmbiguousOutputAllowed;
			const unitexcpp::LocateVariableErrorBehaviour& variableErrorBehaviour;
			const int warnAfterTokenCount;
			const int stopAfterTokenCount;
		};

	}

}

#endif /* LOCATECOMMAND_H_ */

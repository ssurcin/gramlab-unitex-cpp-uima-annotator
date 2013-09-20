/*
 * Fst2TxtCommand.h
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef FST2TXTCOMMAND_H_
#define FST2TXTCOMMAND_H_

#include "UnitexCommand.h"
#include "UnitexTypes.h"

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class Fst2TxtCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			Fst2TxtCommand(	unitexcpp::engine::UnitexEngine& unitexEngine,
							const std::string& strInputSNT,
							const std::string& strAlphabet,
							bool bStartOnSpace,
							bool bCharByChar,
							const std::string& strFstName,
							const unitexcpp::Fst2TxtMode& aMode,
							const std::string& inputOffsetsName ="",
							const std::string& outputOffsetsName =""
							);
			virtual ~Fst2TxtCommand();

			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			const std::string inputSNT;
			const std::string alphabetFile;
			const bool startOnSpace;
			const bool charByChar;
			const std::string fstName;
			const unitexcpp::Fst2TxtMode& mode;
			const std::string inputOffsets;
			const std::string outputOffsets;
		};

	}

}

#endif /* FST2TXTCOMMAND_H_ */

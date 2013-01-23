/*
 * Fst2TxtCommand.cpp
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "Fst2TxtCommand.h"
#include "UnitexEngine.h"
#include "Unitex-C++/Fst2Txt.h"

using namespace std;
using namespace unitexcpp;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace command
	{

		Fst2TxtCommand::Fst2TxtCommand(	UnitexEngine& unitexEngine,
										const string& strInputSNT,
										const string& strAlphabet,
										bool bStartOnSpace,
										bool bCharByChar,
										const string& strFstName,
										const Fst2TxtMode& aMode,
										const string& inputOffsetsName,
										const string& outputOffsetsName) :
			UnitexCommand(unitexEngine, "Fst2Txt"), inputSNT(strInputSNT), alphabetFile(strAlphabet), startOnSpace(bStartOnSpace), charByChar(bCharByChar), fstName(strFstName), mode(aMode),
					inputOffsets(inputOffsetsName), outputOffsets(outputOffsetsName)
		{
		}

		Fst2TxtCommand::~Fst2TxtCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand Fst2TxtCommand::getUnitexCommandFunction() const {
			return &unitex::main_Fst2Txt;
		}

		void Fst2TxtCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Fst2Txt");
			arguments.push_back("-t");
			arguments.push_back(absolutePathnameOf(inputSNT));
			arguments.push_back("-a");
			arguments.push_back(absolutePathnameOf(alphabetFile));
			arguments.push_back(startOnSpace ? "-s" : "-x");
			arguments.push_back(charByChar ? "-c" : "-w");
			arguments.push_back("--" + mode.getValue());
			if (!inputOffsets.empty()) {
				arguments.push_back("--input_offsets");
				arguments.push_back(inputOffsets);
			}
			if (!outputOffsets.empty()) {
				arguments.push_back("--output_offsets");
				arguments.push_back(outputOffsets);
			}
			arguments.push_back(fstName);
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

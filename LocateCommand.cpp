/*
 * LocateCommand.cpp
 *
 *  Created on: 10 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "LocateCommand.h"
#include "UnitexEngine.h"
#include "Unitex-C++/Locate.h"
#include <boost/foreach.hpp>

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace unitexcpp;
using namespace engine;

namespace unitexcpp
{

	namespace command
	{

		/**
		 * Builds an instance of the Locate command.
		 *
		 * @param unitexEngine
		 *        a reference to a Unitex engine
		 * @param automaton
		 *        the name of an automaton file (starts with a * if virtual)
		 * @param inputFilename
		 *        the name of a text file to apply the automaton (starts with a * if virtual)
		 * @param matchMode
		 *        a matching mode (default: longest match)
		 * @param transductionMode
		 *        a transduction output mode (default: replace matched text with transduction output)
		 * @param protectDicChars
		 *        in MERGE and REPLACE transduction, protect certain characters with backslash (default: false)
		 * @param maxMatchNumber
		 *        the maximum number of matches (default: -1 is no limit)
		 * @param alphabet
		 *        complete path to the alphabet file (default: the Unitex engine's alphabet)
		 * @param morphoDics
		 *        a list of morphological dictionaries to apply (default: none)
		 * @param startOnSpace
		 *        allows search to start on spaces (default: false)
		 * @param noStartOnSpaceMatching
		 *        allows matching on spaces (default: false)
		 * @param charByChar
		 *        search character by character (default: false)
		 * @param wordByWord
		 *        search word by word (default: true)
		 * @param sntDir
		 *        specifies another directory where to store the output file than the text dictory (default: empty)
		 * @param korean
		 *        language is Korean (default: false)
		 * @param arabicTypoRulesFile
		 *        a file containing arabic typographic rules (default: empty)
		 * @param negOperator
		 *        the negation operator (default: TILDE)
		 * @param allowsAmbiguousOutput
		 *        allows ambiguous output (default: true)
		 * @param errorBehaviour
		 *        specifies the behaviour to adopt when an error of variable occurs (default: ignore)
		 * @param warnTokenCount
		 *        after how many processed tokens a warning will be issued (default: -1 is no limit)
		 * @param stopTokenCount
		 *       after how many processed tokens the process will be interrupted (default: -1 is no limit)
		 */
		LocateCommand::LocateCommand(	UnitexEngine& unitexEngine,
										const string& automaton,
										const string& inputFilename,
										const MatchMode& matchMode,
										const TransductionMode& transductionMode,
										bool protectDicChars,
										int maxMatchNumber,
										const string& alphabet,
										const Stringlist& morphoDics,
										bool startOnSpace,
										bool noStartOnSpaceMatching,
										bool charByChar,
										bool wordByWord,
										const string& sntDir,
										bool korean,
										const string& arabicTypoRulesFile,
										const NegationOperator& negOperator,
										bool allowsAmbiguousOutput,
										const LocateVariableErrorBehaviour& errorBehaviour,
										int warnTokenCount,
										int stopTokenCount) :
			UnitexCommand(unitexEngine, "Locate"), automatonName(automaton), inputName(inputFilename), mode(matchMode), transMode(transductionMode), bProtectDicChars(protectDicChars),
					maxNumberOfMatches(maxMatchNumber), alphabetName(alphabet.empty() ? unitexEngine.getAlphabetFile() : alphabet), morphoBinDics(morphoDics), bStartOnSpace(startOnSpace),
					bNoStartOnSpaceMatching(noStartOnSpaceMatching), bCharByChar(charByChar), bWordByWord(wordByWord), outputSntDir(sntDir), bKorean(korean), arabicTypoName(arabicTypoRulesFile),
					negationOperator(negOperator), bAmbiguousOutputAllowed(allowsAmbiguousOutput), variableErrorBehaviour(errorBehaviour), warnAfterTokenCount(warnTokenCount), stopAfterTokenCount(
							stopTokenCount)
		{
		}

		LocateCommand::~LocateCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand LocateCommand::getUnitexCommandFunction() const 
		{
			return &unitex::main_Locate;
			//return NULL;
		}

		void LocateCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Locate");

			arguments.push_back("-a");
			arguments.push_back(alphabetName);

			if (morphoBinDics.size() > 0) {
				arguments.push_back("-m");
				ostringstream oss;
				bool firstItem = true;
				BOOST_FOREACH(const string& morphoDict, morphoBinDics) {
					if (firstItem)
						firstItem = false;
					else
						oss << ";";
					oss << morphoDict;
				}
				arguments.push_back(oss.str());
			}

			if (bStartOnSpace)
				arguments.push_back("-s");
			if (bNoStartOnSpaceMatching)
				arguments.push_back("-x");

			if (bCharByChar)
				arguments.push_back("-c");
			if (bWordByWord)
				arguments.push_back("-w");

			if (!outputSntDir.empty()) {
				arguments.push_back("-d");
				arguments.push_back(outputSntDir);
			}

			if (bKorean)
				arguments.push_back("-K");

			if (!arabicTypoName.empty()) {
				arguments.push_back("-u");
				arguments.push_back(arabicTypoName);
			}

			if (negationOperator != NegationOperator::TILDE) {
				arguments.push_back("-g");
				arguments.push_back(negationOperator.getValue());
			}

			if (maxNumberOfMatches >= 0) {
				arguments.push_back("-n");
				ostringstream oss;
				oss << maxNumberOfMatches;
				arguments.push_back(oss.str());
			}
			else
				arguments.push_back("--all");

			if (stopAfterTokenCount >= 0) {
				arguments.push_back("-o");
				if (warnAfterTokenCount >= 0) {
					ostringstream oss;
					oss << warnAfterTokenCount << "," << stopAfterTokenCount;
					arguments.push_back(oss.str());
				}
				else {
					ostringstream oss;
					oss << stopAfterTokenCount;
					arguments.push_back(oss.str());
				}
			}

			arguments.push_back(mode.getValue());
			arguments.push_back(transMode.getValue());
			if (bProtectDicChars)
				arguments.push_back("-p");

			if (!bAmbiguousOutputAllowed)
				arguments.push_back("-z");
			else
				arguments.push_back("-b");

			if (variableErrorBehaviour != LocateVariableErrorBehaviour::IGNORE)
				arguments.push_back(variableErrorBehaviour.getValue());
			else
				arguments.push_back("-Y");

			// mandatory arguments
			arguments.push_back("-t");
			arguments.push_back(inputName);
			arguments.push_back(automatonName);
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

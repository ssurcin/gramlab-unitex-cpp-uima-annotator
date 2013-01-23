/*
 * ConcordCommand.cpp
 *
 *  Created on: 10 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "ConcordCommand.h"
#include "UnitexEngine.h"
#include "Unitex-C++/Concord.h"

using namespace std;
using namespace unitexcpp;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace command
	{

		/**
		 * Build an instance of command Concord.
		 *
		 * @param unitexEngine
		 *            an instance of Unitex engine
		 * @param aConcordFile
		 *            a raw concordance as produced by locate
		 * @param aSortedAlphabet
		 *            a sorted alphabet file
		 *            (default: the unitex engine's sorted alphabet)
		 * @param aMode
		 *            the concordance mode (see Unitex manual)
		 * @param aGlossanetScript
		 *            if mode is glossanet, this is the name of a glossanet script
		 *            file to use
		 * @param anOrder
		 *            the concordance order (see Unitex manual)
		 * @param nLeftContext
		 *            the number of characters to take in the left context
		 * @param nRightContext
		 *            the number of characters to take in the right context
		 * @param aFontName
		 *            the font name to use when mode is HTML
		 * @param nFontSize
		 *            the font size to use when mode is HTML
		 * @param bThai
		 *            true if we are processing thai
		 * @param aMergeInFile
		 *            if not null, the name (without extension) of a SNT file that
		 *            will be generated to contain the result of merging the source
		 *            text with the match outputs
		 * @throws UnitexException
		 */
		ConcordCommand::ConcordCommand(	UnitexEngine& unitexEngine,
										const string& aConcordFile,
										const string& aSortedAlphabet,
										const ConcordanceMode& aMode,
										const string& aGlossanetScript,
										const ConcordanceOrder& anOrder,
										int nLeftContext,
										int nRightContext,
										const std::string& aFontName,
										int nFontSize,
										bool bThai,
										const string& aMergeInFile) :
			UnitexCommand(unitexEngine, "Concord"), concordFile(aConcordFile), sortedAlphabetFile(aSortedAlphabet.empty() ? unitexEngine.getSortedAlphabetFile() : aSortedAlphabet), mode(aMode),
					glossanetScript(aGlossanetScript), order(anOrder), leftContext(nLeftContext), rightContext(nRightContext), fontName(aFontName), fontSize(nFontSize), thai(bThai), mergeInFile(
							aMergeInFile)
		{
		}

		ConcordCommand::~ConcordCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand ConcordCommand::getUnitexCommandFunction() const {
			return &unitex::main_Concord;
		}

		void ConcordCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Concord");

			ostringstream oss;

			string modeArg = mode.getValue();
			if (mode == ConcordanceMode::GLOSSANET)
				modeArg += "=" + glossanetScript;

			arguments.push_back("-f");
			if (fontName.find_first_of(' ') != string::npos) {
				ostringstream oss;
				oss << "\"" << fontName << "\"";
				arguments.push_back(oss.str());
			}
			else
				arguments.push_back(fontName);

			arguments.push_back("-s");
			oss << fontSize;
			arguments.push_back(oss.str());

			arguments.push_back("-l");
			oss.str("");
			oss << leftContext;
			arguments.push_back(oss.str());

			arguments.push_back("-r");
			oss.str("");
			oss << rightContext;
			arguments.push_back(oss.str());

			arguments.push_back("--" + order.getValue());

			if (mode == ConcordanceMode::REPLACE) {
				arguments.push_back("-m");
				arguments.push_back(glossanetScript);
			}
			else
				arguments.push_back("--" + modeArg);

			arguments.push_back("-a");
			arguments.push_back(sortedAlphabetFile);

			if (thai)
				arguments.push_back("-T");

			arguments.push_back(concordFile);
			if (!mergeInFile.empty()) {
				arguments.push_back("-m");
				arguments.push_back(mergeInFile);
			}
		}
	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

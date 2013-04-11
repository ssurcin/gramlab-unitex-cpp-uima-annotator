/*
 * DicoCommand.cpp
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "DicoCommand.h"
#include "UnitexEngine.h"
#include "Unitex-C++/Dico.h"
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
using namespace unitexcpp::engine;
using namespace boost;

namespace unitexcpp
{

	namespace command
	{

		DicoCommand::DicoCommand(UnitexEngine& unitexEngine, const string& inputName, const string& alphabet, const Stringlist& binDicNames, const Stringlist& lstDictionaries) :
			UnitexCommand(unitexEngine, "Dico")
		{
			m_inputFile = inputName;

			m_alphabetFile = alphabet.empty() ? unitexEngine.getAlphabetFile() : alphabet;
			Stringlist engineBinDictionaries;
			unitexEngine.getBinDictionaries(engineBinDictionaries);
			m_mainDictionaries = (binDicNames.size() == 0) ? engineBinDictionaries : binDicNames;

			if (lstDictionaries.size() == 0) {
				// Compile the .INF dynamic dictionaries into .BIN binaries
				const Stringlist& lstDynDict = unitexEngine.getDynamicDictionaries();
				Stringlist lstDynBin;
				BOOST_FOREACH(const string& dynDict, lstDynDict) {
					lstDynBin.push_back(dynDict.substr(0, dynDict.length() - 4) + ".bin");
				}
				m_dynamicDictionaries = lstDynBin;
			}
			else
				m_dynamicDictionaries = lstDictionaries;
		}

		DicoCommand::~DicoCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand DicoCommand::getUnitexCommandFunction() const 
		{
			return &unitex::main_Dico;
		}

		void DicoCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Dico");
			arguments.push_back("-a");
			arguments.push_back(absolutePathnameOf(m_alphabetFile));
			arguments.push_back("-t");
			arguments.push_back(absolutePathnameOf(m_inputFile));
			BOOST_FOREACH(const string& mainDict, m_mainDictionaries) 
				arguments.push_back(mainDict);
			BOOST_FOREACH(const string& dynDict, m_dynamicDictionaries)
				arguments.push_back(absolutePathnameOf(dynDict));
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

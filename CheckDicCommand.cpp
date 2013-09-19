/*
 * CheckDicCommand.cpp
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "CheckDicCommand.h"
#include "UnitexTypes.h"
#include "UnitexEngine.h"
#include "UnitexToolCommand.h"
#include "Unitex-C++/CheckDic.h"

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
using namespace boost::filesystem;

namespace unitexcpp
{

	namespace command
	{

		CheckDicCommand::CheckDicCommand(UnitexEngine& unitexEngine, const string& dictName, const DictionaryType& dictType, const string& alphName) :
			UnitexCommand(unitexEngine, "CheckDic"), dictionaryName(dictName), dictionaryType(dictType), alphabetName(alphName)
		{
		}

		CheckDicCommand::CheckDicCommand(UnitexEngine& unitexEngine, const path& dictPath, const DictionaryType& dictType, const path& alphabetPath) :
			UnitexCommand(unitexEngine, "CheckDic"), dictionaryName(dictPath.string()), dictionaryType(dictType), alphabetName(alphabetPath.string())
		{
		}

		CheckDicCommand::~CheckDicCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand CheckDicCommand::getUnitexCommandFunction() const {
			return &unitex::main_CheckDic;
		}

		void CheckDicCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getCommandName());
			if (dictionaryType == DictionaryType::DELAF)
				arguments.push_back("-f");
			else if (dictionaryType == DictionaryType::DELAS)
				arguments.push_back("-s");
			arguments.push_back("-a");
			arguments.push_back(absolutePathnameOf(alphabetName));
			arguments.push_back(absolutePathnameOf(dictionaryName));
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

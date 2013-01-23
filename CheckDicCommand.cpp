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

using namespace std;
using namespace unitexcpp;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace command
	{

		CheckDicCommand::CheckDicCommand(UnitexEngine& unitexEngine, const string& dictName, const DictionaryType& dictType) :
			UnitexCommand(unitexEngine, "CheckDic"), dictionaryName(dictName), dictionaryType(dictType)
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

			Stringlist subargs;
			if (dictionaryType == DictionaryType::DELAF)
				subargs.push_back("-f");
			else if (dictionaryType == DictionaryType::DELAS)
				subargs.push_back("-s");
			subargs.push_back(absolutePathnameOf(dictionaryName));
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

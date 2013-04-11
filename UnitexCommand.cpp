/*
* UnitexCommand.cpp
*
*  Created on: 4 janv. 2011
*      Author: sylvainsurcin
*/

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "UnitexCommand.h"
#include "UnitexEngine.h"
#include "Utils.h"
#include <boost/filesystem.hpp>
#include "UnitexToolCommand.h"
#include "UnitexException.h"
#include <sstream>

#if (defined(_WIN32) || defined(_WIN64))
#define WINDOWS
#include <windows.h>
#else
#undef WINDOWS
#endif

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
using namespace boost::filesystem;
using namespace unitexcpp::engine;

namespace unitexcpp
{
	namespace command
	{

		/**
		* Builds an instance of UnitexCommand to be used with a given Unitex engine,
		* and specifies the command name.
		*
		* Beware, that this is an abstract class and it cannot be instantiated. Only
		* subclasses can.
		*/
		UnitexCommand::UnitexCommand(UnitexEngine& unitexEngine, const string& aCommand) :
			m_unitexEngine(unitexEngine), m_command(aCommand)
		{
		}

		UnitexCommand::~UnitexCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Properties
		//
		///////////////////////////////////////////////////////////////////////////

		const UnitexEngine& UnitexCommand::getUnitexEngine() const {
			return m_unitexEngine;
		}

		const string& UnitexCommand::getCommandName() const
		{
			return m_command;
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Execution
		//
		///////////////////////////////////////////////////////////////////////////

		/**
		* Execute the command by calling the adequate library function, or the
		* right executable file.
		*
		* @return true if the execution was ok, or false in case of problem
		* @throws UnitexException
		*/
		bool UnitexCommand::execute()
		{
			Stringlist arguments;
			buildArguments(arguments);

			size_t argc = arguments.size();
			char** argv = stringListToCharStarArray(arguments);
#ifdef DEBUG_UIMA_CPP
			printArguments(argc, argv);
#endif

			fnUnitexMainCommand pfnCommand = getUnitexCommandFunction();
			int ret = (pfnCommand == NULL) ? 0 : (*pfnCommand)(argc, argv);

			deleteCharStarArray(argc, argv);

#ifdef DEBUG_UIMA_CPP
			if (ret != 0) 
				printUsage(pfnCommand);
#endif
			return (ret == 0);
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Paths
		//
		///////////////////////////////////////////////////////////////////////////

		/**
		* Converts a string (supposedly a filename) into an absolute path, or
		* do nothing if it is a virtual file name (i.e. starts with a *)
		*/
		string UnitexCommand::absolutePathnameOf(const std::string& argument) const
		{
			string result;
			if (isPersistedPath(argument))
				result = argument;
			else
				result = boost::filesystem::system_complete(path(argument)).string();
			return result;
		}

		path UnitexCommand::absolutePathnameOf(const path& path)
		{
			if (isPersistedPath(path))
				return path;
			else
				return system_complete(path);
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Input/Output
		//
		///////////////////////////////////////////////////////////////////////////

		ostream& operator <<(ostream& os, const UnitexCommand& command)
		{
			os << "[" << command.getCommandName() << " ";

			Stringlist arguments;
			command.buildArguments(arguments);
			size_t i = 0, n = arguments.size();
			for (Stringlist::const_iterator it = arguments.begin(); it != arguments.end(); it++) {
				os << *it;
				if (i < n - 1)
					os << " ";
			}

			os << "]";
			return os;
		}

#ifdef DEBUG_UIMA_CPP
		void UnitexCommand::printArguments(size_t argc, char** argv)
		{
			ostringstream oss;
			oss << "UnitexCommand: ";
			for (size_t i = 0; i < argc; i++) {
				if (i > 0)
					oss << " ";
				oss << argv[i];
			}
			getUnitexEngine().getAnnotator().logMessage(oss.str());
		}

		void UnitexCommand::printUsage(fnUnitexMainCommand pfnCommand)
		{
			Stringlist help;
			help.push_back(getCommandName());
			help.push_back("--help");
			char** helpArgv = stringListToCharStarArray(help);
			(*pfnCommand)(2, helpArgv);
			deleteCharStarArray(2, helpArgv);
		}
#endif
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

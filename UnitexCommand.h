/*
 * UnitexCommand.h
 *
 *  Created on: 4 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef UNITEXCOMMAND_H_
#define UNITEXCOMMAND_H_

#include "Utils.h"
#include <string>
#include <list>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace unitexcpp
{
	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class UnitexToolCommand;

		/**
		 * Abstract root class for all Unitex commands. It provides a way to factorize
		 * much of the annoying stuff for passing parameters and calling either Unitex
		 * as a library or through calls to executable files.
		 */
		class UnitexCommand
		{
			friend class UnitexToolCommand;

		public:
			UnitexCommand(unitexcpp::engine::UnitexEngine& unitexEngine, const std::string& aCommand);
			virtual ~UnitexCommand();

			const std::string& getCommandName() const;

			virtual bool execute();

			friend std::ostream& operator <<(std::ostream& os, const UnitexCommand& command);

		protected:
			typedef int (*fnUnitexMainCommand)(int argc, char* const argv[]);
			virtual fnUnitexMainCommand getUnitexCommandFunction() const =0;

			virtual void buildArguments(unitexcpp::Stringlist& arguments) const =0;
			std::string absolutePathnameOf(const std::string& argument) const;
			static boost::filesystem::path absolutePathnameOf(const boost::filesystem::path& path);

		private:
			unitexcpp::engine::UnitexEngine& m_unitexEngine;
			std::string m_command;
		protected:
			unitexcpp::engine::UnitexEngine const& getUnitexEngine() const;
		};

	}
}

#endif /* UNITEXCOMMAND_H_ */

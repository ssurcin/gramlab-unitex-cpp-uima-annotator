/*
 * UnitexToolCommand.h
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef UNITEXTOOLCOMMAND_H_
#define UNITEXTOOLCOMMAND_H_

#include "UnitexCommand.h"
#include "Utils.h"
#include <string>
#include <list>
#include <boost/tuple/tuple.hpp>

namespace unitexcpp
{
	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class UnitexToolCommand: public UnitexCommand
		{
		public:
			UnitexToolCommand(unitexcpp::engine::UnitexEngine& unitexEngine);
			virtual ~UnitexToolCommand();

		private:
			typedef boost::tuple<std::string, std::list<std::string> > Subcommand;
			typedef std::list<Subcommand> SubcommandList;
			SubcommandList subcommands;

		public:
			void addSubcommand(const UnitexCommand& aCommand);
		protected:
			friend class UnitexCommand;
			void addSubcommand(const std::string& strName, const Stringlist& arguments);
		public:
			virtual void buildArguments(unitexcpp::Stringlist& arguments) const;
			void clear();
		};

	}
}

#endif /* UNITEXTOOLCOMMAND_H_ */

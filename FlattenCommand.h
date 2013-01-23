/*
 * FlattenCommand.h
 *
 *  Created on: 6 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef FLATTENCOMMAND_H_
#define FLATTENCOMMAND_H_

#include "UnitexCommand.h"
#include <string>

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class FlattenCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			FlattenCommand(unitexcpp::engine::UnitexEngine& unitexEngine,
					       const boost::filesystem::path& automaton,
					       bool unfold,
					       bool recursive,
					       int depth);
			virtual ~FlattenCommand();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			const boost::filesystem::path& m_automaton;
			bool m_unfold;
			bool m_recursive;
			int m_depth;
		};

	}

}

#endif /* FLATTENCOMMAND_H_ */

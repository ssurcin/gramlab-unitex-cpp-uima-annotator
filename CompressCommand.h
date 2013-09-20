/*
 * CompressCommand.h
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef COMPRESSCOMMAND_H_
#define COMPRESSCOMMAND_H_

#include "UnitexCommand.h"

namespace unitexcpp
{
	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class CompressCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			CompressCommand(unitexcpp::engine::UnitexEngine& unitexEngine, std::string const& dictName, bool swapInflectedAndLemmaForms = false);
			virtual ~CompressCommand();

			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			std::string m_dictionaryName;
			bool m_swapForms;
		};

	}

}

#endif /* COMPRESSCOMMAND_H_ */

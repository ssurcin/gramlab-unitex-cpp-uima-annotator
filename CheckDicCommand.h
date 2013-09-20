/*
 * CheckDicCommand.h
 *
 *  Created on: 5 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef CHECKDICCOMMAND_H_
#define CHECKDICCOMMAND_H_

#include "UnitexCommand.h"
#include <string>
#include <boost/filesystem.hpp>

namespace unitexcpp
{
	class DictionaryType;

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class CheckDicCommand: public UnitexCommand
		{
		public:
			CheckDicCommand(unitexcpp::engine::UnitexEngine& unitexEngine, const std::string& dictName, const unitexcpp::DictionaryType& dictType, const std::string& alphName);
			CheckDicCommand(unitexcpp::engine::UnitexEngine& unitexEngine, const boost::filesystem::path& dictPath, const unitexcpp::DictionaryType& dictType, const boost::filesystem::path& alphabetPath);
			virtual ~CheckDicCommand();

			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			std::string dictionaryName;
			const unitexcpp::DictionaryType& dictionaryType;
			std::string alphabetName;
		};

	}

}

#endif /* CHECKDICCOMMAND_H_ */

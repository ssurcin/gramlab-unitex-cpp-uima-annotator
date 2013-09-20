/*
 * Fst2CheckCommand.h
 *
 *  Created on: 7 août 2012
 *      Author: sylvain
 */

#ifndef FST2CHECKCOMMAND_H_
#define FST2CHECKCOMMAND_H_

#include "UnitexCommand.h"

namespace unitexcpp
{
	namespace command
	{

		class Fst2CheckCommand:	public unitexcpp::command::UnitexCommand
		{
		public:
			Fst2CheckCommand(unitexcpp::engine::UnitexEngine& unitexEngine,
					         const boost::filesystem::path& inputPath,
					         bool sentenceCheck =false,
					         bool loopCheck =false,
					         bool emptyGraphWarning =true,
					         bool appendMode =false,
					         bool statistics =false,
					         const std::string inputEncoding ="utf8-no-bom,bom",
					         const boost::filesystem::path outputPath = boost::filesystem::path(),
					         const std::string outputEncoding ="utf8-no-bom");
			virtual ~Fst2CheckCommand();

			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			bool m_loopCheck;
			bool m_sentenceCheck;
			bool m_emptyGraphWarnings;
			boost::filesystem::path m_inputPath;
			std::string m_inputEncoding;
			boost::filesystem::path m_outputPath;
			std::string m_outputEncoding;
			bool m_appendOutputMode;
			bool m_showStatistics;
		};

	} /* namespace command */
} /* namespace unitexcpp */
#endif /* FST2CHECKCOMMAND_H_ */

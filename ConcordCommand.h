/*
 * ConcordCommand.h
 *
 *  Created on: 10 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef CONCORDCOMMAND_H_
#define CONCORDCOMMAND_H_

#include "UnitexCommand.h"
#include "UnitexTypes.h"

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class ConcordCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			ConcordCommand(unitexcpp::engine::UnitexEngine& unitexEngine, const std::string& aConcordFile, const std::string& aSortedAlphabet = "", const unitexcpp::ConcordanceMode& mode =
					unitexcpp::ConcordanceMode::HTML, const std::string& aGlossanetScript = "", const unitexcpp::ConcordanceOrder& order = unitexcpp::ConcordanceOrder::TEXT_ORDER, int nLeftContext =
					40, int nRightContext = 255, const std::string& aFontName = "Courier New", int nFontSize = 12, bool bThai = false, const std::string& mergeInFile = "");
			virtual ~ConcordCommand();

			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			const std::string& concordFile;
			const std::string& sortedAlphabetFile;
			const unitexcpp::ConcordanceMode& mode;
			const std::string& glossanetScript;
			const unitexcpp::ConcordanceOrder& order;
			const int leftContext;
			const int rightContext;
			const std::string& fontName;
			const int fontSize;
			const bool thai;
			const std::string& mergeInFile;

		};

	}

}

#endif /* CONCORDCOMMAND_H_ */

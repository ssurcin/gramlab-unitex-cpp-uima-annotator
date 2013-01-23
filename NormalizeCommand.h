/*
 * NormalizeCommand.h
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef NORMALIZECOMMAND_H_
#define NORMALIZECOMMAND_H_

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

		class NormalizeCommand: public unitexcpp::command::UnitexCommand
		{
		public:
			NormalizeCommand(unitexcpp::engine::UnitexEngine& unitexEngine,
			                 const std::string& inputName,
			                 const std::string& equivFileName,
			                 bool bNoCR,
			                 bool bNoCRLFNormalization,
			                 const std::string& offsetsName ="");
			virtual ~NormalizeCommand();

			UnitexCommand::fnUnitexMainCommand getUnitexCommandFunction() const;
			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			const std::string m_inputFilename;
			const std::string m_equivalenceFile;
		    const bool m_noCR;
		    const bool m_noCRLFNormalization;
		    const std::string m_offsetsFilename;
		};

	}

}

#endif /* NORMALIZECOMMAND_H_ */

/*
 * NormalizeCommand.cpp
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "NormalizeCommand.h"
#include "UnitexEngine.h"
#include "UnitexToolCommand.h"
#include "Unitex-C++/Normalize.h"

using namespace std;
using namespace unitexcpp;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace command
	{

		/**
		 * Builds the argument list to call Normalize according to the Unitex
		 * version.
		 *
		 * \param unitexEngine an instance of UnitexEngine
		 * \param inputName the name of the input file
		 * \param equivFileName the name of the normalization file
		 * \param bNoCR flag to replace or not CR/LF
		 * \param offsetsName a file where to store offset mappings from original input to normalized output (null if none)
		 */
		NormalizeCommand::NormalizeCommand(UnitexEngine& unitexEngine, const string& inputName, const string& equivFileName, bool bNoCR, bool bNoCRLFNormalization, const string& offsetsName) :
				UnitexCommand(unitexEngine, "Normalize"), m_inputFilename(inputName), m_equivalenceFile(equivFileName), m_noCR(bNoCR), m_noCRLFNormalization(bNoCRLFNormalization), m_offsetsFilename(
						offsetsName)
		{
		}

		NormalizeCommand::~NormalizeCommand()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// Implementation of abstract methods
		//
		///////////////////////////////////////////////////////////////////////////

		UnitexCommand::fnUnitexMainCommand NormalizeCommand::getUnitexCommandFunction() const
		{
			return &unitex::main_Normalize;
		}

		void NormalizeCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Normalize");
			arguments.push_back("-r");
			arguments.push_back(absolutePathnameOf(m_equivalenceFile));
			if (m_noCR)
				arguments.push_back("-n");
			if (m_noCRLFNormalization)
				arguments.push_back("--no_separator_normalization");
			if (!m_offsetsFilename.empty()) {
				arguments.push_back("--output_offsets");
				arguments.push_back(m_offsetsFilename);
			}
			arguments.push_back(absolutePathnameOf(m_inputFilename));
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

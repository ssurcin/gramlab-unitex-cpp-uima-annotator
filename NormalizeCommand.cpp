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
using namespace unitexcpp;
using namespace unitexcpp::engine;

namespace unitexcpp
{

	namespace command
	{

		/// <summary>
		/// Builds the argument list to call Normalize according to the Unitex
		/// version.
		/// </summary>
		/// <param name='unitexEngine'>An instance of UnitexEngine.</param>
		/// <param name='inputName'>The name (path) of the input file.</param>
		/// <param name='equivFileName'>The name (path) of the normalization file.</param>
		/// <param name='bConvertCRtoSpace'>Flag to replace CR by space.</param>
		/// <param name='bNormalizeSeparators'>Flag to normalize separators.</param>
		/// <param name='bConvertLFtoCRLF'>Flag to convert LF into CRLF.</param>
		/// <param name='offsetsName'>A file where to store offset mappings from original input to normalized output (null if none).</param>
		///
		NormalizeCommand::NormalizeCommand(
			UnitexEngine& unitexEngine, 
			const string& inputName, 
			const string& equivFileName, 
			bool bConvertCRtoSpace,
			bool bNormalizeSeparators,
			bool bConvertLFtoCRLF, 
			const string& offsetsName) :
		UnitexCommand(unitexEngine, "Normalize"), 
			m_inputFilename(inputName), 
			m_equivalenceFile(equivFileName), 
			m_noCR(bConvertCRtoSpace), 
			m_noSeparatorNormalization(!bNormalizeSeparators), 
			m_noLFtoCRLF(!bConvertLFtoCRLF),
			m_offsetsFilename(offsetsName)
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

		void NormalizeCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back("Normalize");
			arguments.push_back("-r");
			arguments.push_back(absolutePathnameOf(m_equivalenceFile));
			if (m_noCR)
				arguments.push_back("-n");
			if (m_noSeparatorNormalization)
				arguments.push_back("--no_separator_normalization");
			if (m_noLFtoCRLF)
				arguments.push_back("-l");
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

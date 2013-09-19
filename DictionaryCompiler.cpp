/*
 * DictionaryCompiler.cpp
 *
 *  Created on: 4 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "DictionaryCompiler.h"
#include "UnitexEngine.h"
#include "CheckDicCommand.h"
#include "CompressCommand.h"
#include "UnitexException.h"
#include "Utils.h"
#include "FileUtils.h"
#include <sstream>
#include <boost/filesystem.hpp>

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
using namespace boost::filesystem;

namespace unitexcpp
{
	namespace engine
	{

		set<path> DictionaryCompiler::compiledDictionaries;

		DictionaryCompiler::DictionaryCompiler(UnitexEngine& engine) :
			UnitexSubengine(engine)
		{
		}

		DictionaryCompiler::~DictionaryCompiler()
		{
		}

		/**
		 * Compile a dictionary for the current language. The dictionary is a
		 * UTF16-LE text using the DELAF or DELAS format.
		 *
		 * The method performs the following steps: 1. check the dictionary 2.
		 * compress it into a BIN dictionary.
		 *
		 * If the LibUnitex is available, use UnitexTool in order to make a single
		 * call instead of several calls.
		 *
		 * @param dictionaryName
		 *            the dictionary file name
		 * @param dictionaryType
		 *            the dictionary type (DELAF or DELAS)
		 * @param moveFile
		 *            set to true if the compiled dictionaries must be moved to
		 *            Unitex binary resources dictionary
		 * @return true if everything went well
		 */
		bool DictionaryCompiler::compile(const path& dictionaryPath, const path& alphabetPath, const DictionaryType& dictionaryType, bool moveFile)
		{
#ifdef DEBUG_UIMA_CPP
			cout << "Compiling dictionary " << dictionaryPath << " with alphabet " << alphabetPath << endl;
#endif

			set<path>::const_iterator it = compiledDictionaries.find(dictionaryPath);
			if (it != compiledDictionaries.end()) {
#ifdef DEBUG_UIMA_CPP
				cout << "  already compiled" << endl;
#endif
				return true;
			}

			if (!checkDictionary(dictionaryPath, alphabetPath)) {
				ostringstream oss;
				oss << typeid(*this).name() << " error in CheckDic";
				throw UnitexException(oss.str());
			}

			if (!compressDictionary(dictionaryPath)) {
				ostringstream oss;
				oss << typeid(*this).name() << " error in Compress";
				throw UnitexException(oss.str());
			}

			compiledDictionaries.insert(dictionaryPath);

			if (!moveFile)
				return true;

			// Move the compiled dictionary into the corresponding bin directory
			try {
				boost::filesystem::path binaryPath = boost::filesystem::change_extension(dictionaryPath, ".bin");
				boost::filesystem::path relativePath = getRelativePathFrom(getEngine().getUnitexSrcResourcesDir(), binaryPath);
				boost::filesystem::path newBinaryPath = createPathRelativeTo(getEngine().getUnitexBinResourcesDir(), relativePath);
				boost::filesystem::copy_file(binaryPath, newBinaryPath);
				boost::filesystem::remove(binaryPath);
			}
			catch (boost::filesystem::filesystem_error e) {
				ostringstream oss;
				oss << "Cannot move compiled dictionary to directory " << getEngine().getUnitexBinResourcesDir() << ": " << e.what();
				throw UnitexException(oss.str());
			}

			return true;
		}

		/**
		 * Check the consistency of a text DELAF dictionary.
		 *
		 * @param dictionaryName
		 *            name of the file containing the dictionary
		 * @param dictionaryType
		 *            DELAF or DELAS
		 * @return true if ok
		 * @throws UnitexException
		 */
		bool DictionaryCompiler::checkDictionary(const path& dictionaryPath, const path& alphabetPath, const DictionaryType& dictionaryType)
		{
			command::CheckDicCommand checkDic(getEngine(), dictionaryPath.string(), dictionaryType, alphabetPath);
			return checkDic.execute();
		}

		/**
		 * Compress a text DELA dictionary into a BIN dictionary. The dictionary
		 * must be DELA compliant (see checkDictionary).
		 *
		 * @param dicName
		 *            the name of the file containing the dictionary
		 * @param swapInflectedAndLemmaForms
		 *            indicates whether these formes must be swapped
		 * @return true if ok
		 * @throws UnitexException
		 */
		bool DictionaryCompiler::compressDictionary(const path& dictionaryPath, bool swapInflectedAndLemmaForms)
		{
			command::CompressCommand compress(getEngine(), dictionaryPath.string(), swapInflectedAndLemmaForms);
			return compress.execute();
		}
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

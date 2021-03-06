/*
* TextPreprocessor.cpp
*
*  Created on: 7 janv. 2011
*      Author: sylvainsurcin
*/

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "TextPreprocessor.h"
#include "UnitexEngine.h"
#include "UnitexTypes.h"
#include "UnitexException.h"
#include "NormalizeCommand.h"
#include "Fst2TxtCommand.h"
#include "TokenizeCommand.h"
#include "DicoCommand.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include "FileUtils.h"

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
using namespace unitexcpp::command;
using namespace uima;
using namespace icu;
using namespace boost::filesystem;

namespace unitexcpp
{

	namespace engine
	{

		/**
		* Builds a new instance of TextPreprocessor for a given Unitex engine.
		*/
		TextPreprocessor::TextPreprocessor(UnitexEngine& unitexEngine) :
			UnitexSubengine(unitexEngine)
		{
		}

		TextPreprocessor::~TextPreprocessor()
		{
		}

		///////////////////////////////////////////////////////////////////////
		//
		// Public interface
		//
		///////////////////////////////////////////////////////////////////////

		/**
		* Preprocesses an input file.
		*
		* @param inputFilename
		*           the input file name
		*/
		bool TextPreprocessor::preprocess(const string& inputFilename)
		{
#ifdef DEBUG_UIMA_CPP
			cout << "----------------------------------------------------------------------" << endl;
			cout << "1) Preprocessing for " << getEngine().getAnnotator().getDocumentIdAsString() << " (" << inputFilename << ")" << endl;
			cout << "----------------------------------------------------------------------" << endl;
#endif
			string sntName = UnitexEngine::buildSntFileNameFrom(inputFilename);
			path sntPath = UnitexEngine::buildSntDirNameFrom(inputFilename);

			// Create the subdirectory for SNT normalized files
			if (!isVirtualPath(sntPath)) {
				if (!exists(sntPath)) {
					create_directory(sntPath);
				}
			}

#ifdef DEBUG_UIMA_CPP
			cout << "Normalizing input text" << endl;
#endif
			path normOffsets = sntPath / "normoffsets.txt";
			if (!normalize(inputFilename, getEngine().getNormalizationDictionaryFile(), false, false, false, normOffsets.string())) {
				ostringstream oss;
				oss << "TextPreprocessor error while normalizing input file " << inputFilename;
				throw new UnitexException(oss.str());
			}
#ifdef DEBUG_UIMA_CPP
			cout << "Contents of " << normOffsets << ":" << endl;
			UnicodeString ustr;
			getUnicodeStringFromUnitexFile(normOffsets, ustr);
			cout << ustr << endl;

			cout << "Normalized input = " << endl;
			ustr = "";
			getUnicodeStringFromUnitexFile(sntName, ustr);
			cout << ustr << endl;
#endif

			path sntOffsetsPathAfterSentence = sntPath / "sntoffsets_1.txt";
#ifdef DEBUG_UIMA_CPP
			cout << "Sentence automaton = " << getEngine().getSentenceFstFile() << endl;
#endif
			if (!fst2txt(sntName, getEngine().getAlphabetFile(), false, false, getEngine().getSentenceFstFile(), Fst2TxtMode::MERGE, normOffsets.string(), sntOffsetsPathAfterSentence.string())) {
				ostringstream oss;
				oss << "TextPreprocessor error while applying Sentence formatting automaton on input file " << inputFilename;
				throw new UnitexException(oss.str());
			}
#ifdef DEBUG_UIMA_CPP
			cout << "Contents of " << sntOffsetsPathAfterSentence << ":" << endl;
			getUnicodeStringFromUnitexFile(sntOffsetsPathAfterSentence, ustr);
			cout << ustr << endl;

			cout << "Normalized input = " << endl;
			ustr = "";
			getUnicodeStringFromUnitexFile(sntName, ustr);
			cout << ustr << endl;
#endif

			path sntOffsetsPathAfterReplace = sntPath / "sntoffsets.txt";
			if (!fst2txt(sntName, getEngine().getAlphabetFile(), false, false, getEngine().getReplaceFstFile(), Fst2TxtMode::REPLACE, sntOffsetsPathAfterSentence.string(),
				sntOffsetsPathAfterReplace.string())) {
					ostringstream oss;
					oss << "TextPreprocessor error while applying Replace formatting automaton on input file " << inputFilename;
					throw new UnitexException(oss.str());
			}
#ifdef DEBUG_UIMA_CPP
			cout << "Contents of " << sntOffsetsPathAfterReplace << ":" << endl;
			getUnicodeStringFromUnitexFile(sntOffsetsPathAfterReplace, ustr);
			cout << ustr << endl;

			cout << "Normalized input = " << endl;
			ustr = "";
			getUnicodeStringFromUnitexFile(sntName, ustr);
			cout << ustr << endl;
#endif

			path sntTokensPath = sntPath / "snttokens.txt";
			if (!tokenize(sntName, getEngine().getAlphabetFile(), false, sntOffsetsPathAfterReplace.string(), sntTokensPath.string())) {
				ostringstream oss;
				oss << "TextPreprocessor error while tokenizing input file " << inputFilename;
				throw new UnitexException(oss.str());
			}
#ifdef DEBUG_UIMA_CPP
			cout << "Contents of " << sntTokensPath << ":" << endl;
			getUnicodeStringFromUnitexFile(sntTokensPath, ustr);
			cout << ustr << endl;
#endif

			Stringlist dictionaries;
			getEngine().getBinDictionaries(dictionaries);
			if (!applyDictionaries(sntName, getEngine().getAlphabetFile(), dictionaries, getEngine().getDynamicDictionaries())) {
				ostringstream oss;
				oss << "TextPreprocessor error while applying dictionaries on input file " << inputFilename;
				throw new UnitexException(oss.str());
			}
#ifdef DEBUG_UIMA_CPP
			cout << "Simple words:" << endl;
			getUnicodeStringFromUnitexFile(sntPath / "dlf", ustr);
			cout << ustr << endl << endl;
			cout<< "Compound words: " << endl;
			getUnicodeStringFromUnitexFile(sntPath / "dlc", ustr);
			cout << ustr << endl << endl;
			cout << "Unknown words: " << endl;
			getUnicodeStringFromUnitexFile(sntPath / "err", ustr);
			cout << ustr << endl << endl;
			cout << "Tags: " << endl;
			getUnicodeStringFromUnitexFile(sntPath / "tags.ind", ustr);
			cout << ustr << endl;
#endif

#ifdef DEBUG_UIMA_CPP
			cout << "----------------------------------------------------------------------" << endl;
			cout << "End of preprocessing for " << getEngine().getAnnotator().getDocumentIdAsString() << endl;
			cout << "----------------------------------------------------------------------" << endl;
#endif
			return true;
		}

		/**
		* Launch Unitex's Normalize module.
		*
		* @param inputName
		*            the name of the input file
		* @param equivFileName
		*            the name of the normalization file
		* @param bNoCR
		*            flag to replace or not CR/LF
		* \param noSeparatorNormalization
		*            flag indicating that the separators should not be modified
		*            (i.e. LF does not become CRLF for instance)
		* \param offsetsFileName a file where to store offset mappings from original input to normalized output (null if none)
		* @return true if the process executed ok
		* @throws UnitexException
		*/
		bool TextPreprocessor::normalize(const string& inputName, const string& equivFileName, bool bConvertCRtoSpace, bool bNormalizeSeparators, bool bConvertLFtoCRLF, const string& offsetsFileName)
		{
			NormalizeCommand command(getEngine(), inputName, equivFileName, bConvertCRtoSpace, bNormalizeSeparators, bConvertLFtoCRLF, offsetsFileName);
			return command.execute();
		}

		/**
		* Applies a FST to a text file (mainly for preprocessing purpose, such as
		* marking sentences).
		*
		* @param inputName
		*            the SNT file on which to work
		* @param alphabet
		*            the alphabet file (default: the Unitex engine instance's alphabet)
		* @param startOnSpace
		*            true if the application starts on space (default: false)
		* @param charByChar
		*            true if the process is done char by char (default: false)
		* @param fstName
		*            the FST file to apply (default: the Unitex engine instance's sentence FST2)
		* @param mode
		*            MERGE or REPLACE (default: MERGE)
		* \param inputOffsets
		*            a file containing offset mappings produced by Normalize (may be null)
		* \param outputOffsets
		*            a filename where to store offset mappings from original input
		*            to normalized output (null if none)
		* @return true if no error
		* @throws UnitexException
		*/
		bool TextPreprocessor::fst2txt(const string& inputName, const string& alphabet, bool startOnSpace, bool charByChar, const string& fstName, const Fst2TxtMode& mode, const string& inputOffsets,
			const string& outputOffsets)
		{
			Fst2TxtCommand command(getEngine(), inputName, alphabet.empty() ? getEngine().getAlphabetFile() : alphabet, startOnSpace, charByChar,
				fstName.empty() ? getEngine().getSentenceFstFile() : fstName, mode, inputOffsets, outputOffsets);
			return command.execute();
		}

		/**
		* Tokenize the input text.
		*
		* @param inputName
		*            the name of the input file
		* @param alphabet
		*            a file name specifying the alphabet. If empty (by default) we
		*            use the default alphabet for the current language
		*            (../Alphabet.txt)
		* @param charByChar
		*            if true, the program does a char by char tokenization (except
		*            for the tags {S}, {STOP} or like {today,.ADV}). this mode may
		*            be used for languages like Thai.
		* \param inputOffsets
		*            a file containing offset mappings produced by Normalize (may be null)
		* \param outputOffsets
		*            a filename where to store offset mappings from original input
		*            to normalized output (null if none)
		* @throws UnitexException
		*/
		bool TextPreprocessor::tokenize(const string& inputName, const string& alphabet, bool charByChar, const string& inputOffsets, const string& outputOffsets)
		{
			TokenizeCommand command(getEngine(), inputName, alphabet.empty() ? getEngine().getAlphabetFile() : alphabet, charByChar, inputOffsets, outputOffsets);
			return command.execute();
		}

		/**
		* Apply all dictionaries for the current language to extract morphological
		* and semantic information.
		*
		* @param inputName
		*            the input file name
		* @param alphabet
		*            the alphabet file (default: the Unitex engine instance's alphabet)
		* @param binDicNames
		*            the main binary dictionaries for the language forms
		*            (default: the Unitex engine instance's BIN dictionaries)
		* @param lstDictionaries
		*            a list of binary dynamic dictionary names
		*            (default: the Unitex engine instance's dynamic dictionaries)
		* @return true if no error
		* @throws UnitexException
		*/
		bool TextPreprocessor::applyDictionaries(const string& inputName, const string& alphabet, const Stringlist& binDicNames, const Stringlist& lstDictionaries)
		{
			Stringlist engineDictionaries; // define here because we might use a reference to it
			Stringlist& currentBinDicNames = const_cast<Stringlist&>(binDicNames);
			if (binDicNames.size() == 0) {
				getEngine().getBinDictionaries(engineDictionaries);
				currentBinDicNames = engineDictionaries;
			}

			Stringlist& currentDynDicNames = const_cast<Stringlist&>(lstDictionaries);
			if (lstDictionaries.size() == 0) {
				Stringlist lst;
				const Stringlist& dynDics = getEngine().getDynamicDictionaries();
				BOOST_FOREACH(const string& dynDic, dynDics) {
					lst.push_back(dynDic.substr(0, dynDic.length() - 4) + ".bin");
				}
				currentDynDicNames = lst;
			}

			DicoCommand command(getEngine(), inputName, alphabet.empty() ? getEngine().getAlphabetFile() : alphabet, currentBinDicNames, currentDynDicNames);
			return command.execute();
		}

		///////////////////////////////////////////////////////////////////////
		//
		// Tools
		//
		///////////////////////////////////////////////////////////////////////

		/**
		* Add a Unitex sentence mark {S} at the beginning of the normalized text,
		* because Unitex does not do it by itself, and therefore we cannot detect
		* the 1st sentence.
		*
		* @param sntFilename
		*            the normalized text file name
		*/
		void TextPreprocessor::addInitialSentenceMark(const string& sntFilename)
		{
			UnicodeString normalizedText;
			getEngine().getNormalizedText(normalizedText, sntFilename);

			// Find the first non white space character
			int32_t length = normalizedText.length(), i;
			for (i = 0; (i < length) && u_isUWhiteSpace(normalizedText.char32At(i)); i++)
				;

			// Extract all white spaces on the left
			UnicodeString whiteSpaces;
			if (i > 0)
				normalizedText.extract(0, i - 1, whiteSpaces);

			// Extract the rest of the string
			UnicodeString rest;
			normalizedText.extract(i, length - i, rest);

			// Insert the sentence mark
			UnicodeString newNormalizedText = whiteSpaces + UNICODE_STRING_SIMPLE("{S}") + rest;

			// Replace the normalized text
			setNormalizedText(sntFilename, newNormalizedText);
		}

		/**
		* Sets the content of the normalized text to be used by Unitex
		*
		* @param sntFilename
		*            the normalized text file name
		* @param normalizedText
		*            the new content of the file
		*/
		void TextPreprocessor::setNormalizedText(const string& sntFilename, const UnicodeStringRef& normalizedText)
		{
			writeUnitexFile(sntFilename, normalizedText);
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

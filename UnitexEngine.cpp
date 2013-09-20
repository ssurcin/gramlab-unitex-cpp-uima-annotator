/*
* UnitexEngine.cpp
*
*  Created on: 28 déc. 2010
*      Author: sylvainsurcin
*/

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "UnitexEngine.h"
#include "Language.h"
#include <unicode/ustream.h>
#include <unicode/ustdio.h>
#include <iostream>
#include <sstream>
#include <map>
#include "JavaLikeEnum.h"
#include <map>
#include <set>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "Unitex-C++/Af_stdio.h"
#include "Unitex-C++/AbstractFilePlugCallback.h"
#include "Unitex-C++/VirtualFiles.h"
#include "FileUtils.h"
#include "VirtualFolderCleaner.h"
#include <boost/foreach.hpp>

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
using namespace boost::algorithm;
using namespace icu;

namespace unitexcpp
{

	namespace engine
	{

		///////////////////////////////////////////////////////////////////////
		//
		// Static members
		//
		///////////////////////////////////////////////////////////////////////

		long UnitexEngine::ms_unitexRuntime = 0L;
		long UnitexEngine::ms_vfsRuntime = 0L;
		bool UnitexEngine::ms_allowedToStdout = true;
		bool UnitexEngine::ms_isVfsInitialized = false;

		///////////////////////////////////////////////////////////////////////
		//
		// UnitexEngine
		//
		///////////////////////////////////////////////////////////////////////

		/**
		* Builds an instance of Unitex engine for a given language, and preloads
		* resources.
		*
		* @param annotator
		*            the annotator
		* @param normalizedLanguage
		*            the normalized form of a language (e.g. "en" or "fr")
		* @param resourcesDir
		*            the path to the local Unitex source resources (.grf, .fst2, .dic and .bin)
		*            working directory (partially equivalent to "My Unitex")
		* @param dictionariesPaths
		* 			  a set of dictionary paths
		* @param automataPaths
		* 			  a set of automata paths
		*/
		UnitexEngine::UnitexEngine(UnitexAnnotatorCpp const& annotator, UnicodeString const& normalizedLanguage, const path& resourcesDir, const vector<UnicodeString>& dictionaries,
			const vector<UnicodeString>& automata, const map<UnicodeString, UnicodeString>& morphoDictNames) :
		m_annotator(annotator), m_language(Language::getLanguage(normalizedLanguage)), m_languageResources(*this, m_language), m_dictionaryCompiler(*this), m_graphCompiler(*this), m_textPreprocessor(
			*this), m_textProcessor(*this)
		{
			m_unitexSrcResourcesDir = resourcesDir;
			m_unitexBinResourcesDir = resourcesDir;

			m_validResources = m_languageResources.initialize("Norm.txt", "Sentence.fst2", "Replace.fst2", "Alphabet.txt", "Alphabet_sort.txt");
			m_languageResources.setAutomata(automata);
			m_languageResources.setBinaryDictionaries(dictionaries);
			m_languageResources.setMorphologicalDictionaries(morphoDictNames);
		}

		/**
		* Builds an instance of Unitex engine for a given language, and preloads
		* resources.
		*
		* @param annotator
		*            the annotator
		* @param normalizedLanguage
		*            the normalized form of a language (e.g. "en" or "fr")
		* @param srcResourcesDir
		*            the path to the local Unitex source resources (.grf and .dic)
		*            working directory (partially equivalent to "My Unitex") * @param
		* @param binResourcesDir
		*            the path to the local Unitex compiled resources (.fst2 and
		*            .bin) working directory (partially equivalent to "My Unitex")
		* @param dictionariesPaths
		* 			  a set of dictionary paths
		* @param automataPaths
		* 			  a set of automata paths
		*/
		UnitexEngine::UnitexEngine(UnitexAnnotatorCpp const& annotator, UnicodeString const& normalizedLanguage, const path& srcResourcesDir, const path& binResourcesDir,
			const vector<UnicodeString>& dictionaries, const vector<UnicodeString>& automata, const map<UnicodeString, UnicodeString>& morphoDictNames) :
		m_annotator(annotator), m_language(Language::getLanguage(normalizedLanguage)), m_languageResources(*this, m_language), m_dictionaryCompiler(*this), m_graphCompiler(*this), m_textPreprocessor(
			*this), m_textProcessor(*this)
		{
			m_unitexSrcResourcesDir = srcResourcesDir;
			m_unitexBinResourcesDir = binResourcesDir.empty() ? m_unitexSrcResourcesDir : binResourcesDir;

			m_validResources = m_languageResources.initialize("Norm.txt", "Sentence.fst2", "Replace.fst2", "Alphabet.txt", "Alphabet_sort.txt");
			m_languageResources.setAutomata(automata);
			m_languageResources.setBinaryDictionaries(dictionaries);
			m_languageResources.setMorphologicalDictionaries(morphoDictNames);
		}

		UnitexEngine::~UnitexEngine()
		{
#ifdef DEBUG_UIMA_CPP
			cout << "Destroying a UnitexEngine " << endl;
#endif
		}

		bool UnitexEngine::validResources() const
		{
			return m_validResources;
		}

		const path& UnitexEngine::getUnitexSrcResourcesDir() const
		{
			return m_unitexSrcResourcesDir;
		}

		const path& UnitexEngine::getUnitexBinResourcesDir() const
		{
			return m_unitexBinResourcesDir;
		}

		path UnitexEngine::getGraphsDir() const
		{
			return (getUnitexSrcResourcesDir() / m_language.getNormalizedForm()) / "Graphs";
		}

		const string UnitexEngine::getAlphabetFile() const
		{
			return m_languageResources.getAlphabetPath().string();
		}

		const string UnitexEngine::getSortedAlphabetFile() const
		{
			return m_languageResources.getSortedAlphabetPath().string();
		}

		const string UnitexEngine::getNormalizationDictionaryFile() const
		{
			return m_languageResources.getNormalizationDictionaryPath().string();
		}

		void UnitexEngine::setSentenceFstFile(const string& fileName)
		{
			m_languageResources.setSentenceAutomaton(fileName);
		}

		const string UnitexEngine::getSentenceFstFile() const
		{
			return m_languageResources.getSentenceAutomatonPath().string();
		}

		const string UnitexEngine::getReplaceFstFile() const
		{
			return m_languageResources.getReplaceAutomatonPath().string();
		}

		Stringlist::size_type UnitexEngine::getBinDictionaries(Stringlist& list) const
		{
			list.clear();
			set<path> dictionaryPaths = m_languageResources.getBinDictionariesPath();
			BOOST_FOREACH(const path& binPath, dictionaryPaths) {
				path actualPath = m_languageResources[binPath];
				list.push_back(actualPath.string());
			}
			return list.size();
		}

		const Stringlist& UnitexEngine::getDynamicDictionaries() const
		{
			return m_dynamicDictionaries;
		}

		void UnitexEngine::getMorphologicalDictionaries(const string& automatonPath, Stringlist& morphoDictList) const
		{
			m_languageResources.getMorphologicalDictionaries(automatonPath, morphoDictList);
		}

		/**
		* Gets the path to the SNT directory corresponding to the current input file
		* name.
		*/
		path UnitexEngine::getSntDirectory() const
		{
			string pathname;
			size_t dot = m_inputFilename.find_last_of('.');
			if ((dot != string::npos) && (dot <= m_inputFilename.length() - 2))
				pathname = m_inputFilename.substr(0, dot) + "_snt";
			else
				pathname = m_inputFilename + "_snt";
			return path(pathname);
		}

		/**
		* Resets the dynamic dictionary list used by this engine.
		*/
		void UnitexEngine::clearDynamicDictionaries()
		{
			m_dynamicDictionaries.clear();
		}

		/**
		* Add a dynamic dictionary for the given language into the current session.
		* The dictionary is a UTF16-LE text using the DELAF or DELAS format.
		*
		* The method performs the following steps: 1. check the dictionary 2.
		* compress it into a BIN dictionary.
		*
		* If possible, use UnitexTool in order to make a single call.
		*
		* @param strDictName
		*            the dictionary file name
		* @param dictionaryType
		*            the dictionary type (DELAF or DELAS)
		* @return true if everything went well
		*/
		bool UnitexEngine::addDynamicDictionary(string const& strDictName, DictionaryType const& dictionaryType)
		{
			bool compileOk = m_dictionaryCompiler.compile(strDictName, getAlphabetFile(), dictionaryType);

			// Add this dictionary to the given language list
			if (compileOk)
				m_dynamicDictionaries.push_back(strDictName);

			return true;
		}

		///////////////////////////////////////////////////////////////////////
		//
		// Preprocessing
		//
		///////////////////////////////////////////////////////////////////////

		/*!
		* Builds the expected corresponding SNT filename for a given input file
		* name.
		*
		* \param fileName the input file name
		* \return the corresponding SNT file name
		*/
		string UnitexEngine::buildSntFileNameFrom(const string& filename)
		{
			path originalPath(filename);
			path sntPath = change_extension(originalPath, ".snt");
			return sntPath.string();
		}

		/*!
		* Builds the expected corresponding SNT directory name for a given input file
		* name.
		*
		* \param fileName the input file name
		* \return the corresponding SNT directory name
		*/
		string UnitexEngine::buildSntDirNameFrom(const string& filename)
		{
			path originalPath(filename);
			path sntPath = change_extension(originalPath, "");
			return sntPath.string() + "_snt";
		}

		void UnitexEngine::getNormalizedText(UnicodeString& normalizedText, const string& aFileName) const
		{
			const string& filename = aFileName.empty() ? m_sntFilename : aFileName;
			getUnicodeStringFromUnitexFile(filename, normalizedText);
		}

		void UnitexEngine::getInputText(UnicodeString& inputText, const string& aFileName) const
		{
			const string& filename = aFileName.empty() ? m_inputFilename : aFileName;
			getUnicodeStringFromUnitexFile(filename, inputText);
		}

		///////////////////////////////////////////////////////////////////////
		//
		// Performance indicators
		//
		///////////////////////////////////////////////////////////////////////

		void UnitexEngine::spendTimeInVFS(long ms)
		{
			ms_vfsRuntime += ms;
		}

		void UnitexEngine::spendTimeInUnitex(long ms)
		{
			ms_unitexRuntime += ms;
		}

		long UnitexEngine::getTimeSpentInVFS()
		{
			return ms_vfsRuntime;
		}

		long UnitexEngine::getTimeSpentInUnitex()
		{
			return ms_unitexRuntime;
		}

		///////////////////////////////////////////////////////////////////////
		//
		// Stdout
		//
		///////////////////////////////////////////////////////////////////////

		/*!
		* Prevents the Unitex library to write to stdout
		*/
		void UnitexEngine::preventWritingToStdout()
		{
			if (ms_allowedToStdout) {
				enum stdwrite_kind swk = stdwrite_kind_out;
				ms_allowedToStdout = (SetStdWriteCB(swk, 1, NULL, NULL) == 1);
				swk = stdwrite_kind_err;
				ms_allowedToStdout &= (SetStdWriteCB(swk, 1, NULL, NULL) == 1);
			}
		}

		size_t UnitexEngine::initializedVirtualFileSystem()
		{
			if (!ms_isVfsInitialized) {
				unitex::virtualfile::init_virtual_files();
				ms_isVfsInitialized = true;
			}
			return GetNbAbstractFileSpaceInstalled();
		}

		///////////////////////////////////////////////////////////////////////
		//
		// Run
		//
		///////////////////////////////////////////////////////////////////////

		/*!
		* Clears performance cache to avoid cumulating results bewteen 2 messages.
		*/
		void UnitexEngine::clearPerformanceCache()
		{
			m_textProcessor.clearPerformanceMap();
		}

		/**
		* Run a whole process from the point of view of PulseUIMA for an input
		* file.
		*
		* @param result
		*            a list of QualifiedString where the tranduscers' outputs
		*            will be stored.
		* @param inputFile
		*            the name of the input file containing the text to analyze
		*            (empty by default, and we expect the input file to have been
		*            prepared before calling).
		*/
		list<QualifiedString> UnitexEngine::run(const string& inputFile)
		{
#ifdef DEBUG_UIMA_CPP
			cout << "UnitexEngine run on " << inputFile << endl;
#endif
			list<QualifiedString> result;
			m_inputFilename = inputFile;

#ifdef DEBUG_UIMA_CPP
			cout << "Input-------------" << endl;
			UnicodeString uString;
			if (!getUnicodeStringFromUnitexFile(inputFile, uString))
				cout << "!!!! ERROR !!!!" << endl << "Cannot read " << inputFile << endl;
			else
				cout << uString << endl;
			cout << "End of input------" << endl;
#endif

			result.clear();

			// Check that all necessary resources are ok and loaded
			m_languageResources.check();

			m_textPreprocessor.preprocess(m_inputFilename);
			m_sntFilename = change_extension(m_inputFilename, ".snt").string();

			BOOST_FOREACH(path const& automatonPath, m_languageResources.getAutomataPaths()) {
				path actualAutomatonPath = m_languageResources[automatonPath];
				string automaton = actualAutomatonPath.string();
				if (getAnnotator().isLoggingEnabled())
					getAnnotator().logMessage("Applying automaton %s", automaton.c_str());

				// For DateTime graph, we allow Unitex's locate to start on spaces
				// as well as on full characters
				//bool locateStartOnSpace = false;
				//if (automaton.find("DateTime-Root.fst2") != string::npos)
				//	locateStartOnSpace = true;
				bool locateStartOnSpace = true;
				// Check eventual double analysis
				list<QualifiedString> automatonOutput;
#ifdef DEBUG_UIMA_CPP
				cout << "run Text processor" << endl;
#endif
				m_textProcessor.processAndCheck(automatonOutput, m_sntFilename, automaton, locateStartOnSpace);
#ifdef DEBUG_UIMA_CPP
				cout << "check double analysis" << endl;
#endif
				checkDoubleAnalysis(automaton, automatonOutput);
#ifdef DEBUG_UIMA_CPP
				cout << automatonOutput.size() << " unambiguous outputs" << endl;
				for (list<QualifiedString>::const_iterator it = automatonOutput.begin(); it != automatonOutput.end(); it++)
					cout << "- " << *it << endl;
#endif
				result.splice(result.end(), automatonOutput);
			}

			result.sort();
			result.unique();

			return result;
		}

		/**
		* Browses a list of automaton outputs to issue a warning message when a
		* double analysis is detected, i.e. when 2 analysis start at the same
		* offset.
		*
		* @param automaton
		*            the automaton's name
		* @param automatonOutput
		*            the list of qualified strings output of the automaton
		*/
		void UnitexEngine::checkDoubleAnalysis(const string& automaton, const list<QualifiedString>& automatonOutput)
		{
			bool debug = false;

			map<int32_t, list<QualifiedString> > analysis;
			for (list<QualifiedString>::const_iterator it = automatonOutput.begin(); it != automatonOutput.end(); it++) {
				if (debug)
					cout << "QualifiedString " << *it << endl;
				int32_t offset = it->getStart();
				if (debug)
					cout << "offset = " << offset << endl;
				map<int32_t, list<QualifiedString> >::iterator jt = analysis.find(offset);
				if (jt == analysis.end()) {
					if (debug)
						cout << "offset not found in map" << endl;
					list<QualifiedString> l;
					l.push_back(*it);
					analysis[offset] = l;
				} else {
					jt->second.push_back(*it);
					if (debug)
						cout << "offset found in map, now " << jt->second.size() << " items" << endl;
				}
			}
			if (debug)
				cout << "--" << endl;
			for (map<int32_t, list<QualifiedString> >::iterator jt = analysis.begin(); jt != analysis.end(); jt++) {
				list<QualifiedString>& l = jt->second;
				if (debug)
					cout << jt->first << " has " << l.size() << " items" << endl;
				if (l.size() > 1) {
					ostringstream oss;
					oss << "Double analysis in automaton '" << automaton << "' at offset " << jt->first << ":" << endl;
					for (list<QualifiedString>::const_iterator kt = l.begin(); kt != l.end(); kt++)
						oss << "\t" << kt->getString() << endl;
					getAnnotator().logWarning(oss.str());
				}
			}
		}

	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

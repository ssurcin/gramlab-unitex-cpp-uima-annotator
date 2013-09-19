/*
* LanguageResources.cpp
*
*  Created on: 28 déc. 2010
*      Author: sylvainsurcin
*/

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4800)
#endif

#include "LanguageResources.h"
#include "UnitexAnnotatorCpp.h"
#include "Utils.h"
#include "FileUtils.h"
#include "UnitexException.h"
#include "UnitexEngine.h"
#include "Language.h"
#include <time.h>
#include <unicode/ustream.h>
#include <iostream>
#include <sstream>
#include "CompressedDic.h"
#include "Fst2.h"
#include "Alphabet.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include "Unitex-C++/UnitexLibIO.h"
#include "Unitex-C++/PersistenceInterface.h"
#include "Unitex-C++/AbstractDelaPlugCallback.h"
#include "Unitex-C++/AbstractFst2PlugCallback.h"

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
using namespace boost;
using namespace boost::filesystem;

static const UnicodeString EmptyUnicodeString = "";

namespace unitexcpp
{

	///////////////////////////////////////////////////////////////////////////////
	//
	// Static members
	//
	///////////////////////////////////////////////////////////////////////////////

	LanguageResources::PersistedResourceCollection LanguageResources::ms_persistedResources;
	size_t LanguageResources::ms_livingInstances = 0;

	///////////////////////////////////////////////////////////////////////////////
	//
	// Constructors
	//
	///////////////////////////////////////////////////////////////////////////////

	LanguageResources::LanguageResources(engine::UnitexEngine& anEngine, Language const& aLanguage) :
		engine(anEngine), language(aLanguage), bAutomataOk(false), m_bDictionariesAreSet(false)
	{
		ms_livingInstances++;
	}

	/**
	* Builds a LanguageInfo structure. Some fields are automatically built,
	* using getUnitexRootDir(). The mandatory BIN dictionaries are left
	* intentionally blank because they should be initialized AFTER the
	* creation of this structure by using the JUnitex.setMandatoryBinDicts
	* method.
	*
	* To avoid the automatic building, use the empty constructor and set
	* the fields.
	*/
	bool LanguageResources::initialize(path const& normDicPath, string const& sentenceName, string const& replaceName, string const& alphName, string const& alphSortName)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

#ifdef DEBUG_UIMA_CPP
		cout << "Initializing language resources for " << language.getNormalizedForm() << endl;
#endif
		m_languageDirectoryPath = engine.getUnitexSrcResourcesDir() / language.getNormalizedForm();
		m_automataDirectoryPath = m_languageDirectoryPath / "Graphs";
		m_preprocessingDirectoryPath = m_automataDirectoryPath / "Preprocessing";
		m_dictionaryDirectoryPath = m_languageDirectoryPath / "Dela";
#ifdef DEBUG_UIMA_CPP
		cout << "language dir path = " << m_languageDirectoryPath << endl;
#endif

		// Normalization dictionary (only virtualized)
		if (!initializeNormalizationDictionary(normDicPath)) return false;

		// Alphabet file (virtualized & persisted)
		if (!initializeAlphabet(alphName, false)) return false;

		// Sorted alphabet file (virtualized & persisted)
		if (!initializeAlphabet(alphSortName, true)) return false;

		// Sentence formatting graph (virtualized & persisted)
		if (!initializeSpecialAutomaton(sentenceName, SENTENCE)) return false;

		// Replacement graph (persisted)
		if (!initializeSpecialAutomaton(replaceName, REPLACE)) return false;

		return true;
	}

	bool LanguageResources::initializeNormalizationDictionary(path const& normDicPath)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		m_normalizationDictionaryPath = m_languageDirectoryPath / normDicPath;

		path virtualNormalizationDictionaryPath;
		if (!virtualizeFile(m_normalizationDictionaryPath, virtualNormalizationDictionaryPath)) return false;

		mapPath(m_normalizationDictionaryPath, virtualNormalizationDictionaryPath);
		return true;
	}

	bool LanguageResources::initializeAlphabet(boost::filesystem::path const& alphPath, bool isSorted)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		path diskPath;
		if (isSorted)
			diskPath = m_sortedAlphabetPath = m_languageDirectoryPath / alphPath;
		else
			diskPath = m_alphabetPath = m_languageDirectoryPath / alphPath;

		path virtualAlphabetPath;
		if (!virtualizeFile(diskPath, virtualAlphabetPath)) return false;

		path persistedPath;
		if (!persistAlphabet(virtualAlphabetPath, persistedPath)) {
#ifdef DEBUG_UIMA_CPP
			cerr << "Could not persist " << (isSorted ? "sorted " : " ") << " alphabet " << virtualAlphabetPath << endl;
#endif
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Could not persist " << (isSorted ? "sorted " : " ") << " alphabet " << virtualAlphabetPath;
			ls.flush();
			return false;
		}
		mapPath(diskPath, persistedPath);

		return true;
	}

	bool LanguageResources::initializeSpecialAutomaton(path const& automatonPath, SpecialAutomatonType automatonType)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		path diskPath;
		string description;
		switch (automatonType) {
		case SENTENCE:
			diskPath = m_sentenceAutomatonPath = m_preprocessingDirectoryPath / "Sentence" / automatonPath;
			description = "Sentence automaton";
			break;
		case REPLACE:
			diskPath = m_replaceAutomatonPath = m_preprocessingDirectoryPath / "Replace" / automatonPath;
			description = "Replace automaton";
			break;
		default:
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Unknown special automaton type " << automatonType << "!";
			ls.flush();
			return false;
		}

		path persistedPath;
		if (!persistAutomaton(diskPath, persistedPath, true)) {
#ifdef DEBUG_UIMA_CPP
			cerr << "Could not persist " << description << " " << diskPath << endl;
#endif
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Could not virtualize " << description << " " << diskPath;
			ls.flush();
			return false;
		}

		mapPath(diskPath, persistedPath);
		return true;
	}

	LanguageResources::~LanguageResources()
	{
		if (--ms_livingInstances == 0)
			freePersistedResources();
	}

	///////////////////////////////////////////////////////////////////////////////
	//
	// Properties
	//
	///////////////////////////////////////////////////////////////////////////////

	/**
	* Gets a reference to the path of the directory containing the compiled resource
	* files for this language.
	*/
	const path& LanguageResources::getLanguageDirectoryPath() const
	{
		return m_languageDirectoryPath;
	}

	/**
	* Gets a reference to the path of the directory containing the compiled automata
	* files for this language.
	*/
	const path& LanguageResources::getAutomataDirectoryPath() const
	{
		return m_automataDirectoryPath;
	}

	/**
	* Gets a reference to the path of the directory containing the compiled
	* preprocessing automata files for this language.
	*/
	const path& LanguageResources::getPreprocessingDirectoryPath() const
	{
		return m_preprocessingDirectoryPath;
	}

	/**
	* Gets a reference to the path of the directory containing the compiled
	* dictionary files for this language.
	*/
	const path& LanguageResources::getDictionaryDirectoryPath() const
	{
		return m_dictionaryDirectoryPath;
	}

	/**
	* Gets a reference to the path of the normalization dictionary file
	* used by this language.
	*/
	const path& LanguageResources::getNormalizationDictionaryPath() const
	{
		return getActualPath(m_normalizationDictionaryPath);
	}

	/**
	* Gets a reference to the path of the alphabet file
	* used by this language.
	*/
	const path& LanguageResources::getAlphabetPath() const
	{
		return getActualPath(m_alphabetPath);
	}

	/**
	* Gets a reference to the path of the sorted alphabet file
	* used by this language.
	*/
	const path& LanguageResources::getSortedAlphabetPath() const
	{
		return getActualPath(m_sortedAlphabetPath);
	}

	/**
	* Gets a reference to the path of the sentence automaton file
	* used by this language.
	*/
	const path& LanguageResources::getSentenceAutomatonPath() const
	{
		return getActualPath(m_sentenceAutomatonPath);
	}

	/**
	* Gets a reference to the path of the replace automaton file
	* used by this language.
	*/
	const path& LanguageResources::getReplaceAutomatonPath() const
	{
		return getActualPath(m_replaceAutomatonPath);
	}

	/**
	* Gets a reference to the set of dictionary paths to be used
	* in the future application of Dico Unitex command.
	*/
	const set<path>& LanguageResources::getBinDictionariesPath() const
	{
		return m_dictionaryPaths;
	}

	/**
	* Gets a reference to te set of automaton paths to be used
	* in the future applications of Locate Unitex command.
	*/
	const set<path>& LanguageResources::getAutomataPaths() const
	{
		return m_automataPaths;
	}

	/**
	* Reset and fill the list of mandatory BIN dictionary names.
	*
	* The paths are expressed as relative paths under the desired Dela directory.
	* Their absolute path will be stored.
	*
	* @param dictionariesPaths
	* 			  a set of dictionary paths
	*/
	void LanguageResources::setBinaryDictionaries(const vector<UnicodeString>& dictionaries)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		BOOST_FOREACH(UnicodeString const& dictionary, dictionaries) {
			path binDicPath = getDictionaryDirectoryPath() / convertUnicodeStringToRawString(dictionary);
#ifdef DEBUG_UIMA_CPP
			cout << "Getting dictionary " << binDicPath << endl;
#endif
			if (annotator.isLoggingEnabled(LogStream::EnMessage)) {
				LogStream& ls = annotator.getLogStream(LogStream::EnMessage);
				ls << "Getting dictionary " << binDicPath << endl;
				ls.flush();
			}

			path persistedPath;
			if (!persistDictionary(binDicPath, persistedPath)) {
				UnitexAnnotatorCpp const& annotator = engine.getAnnotator();
				if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
					LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
					ls << "Cannot persist dictionary " << binDicPath;
					ls.flush();
				}
				continue;
			}

			mapPath(binDicPath, persistedPath);
		}

		m_bDictionariesAreSet = true;
	}

	/**
	* Gets the list of morphological dictionaries associated to an automaton, as a list of dictionaries
	* separated by semi-colons.
	*/
	const UnicodeString& LanguageResources::getMorphologicalDictionariesAsString(const path& automatonPath) const
	{
		path persistedAutomatonPath = virtualizedPath(automatonPath);
		map<path, UnicodeString>::const_iterator it = m_morphologicalDictionaryPaths.find(persistedAutomatonPath);
		if (it == m_morphologicalDictionaryPaths.end())
			return EmptyUnicodeString;
		return it->second;
	}

	/**
	* Stores the list of morphological dictionaries associated to an automaton in the provided string list.
	*/
	void LanguageResources::getMorphologicalDictionaries(const path& automatonPath, Stringlist& morphoDictList) const
	{
		morphoDictList.clear();
		const UnicodeString& ustrMorphoDicts = getMorphologicalDictionariesAsString(automatonPath);
		vector<UnicodeString> dictNames;
		splitRegex(dictNames, ustrMorphoDicts, UNICODE_STRING_SIMPLE(";"));
		BOOST_FOREACH(const UnicodeString& morphoDict, dictNames) {
			morphoDictList.push_back(convertUnicodeStringToRawString(morphoDict));
		}
	}

	/**
	* Associates a list of morphological dictionaries to be used when applying
	* Locate to a graph.
	*
	* The graph and dictionary paths are converted to file paths.
	*
	* @param morphoDictNames
	* 				a map associating to an automaton name (key) a string containing a list of morphological dictionaries (entry).
	*/
	void LanguageResources::setMorphologicalDictionaries(const map<UnicodeString, UnicodeString>& morphoDictNames)
	{
		m_morphologicalDictionaryPaths.clear();
		UnicodeString ustrAutomaton;
		UnicodeString ustrMorphoDicts;
		BOOST_FOREACH(boost::tie(ustrAutomaton, ustrMorphoDicts), morphoDictNames) {
			string automatonName = convertUnicodeStringToRawString(ustrAutomaton);
			path automatonPath = getAutomataDirectoryPath() / automatonName;
			automatonPath.make_preferred();
			automatonPath = virtualizedPath(automatonPath);

			if (m_automataPaths.find(automatonPath) == m_automataPaths.end()) {
				ostringstream oss;
				oss << "Setting morpho dictionaries for automaton: " << automatonPath << " is not a declared automaton";
				engine.getAnnotator().logWarning(oss.str());
				continue;
			}

			if (engine.getAnnotator().isLoggingEnabled(LogStream::EnMessage)) {
				ostringstream oss;
				oss << "Setting morpho dictionaries for automaton " << automatonName;
				engine.getAnnotator().logMessage(oss.str());
			}

			vector<UnicodeString> dictNames;
			list<UnicodeString> fullNames;
			splitRegex(dictNames, ustrMorphoDicts, UNICODE_STRING_SIMPLE(";"));
			BOOST_FOREACH(UnicodeString const& ustrDictName, dictNames) {
				path dictPath = getDictionaryDirectoryPath() / convertUnicodeStringToRawString(ustrDictName);
				path persistedPath;
				if (persistDictionary(dictPath, persistedPath, false)) {
					//path persistedDictPath = persistedPath(dictPath);
					fullNames.push_back(persistedPath.string().c_str());
				}
			}

			m_morphologicalDictionaryPaths[automatonPath] = join(fullNames, UNICODE_STRING_SIMPLE(";"));
		}
	}

	/**
	* Sets a new Sentence automaton, and persist it if needed.
	*/
	void LanguageResources::setSentenceAutomaton(path const& automatonPath)
	{
		m_automataPaths.erase(m_sentenceAutomatonPath);
		path langPath = engine.getUnitexSrcResourcesDir() / language.getNormalizedForm();
		if (isAbsolutePath(automatonPath))
			m_sentenceAutomatonPath = automatonPath;
		else
			m_sentenceAutomatonPath = langPath / "Graphs" / "Preprocessing" / "Sentence" / automatonPath;
		m_sentenceAutomatonPath.make_preferred();
#ifdef DEBUG_UIMA_CPP
		cout << "About to set Sentence automaton to " << m_sentenceAutomatonPath << endl;
#endif
		if (!isPersistedResourcePath(m_sentenceAutomatonPath)) {
			path persistedPath;
			persistAutomaton(m_sentenceAutomatonPath, persistedPath, true);
		}
	}

	/**
	* Loads automata into memory.
	*
	* The paths are expressed as relative paths under the desired Graphs directory.
	* Their absolute path will be stored.
	*
	* \param automataPaths
	* 			  a collection of automata paths (either .grf or .fst2) relative to the current language's
	* 			  graph directory
	* \param forceGraphCompilation
	* 			  if set, force recompilation of all graphs
	*/
	void LanguageResources::setAutomata(const vector<UnicodeString>& automata)
	{
		path graphPath = engine.getUnitexBinResourcesDir() / language.getNormalizedForm() / "Graphs";

		BOOST_FOREACH(UnicodeString const& automaton, automata) {
			path automatonPath = graphPath / convertUnicodeStringToRawString(automaton);

			path persistedPath;
			if (!persistAutomaton(automatonPath, persistedPath)) continue;

			m_automataPaths.insert(automatonPath);
			mapPath(automatonPath, persistedPath);
		}

		bAutomataOk = true;
	}

	/**
	* Gets the path for the source file of an automaton (i.e. the .grf).
	*/
	path LanguageResources::getSourceAutomatonPath(const path& automatonPath)
	{
		if (automatonPath.extension() == ".grf")
			return automatonPath;
		return change_extension(automatonPath, ".grf");
	}

	/**
	* Gets the path for the compiled file of an automaton (i.e. the .fst2).
	*/
	path LanguageResources::getCompiledAutomatonPath(const path& automatonPath)
	{
		if (automatonPath.extension() == ".fst2")
			return automatonPath;
		return change_extension(automatonPath, ".fst2");
	}

	/// <summary>
	/// Gets the path for source dictionary (i.e. the .dic).
	/// </summary>
	path LanguageResources::getSourceDictionaryPath(const path& dictionaryPath)
	{
		if (dictionaryPath.extension() == ".dic")
			return dictionaryPath;
		return change_extension(dictionaryPath, ".dic");
	}

	/// <summary>
	/// Gets the path for compiled dictionary (i.e. the .bin).
	/// </summary>
	path LanguageResources::getCompiledDictionaryPath(const path& dictionaryPath)
	{
		if (dictionaryPath.extension() == ".bin")
			return dictionaryPath;
		return change_extension(dictionaryPath, ".bin");
	}

	/// <summary>
	/// Tells if a source file for resource needs compilation.
	/// </summary>
	/// <param name='sourceFile'>The source file.</param>
	/// <param name='compiledFile'>The corresponding compiled file.</param>
	/// <remarks>
	/// A source file needs compilation if the compiled file does not exist, or if it was last touched before the source file.
	/// </remarks>
	bool LanguageResources::needsCompilation(const path& sourceFile, const path& compiledFile)
	{
		if (!exists(sourceFile))
			return false;
		if (!exists(compiledFile))
			return true;
		if (last_write_time(compiledFile) < last_write_time(sourceFile))
			return true;
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	//
	// File paths mapping
	//
	///////////////////////////////////////////////////////////////////////////////

	void LanguageResources::mapPath(path const& diskPath, path const& virtualPath) 
	{
		m_mapPaths[diskPath] = virtualPath;
	}

	/// <summary>
	/// Gets the actual (virtualized, persisted) path corresponding to a disk path.
	/// </summary>
	/// <param name='diskPath'>A file path on disk.</param>
	/// <returns>The virtual / persisted path corresponding to this disk path, or the disk path if none matches.</returns>
	path const& LanguageResources::getActualPath(path const& diskPath) const
	{
		map<path, path>::const_iterator it = m_mapPaths.find(diskPath);
		return (it == m_mapPaths.end()) ? diskPath : it->second;
	}

	path const& LanguageResources::operator[](path const& diskPath) const
	{
		return getActualPath(diskPath);
	}

	///////////////////////////////////////////////////////////////////////////////
	//
	// Resource virtualization
	//
	///////////////////////////////////////////////////////////////////////////////

	// Checks that a file exists and virtualizes it.
	// The store the virtual path into virtualPath.
	// Returns true if ok, false if error.
	bool LanguageResources::virtualizeFile(path const& diskPath, path& virtualPath)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		if (!exists(diskPath)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Disk file " << diskPath << " does not exist!";
			ls.flush();
			return false;
		}

		path persistedPath;
		virtualPath = virtualizedPath(diskPath);

		if (!copyUnitexFile(diskPath, virtualPath)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Could not virtualize disk file into " << virtualPath;
			ls.flush();
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	//
	// Persisted resources
	//
	///////////////////////////////////////////////////////////////////////////////

	/**
	* Tests whether a resource path is present in the set of already persisted resources.
	*/
	bool LanguageResources::isPersistedResourcePath(path const& aPath)
	{
		return ms_persistedResources.find(aPath) != ms_persistedResources.end();
	}

	bool LanguageResources::persistAutomaton(const path& automatonPath, path& persistedPath, bool sentenceGraph)
	{
		const UnitexAnnotatorCpp& annotator = engine.getAnnotator();
#ifdef DEBUG_UIMA_CPP
		cout << "Persisting automaton " << automatonPath << endl;
#endif
		if (annotator.isLoggingEnabled())
			annotator.logMessage("Persisting automaton %s", automatonPath.string().c_str());

		path sourcePath = getSourceAutomatonPath(automatonPath);
		path compiledPath = getCompiledAutomatonPath(automatonPath);

		if (annotator.forceGraphCompilation() || needsCompilation(sourcePath, compiledPath)) {
#ifdef DEBUG_UIMA_CPP
			cout << "Compiling graph " << sourcePath << " into " << compiledPath << endl;
#endif
			if (annotator.isLoggingEnabled(LogStream::EnMessage))
				annotator.logMessage("Compiling graph %s into %s", sourcePath.string().c_str(), compiledPath.string().c_str());
			if (!exists(unvirtualizedPath(sourcePath))) {
#ifdef DEBUG_UIMA_CPP
				cout << "Source automaton " << sourcePath << " does not exist!";
#endif
				if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
					LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
					ls << "Source automaton " << sourcePath << " file does not exist!";
					ls.flush();
				}
				return false;
			}

			if (!engine.getGraphCompiler().compile(sourcePath, sentenceGraph, compiledPath))
				return false;
		}

#ifdef DEBUG_UIMA_CPP
		cout << "Virtualizing FST2 " << automatonPath << endl;
#endif
		path virtualPath;
		if (!virtualizeFile(automatonPath, virtualPath)) return false;

#ifdef DEBUG_UIMA_CPP
		cout << "Loading persistent FST2 from " << virtualPath << endl;
#endif
		if (annotator.isLoggingEnabled(LogStream::EnMessage))
			annotator.logMessage("Loading persistent FST2 %s", automatonPath.string().c_str());
		char newPath[MAX_PATH];
		if (!unitex::standard_load_persistence_fst2(virtualPath.string().c_str(), newPath, MAX_PATH)) {
			// VFS
			// if (!unitex::load_persistent_fst2(automatonPath.string().c_str())) {
#ifdef DEBUG_UIMA_CPP
			cerr << "Error while persisting FST2 " << virtualPath << endl;
#endif
			annotator.logError("Error while persisting FST2 %s", virtualPath.string().c_str());
			return false;
		} else {
			// VFS
			// ms_persistedResources[automatonPath] = ResourceType::AUTOMATON;
			ms_persistedResources[newPath] = ResourceType::AUTOMATON;
			bool isPreprocessingGraph = false;
			for (path::iterator it = automatonPath.begin(); it != automatonPath.end(); it++) {
				string item = (*it).string();
				if (boost::iequals(item, "preprocessing")) {
					isPreprocessingGraph = true;
					break;
				}
			}
			persistedPath = newPath;
			//if (!isPreprocessingGraph)
			//	m_automataPaths.insert(persistedPath(automatonPath));
		}

		return true;
	}

	/**
	* Persists a dictionary using native Unitex persistence.
	*
	* @param dictionaryPath
	* 				the absolute path of the automaton to persist
	* @param addToDictionaries
	* 				if true (default) add to the list of dictionaries, otherwise do not add
	* @return
	* 				true if ok, false if a problem occurred.
	*/
	bool LanguageResources::persistDictionary(path const& dictionaryPath, path& persistedPath, bool addToDictionaries)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

#ifdef DEBUG_UIMA_CPP
		cout << "Persisting dictionary " << dictionaryPath << endl;
#endif
		if (annotator.isLoggingEnabled())
			annotator.logMessage("Persisting automaton %s", dictionaryPath.string().c_str());

		path sourcePath = getSourceDictionaryPath(dictionaryPath);
		path compiledPath = getCompiledDictionaryPath(dictionaryPath);

		if (annotator.forceGraphCompilation() || needsCompilation(sourcePath, compiledPath)) {
#ifdef DEBUG_UIMA_CPP
			cout << "Compiling dictionary " << sourcePath << " into " << compiledPath << endl;
#endif
			if (annotator.isLoggingEnabled(LogStream::EnMessage))
				annotator.logMessage("Compiling dictionary %s into %s", sourcePath.string().c_str(), compiledPath.string().c_str());
			if (!exists(unvirtualizedPath(sourcePath))) {
#ifdef DEBUG_UIMA_CPP
				cout << "Source dictionary " << sourcePath << " does not exist!";
#endif
				if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
					LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
					ls << "Source dictionary " << sourcePath << " file does not exist!";
					ls.flush();
				}
				return false;
			}

			if (!engine.getDictionaryCompiler().compile(sourcePath, getAlphabetPath()))
				return false;
		}

		// we are provided with the .BIN dictionary name
#ifdef DEBUG_UIMA_CPP
		cout << "Virtualizing BIN dictionary " << compiledPath << endl;
#endif
			path virtualBinDicPath;
			if (!virtualizeFile(compiledPath, virtualBinDicPath)) {
				if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
					LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
					ls << "Could not virtualize " << compiledPath << endl;
					ls.flush();
				}
				return false;
			}

			// we also store the corresponding .INF dictionary name
			path infDicPath = change_extension(compiledPath, ".inf");
#ifdef DEBUG_UIMA_CPP
		cout << "Virtualizing INF dictionary " << infDicPath << endl;
#endif
			path virtualInfDicPath;
			if (!virtualizeFile(infDicPath, virtualInfDicPath)) {
				if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
					LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
					ls << "Could not virtualize " << infDicPath << endl;
					ls.flush();
				}
				return false;
			}

#ifdef DEBUG_UIMA_CPP
		cout << "Loading persistent dictionary " << virtualBinDicPath << endl;
#endif
		if (annotator.isLoggingEnabled(LogStream::EnMessage)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnMessage);
			ls << "Loading persistent dictionary " << virtualBinDicPath << endl;
			ls.flush();
		}

		char newPath[MAX_PATH];
		if (!unitex::standard_load_persistence_dictionary(virtualBinDicPath.string().c_str(), newPath, MAX_PATH - 1)) {
#ifdef DEBUG_UIMA_CPP
			cerr << "Error while persisting dictionary " << virtualBinDicPath << endl;
#endif
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Error while persisting dictionary " << virtualBinDicPath << endl;
			ls.flush();
			return false;
		} 

		// if (addToDictionaries)
		//	m_dictionaryPaths.insert(persistedDictionaryPath);
		ms_persistedResources[newPath] = ResourceType::DICTIONARY;
		if (addToDictionaries)
			m_dictionaryPaths.insert(dictionaryPath);
		persistedPath = newPath;

		return true;
	}

	/**
	* Persists an alphabet using native Unitex persistence.
	*
	* @param automatonPath
	* 				the absolute path of the alphabet to persist
	* @return
	* 				true if ok, false if a problem occurred.
	*/
	bool LanguageResources::persistAlphabet(path const& alphabetPath, path& persistedPath)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		/*
		if (!exists(alphabetPath)) {
		#ifdef DEBUG_UIMA_CPP
		cout << "Loading alphabet " << alphabetPath << " file does not exist!";
		#endif
		if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
		LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
		ls << "Loading alphabet " << alphabetPath << " file does not exist!";
		ls.flush();
		}
		return false;
		}
		*/

		// If the path is not already in the set of persisted resources, persist it.
		/*
		path persistedAlphabetPath = persistedPath(alphabetPath);
		if (!isPersistedResourcePath(persistedAlphabetPath)) {
		ostringstream oss;
		oss << "Loading persistent alphabet " << alphabetPath;
		engine.getAnnotator().logMessage(oss.str());

		if (!unitex::load_persistent_alphabet(alphabetPath.string().c_str())) {
		ostringstream oss;
		oss << "Error while persisting alphabet " << alphabetPath;
		engine.getAnnotator().logError(oss.str());
		return false;
		} else {
		ms_persistedResources[persistedAlphabetPath] = ResourceType::ALPHABET;
		}
		}
		*/

#ifdef DEBUG_UIMA_CPP
		cout << "Loading persistent alphabet " << alphabetPath << endl;
#endif
		if (annotator.isLoggingEnabled(LogStream::EnMessage)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnMessage);
			ls << "Loading persistent alphabet " << alphabetPath << endl;
			ls.flush();
		}

		char newPath[MAX_PATH];
		if (!unitex::standard_load_persistence_alphabet(alphabetPath.string().c_str(), newPath, MAX_PATH - 1)) {
#ifdef DEBUG_UIMA_CPP
			cout << "Error while persisting alphabet " << alphabetPath << endl;
#endif
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Error while persisting alphabet " << alphabetPath << endl;
			ls.flush();
			return false;
		} else {
			ms_persistedResources[newPath] = ResourceType::ALPHABET;
			persistedPath = newPath;
		}

		return true;
	}

	/**
	* Free all persisted resources once all living instances of LanguageResources have been destroyed.
	*/
	void LanguageResources::freePersistedResources() 
	{
#ifdef DEBUG_MEMORY_LEAKS
		cout << "Free persisted language resources" << endl;
#endif
		path p;
		ResourceType rscType;
		BOOST_FOREACH(tie(p, rscType), ms_persistedResources) {
			switch (rscType) {
			case ALPHABET:
				freePersistedAlphabet(p);
				break;
			case AUTOMATON:
				freePersistedAutomaton(p);
				break;
			case DICTIONARY:
				freePersistedDictionary(p);
				break;
			}
		}
		ms_persistedResources.clear();

#ifdef DEBUG_MEMORY_LEAKS
		cout << "Persisted language resources are free." << endl;
#endif
	}

	void LanguageResources::freePersistedAlphabet(const path& alphabetPath) 
	{
#ifdef DEBUG_MEMORY_LEAKS
		cout << "Free persisted alphabet" << alphabetPath << endl;
#endif
		free_persistent_alphabet(alphabetPath.string().c_str());
	}

	void LanguageResources::freePersistedAutomaton(const path& automatonPath) 
	{
#ifdef DEBUG_MEMORY_LEAKS
		cout << "Free persisted automaton" << automatonPath << endl;
#endif
		free_persistent_fst2(automatonPath.string().c_str());
	}

	void LanguageResources::freePersistedDictionary(const path& dictionaryPath) 
	{
#ifdef DEBUG_MEMORY_LEAKS
		cout << "Free persisted dictionary" << dictionaryPath << endl;
#endif
		free_persistent_dictionary(dictionaryPath.string().c_str());
	}

	/**
	* Checks that all resources necessary to run a Unitex engine have been loaded.
	*
	* Raises a UnitexException if not.
	*
	* @throws UnitexException
	*/
	void LanguageResources::check()
	{
		if (!m_bDictionariesAreSet) {
			ostringstream oss;
			oss << "Mandatory BIN dictionaries have not been set for language " << getLanguage().getNormalizedForm();
			throw UnitexException(oss.str());
		}
		if (!bAutomataOk) {
			ostringstream oss;
			oss << "Automata have not been loaded for language " << getLanguage().getNormalizedForm();
			throw UnitexException(oss.str());
		}
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

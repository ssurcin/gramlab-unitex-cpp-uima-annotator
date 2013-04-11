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

///////////////////////////////////////////////////////////////////////////////
//
// Forward declaration of callbacks for AbstractDelaSpace plugin
//
///////////////////////////////////////////////////////////////////////////////

#ifdef MY_ABSTRACT_SPACES
extern "C" 
{
	int ABSTRACT_CALLBACK_UNITEX initDelaSpace(void* privateSpacePtr);
	void ABSTRACT_CALLBACK_UNITEX uninitDelaSpace(void* privateSpacePtr);

	int ABSTRACT_CALLBACK_UNITEX isFilenameDelaSpaceObject(const char* name, void* privateSpacePtr);

	struct unitex::INF_codes* ABSTRACT_CALLBACK_UNITEX loadAbstractInfFile(const VersatileEncodingConfig* vec, const char* name, struct INF_free_info* p_inf_free_info, void* privateSpacePtr);
	void ABSTRACT_CALLBACK_UNITEX freeAbstractInf(struct unitex::INF_codes* INF, struct INF_free_info* p_inf_free_info, void* privateSpacePtr);
	unsigned char* ABSTRACT_CALLBACK_UNITEX loadAbstractBinFile(const char* name, long* file_size, struct BIN_free_info* p_bin_free_info, void* privateSpacePtr);
	void ABSTRACT_CALLBACK_UNITEX freeAbstractBin(unsigned char* BIN, struct BIN_free_info* p_bin_free_info, void* privateSpacePtr);

	const t_persistent_dic_func_array persistentDictionaryHooks = {
		sizeof(t_persistent_dic_func_array),
		isFilenameDelaSpaceObject,
		initDelaSpace,
		uninitDelaSpace,
		loadAbstractInfFile,
		freeAbstractInf,
		loadAbstractBinFile,
		freeAbstractBin
	};
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Forward declaration of callbacks for AbstractFs2Space plugin
//
///////////////////////////////////////////////////////////////////////////////

#ifdef MY_ABSTRACT_SPACES
extern "C"
{
	int ABSTRACT_CALLBACK_UNITEX isFilenameFst2SpaceObject(const char* name, void* privateSpacePtr);

	int ABSTRACT_CALLBACK_UNITEX initFst2Space(void* privateSpacePtr);
	void ABSTRACT_CALLBACK_UNITEX uninitFst2Space(void* privateSpacePtr);

	Fst2* ABSTRACT_CALLBACK_UNITEX loadAbstractFst2(const VersatileEncodingConfig* vec,const char* name,int read_names,struct FST2_free_info* p_fst2_free_info,void* privateSpacePtr);
	void ABSTRACT_CALLBACK_UNITEX freeAbstractFst2(Fst2* fst2,struct FST2_free_info* p_inf_free_info,void* privateSpacePtr);

	const t_persistent_fst2_func_array persistentFst2Hooks = {
		sizeof(t_persistent_fst2_func_array),
		isFilenameFst2SpaceObject,
		initFst2Space,
		uninitFst2Space,
		loadAbstractFst2,
		freeAbstractFst2
	};
}
#endif

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
		engine(anEngine), language(aLanguage)
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

#ifdef MY_ABSTRACT_SPACES
#ifdef DEBUG_UIMA_CPP
		cout << "Creating new persistent space for DELA dictionaries" << endl;
#endif
		AddAbstractDelaSpace(&persistentDictionaryHooks, this);
#ifdef DEBUG_UIMA_CPP
		cout << "Creating new persistent space for FST2 automata" << endl;
#endif
		AddAbstractFst2Space(&persistentFst2Hooks, this);
#endif // MY_ABSTRACT_SPACES

		// normalization dictionary (no need to persist)
		m_normalizationDictionaryPath = m_languageDirectoryPath / normDicPath;
		if (!exists(m_normalizationDictionaryPath)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Normalization dictionary " << m_normalizationDictionaryPath << " does not exist!";
			ls.flush();
			return false;
		}

		// alphabet file (persisted)
		path alphabetPath = m_languageDirectoryPath / alphName;
		if (!exists(alphabetPath)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Alphabet " << alphabetPath << " does not exist!";
			ls.flush();
			return false;
		}
		if (!isPersistedResourcePath(alphabetPath))
			persistAlphabet(alphabetPath);
		m_alphabetPath = persistedPath(alphabetPath);

		// sorted alphabet file (persisted)
		path sortedAlphabetPath = m_languageDirectoryPath / alphSortName;
		if (!exists(sortedAlphabetPath)) {
			LogStream& ls = annotator.getLogStream(LogStream::EnError);
			ls << "Sorted alphabet " << sortedAlphabetPath << " does not exist!";
			ls.flush();
			return false;
		}
		if (!isPersistedResourcePath(sortedAlphabetPath))
			persistAlphabet(sortedAlphabetPath);
		m_sortedAlphabetPath = persistedPath(sortedAlphabetPath);

		// sentence formatting graph (persisted)
		path actualSentencePath = m_preprocessingDirectoryPath / "Sentence" / sentenceName;
		if (!isPersistedResourcePath(actualSentencePath))
			persistAutomaton(actualSentencePath, true);
		m_sentenceAutomatonPath = persistedPath(actualSentencePath);

		// sentence formatting graph (persisted)
		path actualReplacePath = m_preprocessingDirectoryPath / "Replace" / replaceName;
		if (!isPersistedResourcePath(actualReplacePath))
			persistAutomaton(actualReplacePath);
		m_replaceAutomatonPath = persistedPath(actualReplacePath);

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
		return m_normalizationDictionaryPath;
	}

	/**
	* Gets a reference to the path of the alphabet file
	* used by this language.
	*/
	const path& LanguageResources::getAlphabetPath() const
	{
		return m_alphabetPath;
	}

	/**
	* Gets a reference to the path of the sorted alphabet file
	* used by this language.
	*/
	const path& LanguageResources::getSortedAlphabetPath() const
	{
		return m_sortedAlphabetPath;
	}

	/**
	* Gets a reference to the path of the sentence automaton file
	* used by this language.
	*/
	const path& LanguageResources::getSentenceAutomatonPath() const
	{
		return m_sentenceAutomatonPath;
	}

	/**
	* Gets a reference to the path of the replace automaton file
	* used by this language.
	*/
	const path& LanguageResources::getReplaceAutomatonPath() const
	{
		return m_replaceAutomatonPath;
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
		for (vector<UnicodeString>::const_iterator it = dictionaries.begin(); it != dictionaries.end(); it++) {
			// we are provided with the .BIN dictionary name
			path binDicPath = getDictionaryDirectoryPath() / convertUnicodeStringToRawString(*it);
			//m_dictionaryPaths.insert(binDicPath);
			// we also store the corresponding .INF dictionary name
			//path infDicPath = change_extension(binDicPath, ".inf");
			if (!persistDictionary(binDicPath)) {
				UnitexAnnotatorCpp const& annotator = engine.getAnnotator();
				if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
					LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
					ls << "Cannot persist dictionary " << binDicPath;
					ls.flush();
				}
			}
		}
		m_bDictionariesAreSet = true;
	}

	/**
	* Gets the list of morphological dictionaries associated to an automaton, as a list of dictionaries
	* separated by semi-colons.
	*/
	const UnicodeString& LanguageResources::getMorphologicalDictionariesAsString(const path& automatonPath) const
	{
		path persistedAutomatonPath = persistedPath(automatonPath);
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
			automatonPath = persistedPath(automatonPath);

			if (m_automataPaths.find(automatonPath) == m_automataPaths.end()) {
				ostringstream oss;
				oss << "Setting morpho dictionaries for automaton: " << automatonPath << " is not a declared automaton";
				engine.getAnnotator().logWarning(oss.str());
				continue;
			}

			if (engine.getAnnotator().isLoggingEnabled(LogStream::EnEntryType::EnMessage)) {
				ostringstream oss;
				oss << "Setting morpho dictionaries for automaton " << automatonName;
				engine.getAnnotator().logMessage(oss.str());
			}

			vector<UnicodeString> dictNames;
			list<UnicodeString> fullNames;
			splitRegex(dictNames, ustrMorphoDicts, UNICODE_STRING_SIMPLE(";"));
			BOOST_FOREACH(UnicodeString const& ustrDictName, dictNames) {
				path dictPath = getDictionaryDirectoryPath() / convertUnicodeStringToRawString(ustrDictName);
				if (persistDictionary(dictPath, false)) {
					path persistedDictPath = persistedPath(dictPath);
					fullNames.push_back(persistedDictPath.string().c_str());
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
		if (!isPersistedResourcePath(m_sentenceAutomatonPath))
			persistAutomaton(m_sentenceAutomatonPath, true);
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
			if (!isPersistedResourcePath(automatonPath)) {
				if (!persistAutomaton(automatonPath))
					continue;
			}
			m_automataPaths.insert(persistedPath(automatonPath));
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

	bool LanguageResources::needsCompilation(const path& sourceFile, const path& compiledFile)
	{
		if (!exists(sourceFile))
			return false;
		if (!exists(compiledFile))
			return true;
		if (last_write_time(compiledFile) > last_write_time(sourceFile))
			return true;
		return false;
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

	bool LanguageResources::persistAutomaton(const path& automatonPath, bool sentenceGraph)
	{
		const UnitexAnnotatorCpp& annotator = engine.getAnnotator();
		if (annotator.isLoggingEnabled())
			annotator.logMessage("Persisting automaton %s", automatonPath.string().c_str());

		path sourcePath = getSourceAutomatonPath(automatonPath);
		path compiledPath = getCompiledAutomatonPath(automatonPath);

		if (engine.getAnnotator().forceGraphCompilation() || needsCompilation(sourcePath, compiledPath)) {
			if (annotator.isLoggingEnabled(LogStream::EnMessage))
				annotator.logMessage("Compiling graph %s into %s", sourcePath.string().c_str(), compiledPath.string().c_str());
			if (!exists(unpersistedPath(sourcePath))) {
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

		// If the automaton path is not already in the set of persisted resources, persist it.
		if (!isPersistedResourcePath(automatonPath)) {
			if (annotator.isLoggingEnabled(LogStream::EnMessage))
				annotator.logMessage("Loading persistent FST2 %s", automatonPath.string().c_str());

#ifdef MY_ABSTRACT_SPACES
			path persistedFst2Path = persistedPath(automatonPath);
			const VersatileEncodingConfig vec = VEC_DEFAULT;
			Fst2* fst2 = load_fst2(&vec, persistedFst2Path.string().c_str(), 1);
			m_persistedFst2Automata[persistedFst2Path] = new_Fst2_clone(fst2);
#endif
			if (!unitex::load_persistent_fst2(automatonPath.string().c_str())) {
				annotator.logError("Error while persisting FST2 %s", automatonPath.string().c_str());
				return false;
			} else {
				ms_persistedResources[automatonPath] = ResourceType::AUTOMATON;
				bool isPreprocessingGraph = false;
				for (path::iterator it = automatonPath.begin(); it != automatonPath.end(); it++) {
					string item = (*it).string();
					if (boost::iequals(item, "preprocessing")) {
						isPreprocessingGraph = true;
						break;
					}
				}
				//if (!isPreprocessingGraph)
				//	m_automataPaths.insert(persistedPath(automatonPath));
			}
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
	bool LanguageResources::persistDictionary(path const& dictionaryPath, bool addToDictionaries)
	{
		UnitexAnnotatorCpp const& unitexAnnotator = engine.getAnnotator();

		if (!exists(dictionaryPath)) {
			if (unitexAnnotator.isLoggingEnabled(LogStream::EnWarning)) {
				LogStream& ls = unitexAnnotator.getLogStream(LogStream::EnWarning);
				ls << "Loading dictionary " << dictionaryPath << " file does not exist!";
				ls.flush();
			}
			return false;
		}

		// If the dictionary path is not already in the set of persisted resources, persist it.
		if (!isPersistedResourcePath(dictionaryPath)) {
			if (unitexAnnotator.isLoggingEnabled(LogStream::EnMessage)) {
				LogStream& ls = unitexAnnotator.getLogStream(LogStream::EnMessage);
				ls << "Loading persistent dictionary " << dictionaryPath;
				ls.flush();
			}

			path persistedDictionaryPath = persistedPath(dictionaryPath);
#ifdef MY_ABSTRACT_SPACES
			// Preload the BIN dictionary and store it in cache
			const VersatileEncodingConfig vec = VEC_DEFAULT;
			long binSize = 0L;
			BIN_free_info binFreeInfo;
			const unsigned char* binDictionary = load_abstract_BIN_file(dictionaryPath.string().c_str(), &binSize, &binFreeInfo);
			m_persistedBinDictionaries[persistedDictionaryPath] = binDictionary;

			// Preload the INF dictionary and store it in cache
			path persistedInfPath = change_extension(persistedDictionaryPath, ".inf");
			struct unitex::INF_codes* infCodes = load_INF_file(&vec, persistedInfPath.string().c_str());
			m_persistedInfCodes[persistedInfPath] = infCodes;
#endif
			if (!unitex::load_persistent_dictionary(persistedDictionaryPath.string().c_str())) {
				ostringstream oss;
				oss << "Error while persisting dictionary " << dictionaryPath;
				engine.getAnnotator().logError(oss.str());
				return false;
			} else {
				ms_persistedResources[dictionaryPath] = ResourceType::DICTIONARY;
				if (addToDictionaries)
					m_dictionaryPaths.insert(persistedDictionaryPath);
			}
		}

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
	bool LanguageResources::persistAlphabet(path const& alphabetPath)
	{
		UnitexAnnotatorCpp const& annotator = engine.getAnnotator();

		if (!exists(alphabetPath)) {
			if (annotator.isLoggingEnabled(LogStream::EnWarning)) {
				LogStream& ls = annotator.getLogStream(LogStream::EnWarning);
				ls << "Loading alphabet " << alphabetPath << " file does not exist!";
				ls.flush();
			}
			return false;
		}

		// If the path is not already in the set of persisted resources, persist it.
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
			case ResourceType::ALPHABET:
				freePersistedAlphabet(p);
				break;
			case ResourceType::AUTOMATON:
				freePersistedAutomaton(p);
				break;
			case ResourceType::DICTIONARY:
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

	///////////////////////////////////////////////////////////////////////////////
	//
	// Helper methods for interface AbstractDelaSpace
	//
	///////////////////////////////////////////////////////////////////////////////

#ifdef MY_ABSTRACT_SPACES
	bool LanguageResources::initDelaSpace()
	{
		m_persistedInfCodes.clear();
		m_persistedBinDictionaries.clear();
		m_persistedFst2Automata.clear();
		return true;
	}

	void LanguageResources::uninitDelaSpace()
	{
		m_persistedInfCodes.clear();
		m_persistedBinDictionaries.clear();
		m_persistedFst2Automata.clear();
	}

	bool LanguageResources::isPersistedInfCodes(const path& path) const
	{
		return m_persistedInfCodes.find(path) != m_persistedInfCodes.end();
	}

	bool LanguageResources::isPersistedBinDictionary(const path& path) const
	{
		return m_persistedBinDictionaries.find(path) != m_persistedBinDictionaries.end();
	}

	bool LanguageResources::isPersistedFst2Automaton(const path& path) const
	{
		return m_persistedFst2Automata.find(path) != m_persistedFst2Automata.end();
	}

	bool LanguageResources::isPersistedDelaOrFst2(const string& name) const
	{
		path objectPath = persistedPath(name);
		string extension = objectPath.extension().string();
		boost::to_lower(extension);
		if (extension == ".inf")
			return isPersistedInfCodes(objectPath);
		else if (extension == ".bin")
			return isPersistedBinDictionary(objectPath);
		else if (extension == ".fst2")
			return isPersistedFst2Automaton(objectPath);
		return false;
	}

	struct unitex::INF_codes* LanguageResources::getPersistedInfCodes(const path& infPath) const
	{
		struct unitex::INF_codes* result = NULL;
		PersistedInfCodesMap::const_iterator it = m_persistedInfCodes.find(infPath);
		if (it != m_persistedInfCodes.end())
			result = it->second;
		return result;
	}

	const unsigned char* LanguageResources::getPersistedBinDictionary(const boost::filesystem::path& binPath) const
	{
		const unsigned char* result = NULL;
		PersistedBinDictionariesMap::const_iterator it = m_persistedBinDictionaries.find(binPath);
		if (it != m_persistedBinDictionaries.end())
			result = it->second;
		return result;
	}

	bool LanguageResources::initFst2Space()
	{
		m_persistedFst2Automata.clear();
		return true;
	}

	void LanguageResources::uninitFst2Space()
	{
		m_persistedFst2Automata.clear();
	}

	Fst2* LanguageResources::getPersistedFst2(const path& fst2Path) const
	{
		Fst2* result = NULL;
		PersistedFst2Map::const_iterator it = m_persistedFst2Automata.find(fst2Path);
		if (it != m_persistedFst2Automata.end())
			result = new_Fst2_clone(it->second);
		return result;

	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation of callbacks for interface AbstractDelaSpace
//
///////////////////////////////////////////////////////////////////////////////

#ifdef MY_ABSTRACT_SPACES
extern "C" 
{
	/*!
	* Initializes the abstract Dela space.
	*/
	int ABSTRACT_CALLBACK_UNITEX initDelaSpace(void* privateSpacePtr)
	{
		int result = 0;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources)
			result = pLanguageResources->initDelaSpace() ? 1 : 0;
		return result;
	}

	/*!
	* Uninitializes the abstract Dela space.
	*/
	void ABSTRACT_CALLBACK_UNITEX uninitDelaSpace(void* privateSpacePtr)
	{
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources)
			pLanguageResources->uninitDelaSpace();
	}

	/*!
	* Checks if a filename is found in the abstract Dela space
	*/
	int ABSTRACT_CALLBACK_UNITEX isFilenameDelaSpaceObject(const char* name, void* privateSpacePtr)
	{
		int result = 0;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources) 
			result = pLanguageResources->isPersistedDelaOrFst2(name);
		return result;
	}

	struct unitex::INF_codes* ABSTRACT_CALLBACK_UNITEX loadAbstractInfFile(const VersatileEncodingConfig* vec, const char* name, struct INF_free_info* p_inf_free_info, void* privateSpacePtr)
	{
		struct unitex::INF_codes* result = NULL;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources) {
			path infPath = persistedPath(name);
			return pLanguageResources->getPersistedInfCodes(infPath);
		}
		return result;
	}

	void ABSTRACT_CALLBACK_UNITEX freeAbstractInf(struct unitex::INF_codes* INF, struct INF_free_info* p_inf_free_info, void* privateSpacePtr)
	{
		// Do nothing
	}

	unsigned char* ABSTRACT_CALLBACK_UNITEX loadAbstractBinFile(const char* name, long* file_size, struct BIN_free_info* p_bin_free_info, void* privateSpacePtr)
	{
		unsigned char* result = NULL;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources) {
			path infPath = persistedPath(name);
			return (unsigned char*)(pLanguageResources->getPersistedBinDictionary(infPath));
		}
		return result;
	}

	void ABSTRACT_CALLBACK_UNITEX freeAbstractBin(unsigned char* BIN, struct BIN_free_info* p_bin_free_info, void* privateSpacePtr)
	{
		// Do nothing
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Implementation of callbacks for interface AbstractFst2Space
//
///////////////////////////////////////////////////////////////////////////////

#ifdef MY_ABSTRACT_SPACES
extern "C" 
{
	/*!
	* Initializes the abstract FST2 space.
	*/
	int ABSTRACT_CALLBACK_UNITEX initFst2Space(void* privateSpacePtr)
	{
		int result = 0;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources)
			result = pLanguageResources->initFst2Space() ? 1 : 0;
		return result;
	}

	/*!
	* Uninitializes the abstract FST2 space.
	*/
	void ABSTRACT_CALLBACK_UNITEX uninitFst2Space(void* privateSpacePtr)
	{
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources)
			pLanguageResources->uninitFst2Space();
	}

	/*!
	* Checks if a filename is found in the abstract FST2 space
	*/
	int ABSTRACT_CALLBACK_UNITEX isFilenameFst2SpaceObject(const char* name, void* privateSpacePtr)
	{
		int result = 0;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources) 
			result = pLanguageResources->isPersistedDelaOrFst2(name);
		return result;
	}

	Fst2* ABSTRACT_CALLBACK_UNITEX loadAbstractFst2(const VersatileEncodingConfig* vec,const char* name,int read_names,struct FST2_free_info* p_fst2_free_info,void* privateSpacePtr)
	{
		Fst2* result = NULL;
		unitexcpp::LanguageResources* pLanguageResources = (unitexcpp::LanguageResources*)privateSpacePtr;
		if (pLanguageResources) {
			path infPath = persistedPath(name);
			return pLanguageResources->getPersistedFst2(infPath);
		}
		return result;
	}

	void ABSTRACT_CALLBACK_UNITEX freeAbstractFst2(Fst2* fst2,struct FST2_free_info* p_inf_free_info,void* privateSpacePtr)
	{
		// Do nothing
	}
}
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

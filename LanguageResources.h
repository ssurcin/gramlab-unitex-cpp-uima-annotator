/*
 * LanguageResources.h
 *
 *  Created on: 28 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef LANGUAGERESOURCES_H_
#define LANGUAGERESOURCES_H_

#include "Utils.h"
#include "uima/api.hpp"
#include <boost/filesystem.hpp>
#include <set>
#include "Unitex-C++/LoadInf.h"
#include "Unitex-C++/Fst2.h"

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	class Language;

	class LanguageResources
	{
	private:
		engine::UnitexEngine& engine;
		Language const& language;

		boost::filesystem::path m_languageDirectoryPath;
		boost::filesystem::path m_automataDirectoryPath;
		boost::filesystem::path m_preprocessingDirectoryPath;
		boost::filesystem::path m_dictionaryDirectoryPath;

		std::set<boost::filesystem::path> m_dictionaryPaths; // the mandatory .BIN dictionaries (the order is not important)
		std::map<boost::filesystem::path, icu::UnicodeString> m_morphologicalDictionaryPaths; // the morphological dictionaries associated to automata
		std::set<boost::filesystem::path> m_automataPaths; // list of automata for this language (the order is not important)
		boost::filesystem::path m_normalizationDictionaryPath; // the normalization dictionary
		boost::filesystem::path m_alphabetPath; // the alphabet file
		boost::filesystem::path m_sortedAlphabetPath; // the sorted alphabet file
		boost::filesystem::path m_sentenceAutomatonPath; // the sentence formatting automaton
		boost::filesystem::path m_replaceAutomatonPath; // the normalization replacement automaton

		bool m_bDictionariesAreSet; // flag set when the BIN dictionaries have been set
		bool bAutomataOk; // flag set when the automata have been loaded

		/**
		 * The set of persisted resources (using Unitex native persistence), shared among all the Language instances so that we do not persist a resource twice.
		 */
		static std::set<boost::filesystem::path> ms_persistedResources;

	public:
		LanguageResources(engine::UnitexEngine& anEngine, Language const& aLanguage);
		virtual ~LanguageResources();

		bool initialize(boost::filesystem::path const& normDicPath, std::string const& sentenceName, std::string const& replaceName, std::string const& alphName, std::string const& alphSortName);

		Language const& getLanguage() const
		{
			return language;
		}

		boost::filesystem::path const& getLanguageDirectoryPath() const;
		boost::filesystem::path const& getAutomataDirectoryPath() const;
		boost::filesystem::path const& getPreprocessingDirectoryPath() const;
		boost::filesystem::path const& getDictionaryDirectoryPath() const;

		boost::filesystem::path const& getNormalizationDictionaryPath() const;
		boost::filesystem::path const& getAlphabetPath() const;
		boost::filesystem::path const& getSortedAlphabetPath() const;
		boost::filesystem::path const& getSentenceAutomatonPath() const;
		boost::filesystem::path const& getReplaceAutomatonPath() const;
		const std::set<boost::filesystem::path>& getBinDictionariesPath() const;
		const std::set<boost::filesystem::path>& getAutomataPaths() const;

		void setBinaryDictionaries(const std::vector<icu::UnicodeString>& dictionaries);
		void setMorphologicalDictionaries(const std::map<icu::UnicodeString, icu::UnicodeString>& morphoDictNames);
		void setSentenceAutomaton(boost::filesystem::path const& automatonPath);
		void setAutomata(const std::vector<icu::UnicodeString>& automata);
		bool persistAutomaton(boost::filesystem::path const& automatonPath, bool sentenceGraph =false);
		bool persistDictionary(boost::filesystem::path const& dictionaryPath, bool addToDictionaries =true);
		bool persistAlphabet(boost::filesystem::path const& alphabetPath);
		void check();

		static boost::filesystem::path getSourceAutomatonPath(const boost::filesystem::path& automatonPath);
		static boost::filesystem::path getCompiledAutomatonPath(const boost::filesystem::path& automatonPath);
		static bool needsCompilation(const boost::filesystem::path& sourceFile, const boost::filesystem::path& compiledFile);

	private:
		static bool isPersistedResourcePath(boost::filesystem::path const& aPath);

#ifdef MY_ABSTRACT_SPACES
		// Implementation of callbacks for interface AbstractDelaSpace
	private:
		typedef std::map<boost::filesystem::path, struct unitex::INF_codes*> PersistedInfCodesMap;
		PersistedInfCodesMap m_persistedInfCodes;
		typedef std::map<boost::filesystem::path, const unsigned char*> PersistedBinDictionariesMap;
		PersistedBinDictionariesMap m_persistedBinDictionaries;
		typedef std::map<boost::filesystem::path, Fst2*> PersistedFst2Map;
		PersistedFst2Map m_persistedFst2Automata;
	public:
		bool initDelaSpace();
		void uninitDelaSpace();
		bool isPersistedInfCodes(const boost::filesystem::path& path) const;
		bool isPersistedBinDictionary(const boost::filesystem::path& path) const;
		bool isPersistedFst2Automaton(const boost::filesystem::path& path) const;
		bool isPersistedDelaOrFst2(const std::string& filename) const;
		struct unitex::INF_codes* getPersistedInfCodes(const boost::filesystem::path& path) const;
		const unsigned char* getPersistedBinDictionary(const boost::filesystem::path& path) const;
		bool initFst2Space();
		void uninitFst2Space();
		unitex::Fst2* getPersistedFst2(const boost::filesystem::path& fst2Path) const;
#endif
	};

}

#endif /* LANGUAGERESOURCES_H_ */

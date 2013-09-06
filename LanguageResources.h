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

		// Maps regular (disk) file paths to the actual (virtual, persisted) file paths
		std::map<boost::filesystem::path, boost::filesystem::path> m_mapPaths;

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

		typedef enum { ALPHABET, DICTIONARY, AUTOMATON } ResourceType;
		typedef std::map<boost::filesystem::path, ResourceType> PersistedResourceCollection;
		/**
		* The persisted resources (using Unitex native persistence), shared among all the Language instances so that we do not persist a resource twice.
		*/
		static PersistedResourceCollection ms_persistedResources;
		static std::size_t ms_livingInstances;

	public:
		LanguageResources(engine::UnitexEngine& anEngine, Language const& aLanguage);
		virtual ~LanguageResources();

		bool initialize(boost::filesystem::path const& normDicPath, std::string const& sentenceName, std::string const& replaceName, std::string const& alphName, std::string const& alphSortName);
	private:
		typedef enum { SENTENCE, REPLACE } SpecialAutomatonType;
		bool initializeNormalizationDictionary(boost::filesystem::path const& normDicPath);
		bool initializeAlphabet(boost::filesystem::path const& alphPath, bool isSorted);
		bool initializeSpecialAutomaton(boost::filesystem::path const& automatonPath, SpecialAutomatonType automatonType);

	public:
		Language const& getLanguage() const
		{
			return language;
		}

		// Mapping disk paths to virtual paths
		void mapPath(boost::filesystem::path const& diskPath, boost::filesystem::path const& virtualPath);
		boost::filesystem::path const& getActualPath(boost::filesystem::path const& diskPath) const;
		boost::filesystem::path const& operator[](boost::filesystem::path const& diskPath) const;

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
		const icu::UnicodeString& getMorphologicalDictionariesAsString(const boost::filesystem::path& automatonPath) const;
		void getMorphologicalDictionaries(const boost::filesystem::path& automatonPath, unitexcpp::Stringlist& morphoDictList) const;
		const std::set<boost::filesystem::path>& getAutomataPaths() const;

		void setBinaryDictionaries(const std::vector<icu::UnicodeString>& dictionaries);
		void setMorphologicalDictionaries(const std::map<icu::UnicodeString, icu::UnicodeString>& morphoDictNames);
		void setSentenceAutomaton(boost::filesystem::path const& automatonPath);
		void setAutomata(const std::vector<icu::UnicodeString>& automata);

		void check();

		static boost::filesystem::path getSourceAutomatonPath(const boost::filesystem::path& automatonPath);
		static boost::filesystem::path getCompiledAutomatonPath(const boost::filesystem::path& automatonPath);
		static bool needsCompilation(const boost::filesystem::path& sourceFile, const boost::filesystem::path& compiledFile);

		bool virtualizeFile(boost::filesystem::path const& diskPath, boost::filesystem::path& virtualPath);

		bool persistAutomaton(boost::filesystem::path const& automatonPath, boost::filesystem::path& persistedPath, bool sentenceGraph =false);
		bool persistDictionary(boost::filesystem::path const& dictionaryPath, boost::filesystem::path& persistedPath, bool addToDictionaries =true);
		bool persistAlphabet(boost::filesystem::path const& alphabetPath, boost::filesystem::path& persistedPath);

	private:
		static bool isPersistedResourcePath(boost::filesystem::path const& aPath);
		static void freePersistedResources();
		static void freePersistedAlphabet(boost::filesystem::path const& alphabetPath);
		static void freePersistedAutomaton(boost::filesystem::path const& automatonPath);
		static void freePersistedDictionary(boost::filesystem::path const& dictionaryPath);
	};

}

#endif /* LANGUAGERESOURCES_H_ */

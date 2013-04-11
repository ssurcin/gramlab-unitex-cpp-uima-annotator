/*
* UnitexEngine.h
*
*  Created on: 28 déc. 2010
*      Author: sylvainsurcin
*/

#ifndef UNITEXENGINE_H_
#define UNITEXENGINE_H_

#include <string>
#include <list>
#include <vector>
#include <boost/filesystem.hpp>

#include "uima/api.hpp"
using namespace uima;

#include "UnitexAnnotatorCpp.h"
#include "LanguageResources.h"
#include "JavaLikeEnum.h"
#include "DictionaryCompiler.h"
#include "GraphCompiler.h"
#include "TextPreprocessor.h"
#include "TextProcessor.h"
#include "QualifiedString.h"

namespace unitexcpp
{

	class Language;

	namespace engine
	{

		class UnitexEngine
		{
		private:
			UnitexAnnotatorCpp const& m_annotator;

			boost::filesystem::path m_unitexSrcResourcesDir;
			boost::filesystem::path m_unitexBinResourcesDir;

			Language const& m_language;
			LanguageResources m_languageResources;
			bool m_validResources;
			unitexcpp::Stringlist m_dynamicDictionaries;

			DictionaryCompiler m_dictionaryCompiler;
			GraphCompiler m_graphCompiler;
			TextPreprocessor m_textPreprocessor;
			TextProcessor m_textProcessor;

			std::string m_inputFilename;
			std::string m_sntFilename;

		public:
			/*!
			* Number of milliseconds spent in Unitex processes since the beginning of
			* the session.
			*/
			static long ms_unitexRuntime;

			/*!
			* Number of milliseconds spent in Virtual File System processes since the
			* beginning of the session.
			*/
			static long ms_vfsRuntime;

			/*!
			* Flag set if the Unitex library is allowed to write to the standard output.
			*/
			static bool ms_allowedToStdout;

			/*!
			* Flag set if the abstract Virtual File System is initialized
			*/
			static bool ms_isVfsInitialized;

		public:
			UnitexEngine(	uima::UnitexAnnotatorCpp const& annotator,
				icu::UnicodeString const& normalizedLanguage,
				const boost::filesystem::path& srcResourcesDir,
				const std::vector<icu::UnicodeString>& dictionaries,
				const std::vector<icu::UnicodeString>& automata,
				const std::map<icu::UnicodeString, icu::UnicodeString>& morphoDictNames);
			UnitexEngine(	uima::UnitexAnnotatorCpp const& annotator,
				icu::UnicodeString const& normalizedLanguage,
				const boost::filesystem::path& srcResourcesDir,
				const boost::filesystem::path& binResourcesDir,
				const std::vector<icu::UnicodeString>& dictionaries,
				const std::vector<icu::UnicodeString>& automata,
				const std::map<icu::UnicodeString, icu::UnicodeString>& morphoDictNames);
			virtual ~UnitexEngine();

			std::list<QualifiedString> run(const std::string& inputFile = "");
		private:
			void checkDoubleAnalysis(const std::string& automaton, const std::list<QualifiedString>& automatonOutput);

		public:
			UnitexAnnotatorCpp const& getAnnotator() const
			{
				return m_annotator;
			}
			TextProcessor const& getTextProcessor() const
			{
				return m_textProcessor;
			}
			static void preventWritingToStdout();
			static std::size_t initializedVirtualFileSystem();

			// Language resources
			bool validResources() const;
			const boost::filesystem::path& getUnitexSrcResourcesDir() const;
			const boost::filesystem::path& getUnitexBinResourcesDir() const;

			boost::filesystem::path getGraphsDir() const;

			void setSentenceFstFile(const std::string& fileName);

			const std::string getAlphabetFile() const;
			const std::string getSortedAlphabetFile() const;
			const std::string getNormalizationDictionaryFile() const;
			const std::string getSentenceFstFile() const;
			const std::string getReplaceFstFile() const;
			unitexcpp::Stringlist::size_type getBinDictionaries(unitexcpp::Stringlist& list) const;
			const unitexcpp::Stringlist& getDynamicDictionaries() const;
			void getMorphologicalDictionaries(const std::string& automatonPath, unitexcpp::Stringlist& morphoDictList) const;
			boost::filesystem::path getSntDirectory() const;

			void clearDynamicDictionaries();
			bool addDynamicDictionary(std::string const& strDictName, DictionaryType const& dictionaryType = DictionaryType::DELAF);

			// Preprocessing
			static std::string buildSntFileNameFrom(const std::string& filename);
			static std::string buildSntDirNameFrom(const std::string& filename);
			void getNormalizedText(UnicodeString& normalizedText, const std::string& aFileName = "") const;
			void getInputText(UnicodeString& inputText, const std::string& aFileName = "") const;

			GraphCompiler& getGraphCompiler() { return m_graphCompiler; }

		public:
			void clearPerformanceCache();
			static void spendTimeInVFS(long ms);
			static void spendTimeInUnitex(long ms);
			static long getTimeSpentInVFS();
			static long getTimeSpentInUnitex();
		};

	}
}

#endif /* UNITEXENGINE_H_ */

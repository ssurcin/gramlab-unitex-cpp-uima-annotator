/*
* UnitexAnnotatorCpp.h
*
*  Created on: 30 déc. 2010
*      Author: sylvainsurcin
*/

#ifndef UNITEXANNOTATORCPP_H_
#define UNITEXANNOTATORCPP_H_

#include "uima/api.hpp"
#include <map>
#include <list>
#include <string>
#include "QualifiedString.h"
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include "Utils.h"

#if defined(_MSC_VER) && defined(DEBUG_MEMORY_LEAKS)
#include "MemoryLeaksDumper.h"
#endif

// Forward declarations
namespace unitexcpp
{
	class UnitexLogInstaller;

	namespace engine
	{
		class UnitexEngine;
	}

	namespace tokenize
	{
		class UnitexTokenizer;
	}

	namespace annotation
	{
		class LanguageArea;
		class UnitexDocumentParameters;
	}
}

namespace uima
{

	class UnitexAnnotatorCpp: public Annotator
	{
	public:
		static const icu::UnicodeString PARAM_UNITEXLOG;
		static const icu::UnicodeString PARAM_UNITEXAPP;
		static const icu::UnicodeString PARAM_DICTIONARIES;
		static const icu::UnicodeString PARAM_SYSTEMDICTIONARIES;
		static const icu::UnicodeString PARAM_USERDICTIONARIES;
		static const icu::UnicodeString PARAM_MORPHO_DICTIONARIES;
		static const icu::UnicodeString PARAM_GRAPHS;
		static const icu::UnicodeString PARAM_TIMEOUT_DELAY;
		static const icu::UnicodeString PARAM_FAKE_TOKENS;
		static const icu::UnicodeString PARAM_LONGEST_MATCH_OUTPUT;
		static const icu::UnicodeString PARAM_LOG_PROFILING_INFO;
		static const icu::UnicodeString PARAM_FORCE_GRAPH_COMPILATION;
		static const icu::UnicodeString PARAM_HIDE_UNITEX_OUTPUT;

	private:
		LogFacility* pLogger;
		bool m_logEnabled;
	public:
		LogFacility& getLogger() const
		{
			return *pLogger;
		}
		LogStream& getLogStream(LogStream::EnEntryType entryType) const 
		{
			return getLogger().getLogStream(entryType);
		}
		bool isLoggingEnabled() const
		{
			return m_logEnabled;
		}
		bool isLoggingEnabled(LogStream::EnEntryType entryType) const 
		{
			return isLoggingEnabled() && (getLoggingLevel() <= entryType);
		}
		LogStream::EnEntryType getLoggingLevel() const
		{
			return ResourceManager::getInstance().getLoggingLevel();
		}

	private:
		AnnotatorContext* m_pAnnotatorContext;
		unitexcpp::UnitexLogInstaller* pUnitexLogger;
	public:
		typedef std::pair<icu::UnicodeString, icu::UnicodeString> UnicodeStringPair;
		typedef std::map<UnitexAnnotatorCpp::UnicodeStringPair, unitexcpp::engine::UnitexEngine*> UnitexInstanceMap;
	private:
		UnitexInstanceMap unitexInstances;
		bool m_forceGraphCompilation;

		// Parameters
		int m_timeoutDelay;
		unitexcpp::UnicodeStringVector fakeTokens;
		unitexcpp::UnicodeStringVector fakeTokenReplacements;
		bool disambiguateLongestOutput;
		boost::filesystem::path m_pathUnitexResourcesDir;
		boost::filesystem::path m_pathUnitexAppDir;
		bool m_logProfilingInformation;

		boost::timer m_processTimer;

		bool typeSystemInitialized;

		Type tDynamicDictionary;
		Feature fEntries;

		CAS* pCurrentCAS;
		CAS* m_pWorkingView; // The actual working view

		std::size_t nbTokens;
		std::size_t m_nbTokensForOutputProcessing;
		std::size_t m_nbProcessedDocuments;

#if defined(_MSC_VER) && defined(DEBUG_MEMORY_LEAKS)
		unitexcpp::MemoryLeaksDumper m_memoryLeaksDumper;
#endif

	public:
		UnitexAnnotatorCpp(void);
		virtual ~UnitexAnnotatorCpp(void);

	public:
		TyErrorId initialize(AnnotatorContext& rclAnnotatorContext);
		const AnnotatorContext& getAnnotatorContext() const;
	private:
		TyErrorId initializeLogger();
		TyErrorId initializeHideUnitexOutput();
		TyErrorId initializeUnitexTimeout();
		TyErrorId initializeUnitexDisambiguateLongestOutput();
		TyErrorId initializeUnitexLogDirectory();
		TyErrorId initializeUnitexInstances();
		TyErrorId initializeUnitexFakeTokens();
		TyErrorId initializeUnitexResourcePath();
		TyErrorId initializeLogProfilingInformation();
		boost::filesystem::path getUnitexHomePath() const;
		boost::filesystem::path getUnitexResourcePath() const;
		TyErrorId getStrategiesInDescriptor(const icu::UnicodeString& language, std::set<icu::UnicodeString>& strategies) const;
		static std::string getStrategyFromGraphEntry(const std::string& graphEntry);
		void retrieveMasterGraphs(const boost::filesystem::path& rootDir, std::list<boost::filesystem::path>& masterGraphs);
		TyErrorId getMasterGraphsFromDescriptor(const icu::UnicodeString& language, const icu::UnicodeString& strategy, std::vector<icu::UnicodeString>& graphs);
		static icu::UnicodeString getStrategyFromGraphEntry(const icu::UnicodeString& entry);
		static icu::UnicodeString getGraphFromGraphEntry(const icu::UnicodeString& entry);
		TyErrorId getDictionariesFromDescriptor(const icu::UnicodeString& language, std::vector<icu::UnicodeString>& dictionaries);
		void readDictionariesDefinitionFile(const boost::filesystem::path& defFilePath, std::vector<icu::UnicodeString>& dictionaries);
		TyErrorId readMorphoDictionariesParameter(const icu::UnicodeString& language, std::map<icu::UnicodeString, icu::UnicodeString>& morphoDictionaries);
	public:
		TyErrorId typeSystemInit(TypeSystem const& crTypeSystem);

	public:
		TyErrorId destroy();

	public:
		TyErrorId process(CAS & tcas, ResultSpecification const& crResultSpecification);
	private:
		unitexcpp::engine::UnitexEngine& selectUnitexLanguageInstance(const icu::UnicodeString& language, const icu::UnicodeString& strategy);
		void getSupportedStrategies(std::set<UnicodeString>& strategies) const;
		boost::filesystem::path prepareInputFile(unitexcpp::engine::UnitexEngine& unitexEngine, const unitexcpp::annotation::LanguageArea& languageArea, const boost::filesystem::path& corpusPath) const;
		void prepareDynamicDictionary(unitexcpp::engine::UnitexEngine& unitexEngine);
		void processUnitexOutput(std::list<unitexcpp::engine::QualifiedString>& unitexResultMatches, const unitexcpp::annotation::LanguageArea& languageArea, unitexcpp::tokenize::UnitexTokenizer& tokenizer);
		void storeProfilingInformation();
		void storeAutomataProfilingInformation();

	public:
		void logMessage(const char* szFormat, ...) const;
		void logMessage(const std::string& format, ...) const;
		void logWarning(const char* szFormat, ...) const;
		void logWarning(const std::string& format, ...) const;
		void logError(const char* szFormat, ...) const;
		void logError(const std::string& format, ...) const;
	private:
		char* buildLogBuffer(const char* szFormat, va_list ap) const;

	private:
		void initializeWorkingView(uima::CAS& cas);
	public:
		CAS& getView() const;
		UnicodeStringRef getDocumentId() const;
		std::string getDocumentIdAsString() const;
		UnicodeStringRef getAnalysisStrategy() const;
		bool skip() const;
		bool forceGraphCompilation() const;
		const UnitexInstanceMap& getUnitexInstances() const;

	private:
		unitexcpp::annotation::UnitexDocumentParameters getUnitexDocumentParameters() const;
		AnnotationFS getDynamicDictionary() const;

	public:
		static bool equalsModuloUnitexSpecialCharacterReplacements(const icu::UnicodeString& s1, const icu::UnicodeString& s2);
	};

}
#endif /* UNITEXANNOTATORCPP_H_ */

//============================================================================
// Name        : UnitexAnnotatorCpp.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "UnitexAnnotatorCpp.h"
#include <uima/unistrref.hpp>
#include <uima/resmgr.hpp>
#include <unicode/ustring.h>
#include <unicode/ustream.h>
#include <unicode/ustdio.h>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#ifndef WIN32
#include <pthread.h>
#endif
#include <cstdlib>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/future.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include "Utils.h"
#include "FileUtils.h"
#include "UnitexLogInstaller.h"
#include "Language.h"
#include "UnitexEngine.h"
#include "UnitexException.h"
#include "UserCancellingPlugCallback.h"
#include "UnitexTokenizer.h"
#include "ProfilingLogger.h"
#include "MemoryLeaksDumper.h"

#include "LanguageArea.h"
#include "ContextAreaAnnotation.h"
#include "TokenAnnotation.h"
#include "SentenceAnnotation.h"
#include "ParagraphAnnotation.h"
#include "TransductionOutputAnnotation.h"
#include "AnnotatorPerformanceAnnotation.h"
#include "AutomatonLocatePerformanceAnnotation.h"
#include "UnitexDocumentParameters.h"
#include "VirtualFolderCleaner.h"

#include "Unitex-C++/VirtualFiles.h"
#include "Unitex-C++/UnitexLibIO.h"
#include "Unitex-C++/Persistence.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

using namespace std;
using namespace icu;
using namespace boost::filesystem;
using namespace boost::algorithm;
using namespace xercesc;
using namespace unitexcpp;
using namespace unitexcpp::engine;
using namespace unitexcpp::tokenize;
using namespace unitexcpp::annotation;

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace uima
{

	/////////////////////////////////////////////////////////////////////////
	//
	// Constants
	//
	/////////////////////////////////////////////////////////////////////////

	const UnicodeString UnitexAnnotatorCpp::PARAM_UNITEXLOG = UNICODE_STRING_SIMPLE("UnitexLog");
	const UnicodeString UnitexAnnotatorCpp::PARAM_UNITEXAPP = UNICODE_STRING_SIMPLE("UnitexLocalApp");
	const UnicodeString UnitexAnnotatorCpp::PARAM_DICTIONARIES = UNICODE_STRING_SIMPLE("Dictionaries");
	const UnicodeString UnitexAnnotatorCpp::PARAM_SYSTEMDICTIONARIES = UNICODE_STRING_SIMPLE("SystemDictionaries");
	const UnicodeString UnitexAnnotatorCpp::PARAM_USERDICTIONARIES = UNICODE_STRING_SIMPLE("UserDictionaries");
	const UnicodeString UnitexAnnotatorCpp::PARAM_MORPHO_DICTIONARIES = UNICODE_STRING_SIMPLE("MorphoDictionaries");
	const UnicodeString UnitexAnnotatorCpp::PARAM_GRAPHS = UNICODE_STRING_SIMPLE("Graphs");
	const UnicodeString UnitexAnnotatorCpp::PARAM_TIMEOUT_DELAY = UNICODE_STRING_SIMPLE("TimeoutDelay");
	const UnicodeString UnitexAnnotatorCpp::PARAM_FAKE_TOKENS = UNICODE_STRING_SIMPLE("fakeTokens");
	const UnicodeString UnitexAnnotatorCpp::PARAM_LONGEST_MATCH_OUTPUT = UNICODE_STRING_SIMPLE("LongestMatchOutput");
	const UnicodeString UnitexAnnotatorCpp::PARAM_LOG_PROFILING_INFO = UNICODE_STRING_SIMPLE("logProfilingInfo");
	const UnicodeString UnitexAnnotatorCpp::PARAM_FORCE_GRAPH_COMPILATION = UNICODE_STRING_SIMPLE("ForceGraphCompilation");
	const UnicodeString UnitexAnnotatorCpp::PARAM_HIDE_UNITEX_OUTPUT = "HideUnitexOutput";

	/////////////////////////////////////////////////////////////////////////
	//
	// Constructors
	//
	/////////////////////////////////////////////////////////////////////////

	UnitexAnnotatorCpp::UnitexAnnotatorCpp(void)
	{
		m_pAnnotatorContext = NULL;
		pUnitexLogger = NULL;
		typeSystemInitialized = false;
		m_nbProcessedDocuments = 0;
		XMLPlatformUtils::Initialize();
	}

	UnitexAnnotatorCpp::~UnitexAnnotatorCpp(void)
	{
		// Empty virtual file system
		unitex::virtualfile::VFS_reset();
		// Uninitialization
		unitex::dispose_persistence();
		unitex::virtualfile::dispose_virtual_files();
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// Initialization
	//
	/////////////////////////////////////////////////////////////////////////

	/**
	* Initialization entry point for the annotator.
	*/
	TyErrorId UnitexAnnotatorCpp::initialize(AnnotatorContext& rclAnnotatorContext)
	{
		m_pAnnotatorContext = &rclAnnotatorContext;
		initializeLogger();

		logMessage("Initializing UnitexAnnotatorCpp...");

		size_t nbVfsSpaces = UnitexEngine::initializedVirtualFileSystem();
		logMessage("Unitex Virtual File System initialized with %d virtual file spaces", nbVfsSpaces);

		// Prevent Unitex library to write into the standard output
		TyErrorId error = initializeHideUnitexOutput();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing hiding of Unitex output");
			return error;
		}

		error = initializeUnitexTimeout();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing Unitex timeout delay");
			return error;
		}

#ifdef DEBUG_UIMA_CPP
		cout << "Initializing Unitex resource paths..." << endl;
#endif
		error = initializeUnitexResourcePath();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing Unitex resource paths");
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex resource paths initialized" << endl;
#endif

#ifdef DEBUG_UIMA_CPP
		cout << "Initializing Unitex log directory..." << endl;
#endif
		error = initializeUnitexLogDirectory();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing Unitex log directory");
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex log directory initialized" << endl;
#endif

#ifdef DEBUG_UIMA_CPP
		cout << "Initializing Unitex longest output disambiguation..." << endl;
#endif
		error = initializeUnitexDisambiguateLongestOutput();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing Unitex longest output disambiguation mode");
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex longest output disambiguation initialized" << endl;
#endif

#ifdef DEBUG_UIMA_CPP
		cout << "Initializing Unitex instances..." << endl;
#endif
		error = initializeUnitexInstances();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing Unitex instances");
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex instances initialized" << endl;
#endif

#ifdef DEBUG_UIMA_CPP
		cout << "Initializing Unitex fake tokens..." << endl;
#endif
		error = initializeUnitexFakeTokens();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing Unitex fake tokens");
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex fake tokens initialized" << endl;
#endif

		error = initializeLogProfilingInformation();
		if (error != UIMA_ERR_NONE) {
			logError("Cannot initialize UnitexAnnotator because of error intializing log profiling information");
			return error;
		}

		logMessage("UnitexAnnotatorCpp initialized");

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/*!
	* Initializes the showing or hiding Unitex internal output.
	*/
	TyErrorId UnitexAnnotatorCpp::initializeHideUnitexOutput()
	{
		bool hideUnitexOutput = true;

		if (getAnnotatorContext().isParameterDefined(PARAM_HIDE_UNITEX_OUTPUT)) {
			if (getAnnotatorContext().extractValue(PARAM_HIDE_UNITEX_OUTPUT, hideUnitexOutput) != UIMA_ERR_NONE) {
				if (isLoggingEnabled()) {
					LogStream& ls = getLogStream(LogStream::EnError);
					ls << "Cannot extract value of configuration parameter " << PARAM_HIDE_UNITEX_OUTPUT << " in component descriptor";
					ls.flush();
				}
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}
		}

		if (hideUnitexOutput) {
			logMessage("Hiding Unitex output");
			UnitexEngine::preventWritingToStdout();
		} else
			logMessage("Keeping Unitex output");

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/*!
	* Initizalizes the logger reference and its logging level.
	* Currently parses manually the log4j.properties file (if found).
	*/
	TyErrorId UnitexAnnotatorCpp::initializeLogger()
	{
		pLogger = &(m_pAnnotatorContext->getLogger());

		bool logEnabled = true;
		LogStream::EnEntryType logLevel = LogStream::EnMessage;

		ifstream file("conf/log4j.properties", ios_base::in);
		if (file.good()) {
			string line;
			while (getline(file, line)) {
				boost::trim(line);
				if ((line.length() > 0) && !boost::starts_with(line, "#")) {
					if (boost::starts_with(line, "log4j.logger.org.apache.uima.uimacpp=") || boost::starts_with(line, "log4j.logger.org.apache.uima.uimacpp.UnitexAnnotatorCpp=")) {
						size_t sep = line.find("=");
						if (sep != string::npos) {
							string value = line.substr(sep + 1);
							boost::trim(value);
							if (value == "INFO") {
								logLevel = LogStream::EnMessage;
								break;
							}
							else if (value == "WARNING") {
								logLevel = LogStream::EnWarning;
								break;
							}
							else if (value == "ERROR") {
								logLevel = LogStream::EnError;
								break;
							}
							else {
								logEnabled = false;
								break;
							}
						}
					}
				}
			}
		}

#ifdef DEBUG_UIMA_CPP
		if (logEnabled) {
			cout << "Enabling logging" << endl;
			cout << "Logging level = " << logLevel << endl;
		}
		else
			cout << "Logging not enabled" << endl;
#endif
		m_logEnabled = logEnabled;
		if (m_logEnabled)
			ResourceManager::getInstance().setLoggingLevel(logLevel);

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Gets a constant reference to the annotator's UIMA initialization context
	* (to be used to retrieve parameters).
	*/
	const AnnotatorContext& UnitexAnnotatorCpp::getAnnotatorContext() const
	{
		return *m_pAnnotatorContext;
	}

	/**
	* Initialization subroutine for Unitex engine's timeout delay
	*/
	TyErrorId UnitexAnnotatorCpp::initializeUnitexTimeout()
	{
		m_timeoutDelay = 0;

		if (getAnnotatorContext().isParameterDefined(PARAM_TIMEOUT_DELAY)) {
			if (getAnnotatorContext().extractValue(PARAM_TIMEOUT_DELAY, m_timeoutDelay) != UIMA_ERR_NONE) {
				if (isLoggingEnabled()) {
					LogStream& ls = getLogStream(LogStream::EnError);
					ls << "Cannot extract value of configuration parameter " << PARAM_TIMEOUT_DELAY << " in component descriptor";
					ls.flush();
				}
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}
		} else {
			if (isLoggingEnabled()) {
				LogStream& ls = getLogStream(LogStream::EnWarning);
				ls << "Parameter " << PARAM_TIMEOUT_DELAY << " is not defined for Unitex process!";
				ls.flush();
			}
		}

#ifdef DEBUG_UIMA_CPP
		cout << "Unitex timeout parameter = " << m_timeoutDelay << "ms" << endl;
#endif
		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Initializes the parameter telling if we must log profiling information.
	*/
	TyErrorId UnitexAnnotatorCpp::initializeLogProfilingInformation()
	{
		m_logProfilingInformation = false;

		if (getAnnotatorContext().isParameterDefined(PARAM_LOG_PROFILING_INFO)) {
			if (getAnnotatorContext().extractValue(PARAM_LOG_PROFILING_INFO, m_logProfilingInformation) != UIMA_ERR_NONE) {
				if (isLoggingEnabled()) {
					LogStream& ls = getLogStream(LogStream::EnError);
					ls << "Cannot extract value of configuration parameter " << PARAM_LOG_PROFILING_INFO << " in component descriptor";
					ls.flush();
				}
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}
		} 

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Initializes the parameter indicating whether Unitex shall disambiguate the matches based on the longest output.
	*/
	TyErrorId UnitexAnnotatorCpp::initializeUnitexDisambiguateLongestOutput()
	{
		disambiguateLongestOutput = false;

		if (getAnnotatorContext().isParameterDefined(PARAM_LONGEST_MATCH_OUTPUT)) {
			if (getAnnotatorContext().extractValue(PARAM_LONGEST_MATCH_OUTPUT, disambiguateLongestOutput) != UIMA_ERR_NONE) {
				if (isLoggingEnabled()) {
					LogStream& ls = getLogStream(LogStream::EnError);
					ls << "Cannot extract value of configuration parameter " << PARAM_LONGEST_MATCH_OUTPUT << " in component descriptor";
					ls.flush();
				}
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}
		}

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Initialization subroutine to install (or not) a logger for UnitexTool activity.
	*/
	TyErrorId UnitexAnnotatorCpp::initializeUnitexLogDirectory()
	{
		// Get desired path from UimaAnnotator parameter
		if (getAnnotatorContext().isParameterDefined(PARAM_UNITEXLOG)) {
			string strLogPath = "";
			if (getAnnotatorContext().extractValue(PARAM_UNITEXLOG, strLogPath) != UIMA_ERR_NONE) {
				if (isLoggingEnabled()) {
					LogStream& ls = getLogStream(LogStream::EnError);
					ls << "Cannot extract value of configuration parameter " << PARAM_UNITEXLOG << " in component descriptor";
					ls.flush();
				}
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}

			if (!iequals(strLogPath, "no")) {
				logMessage("No Unitex logging");
				return UIMA_ERR_NONE;
			}

			// Check if the directory exists
			path logPath = system_complete(path(strLogPath));

			// If it does not exist, create it
			if (!exists(logPath)) {
				if (!create_directory(logPath)) {
					if (isLoggingEnabled()) {
						LogStream& ls = getLogStream(LogStream::EnError);
						ls << "Cannot create Unitex logging directory " << logPath.string();
						ls.flush();
					}
					return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
				}
				if (isLoggingEnabled()) {
					logMessage("Created Unitex logging directory %s", logPath.string().c_str());
				}
			}

			pUnitexLogger = new UnitexLogInstaller(logPath.string());
			if (isLoggingEnabled()) {
				logMessage("Installed Unitex logging into %s", logPath.string().c_str());
			}
		}

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Initializes the path for Unitex resources directory.
	*/
	TyErrorId UnitexAnnotatorCpp::initializeUnitexResourcePath()
	{
		boost::optional<string> unitexResourcePath = get_env("UNITEX_RESOURCES");
		if (isLoggingEnabled()) {
			LogStream& ls = getLogStream(LogStream::EnMessage);
			if (unitexResourcePath)
				ls << "Checking UNITEX_RESOURCES = '" << *unitexResourcePath << "'";
			else
				ls << "UNITEX_RESOURCES is not defined";
			ls.flush();
		}
		if (!unitexResourcePath || (unitexResourcePath == boost::none) || unitexResourcePath->empty()) {
			logWarning("Environment variable UNITEX_RESOURCES is not defined. Looking in DataPath");
			// In UIMACPP the getDataPath() does not exist so we rather use the following
			string resourcePathDefinition = ResourceManager::getInstance().getLocationData().getAsCString();
			size_t len = resourcePathDefinition.length();
			while ((len > 0) && ((resourcePathDefinition[len - 1] == '/') || (resourcePathDefinition[len - 1] == '\\')))
				len--;
			path dataPath = resourcePathDefinition.substr(0, len);
			dataPath.make_preferred();
			// First check if there is a "unitex" subdirectory containing resources
			path unitexPath = dataPath / "unitex";
			if (isLoggingEnabled())
				logMessage("Checking if %s exists...", unitexPath.string().c_str());
			if (is_directory(unitexPath)) {
				unitexResourcePath = absolute(unitexPath).string();
			}
			else {
				// Then check if there is a PulseMyUnitex subdirectory
				unitexPath = dataPath / "PulseMyUnitex";
				if (isLoggingEnabled())
					logMessage("Checking if %s exists...", unitexPath.string().c_str());
				if (is_directory(unitexPath)) {
					unitexResourcePath = absolute(unitexPath).string();
				}
				else  {
					// Otherwise try to find the first subdirectory with "unitex" in the filename
					directory_iterator end_dirIt;
					for (directory_iterator dirIt(dataPath); dirIt != end_dirIt; dirIt++) {
						path entryPath = (*dirIt).path();
						if (is_directory(entryPath) && icontains(entryPath.string(), "unitex")) {
							if (isLoggingEnabled())
								logMessage("DataPath subdirectory %s is an Unitex ressource directory", entryPath.string().c_str());
							unitexResourcePath = absolute(entryPath).string();
							break;
						}
					}
				}
			}
		}

		if (!unitexResourcePath) {
			logError("No Unitex directory in DataPath");
			return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
		} else {
			if (isLoggingEnabled())
				logMessage("DataPath subdirectory %s is an Unitex ressource directory", unitexResourcePath->c_str());
		}

		m_pathUnitexResourcesDir = absolute(path(*unitexResourcePath));
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex Resources directory =" << m_pathUnitexResourcesDir << endl;
#endif

		boost::optional<string> unitexAppPath = get_env("UNITEX_HOME");
		if (unitexAppPath)
			m_pathUnitexAppDir = absolute(path(*unitexAppPath)) / path("App");
#ifdef DEBUG_UIMA_CPP
		cout << "Unitex App directory =" << m_pathUnitexAppDir << endl;
#endif

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Initializes as many instances of Unitex as supported languages
	*/
	TyErrorId UnitexAnnotatorCpp::initializeUnitexInstances()
	{
		ostringstream oss;

		// If the parameter -Dunitex.log=true was passed to the JVM, we want
		// to log Unitex activity for further debugging purposes
		//initializeUnitexLogger();

#ifdef DEBUG_UIMA_CPP
		cout << "Enter UnitexAnnotatorCpp::initializeUnitexInstances" << endl;
#endif
		unitexInstances.clear();

		bool isFirstLanguage = true;
		TyErrorId uimaError = UIMA_ERR_NONE;

		const set<UnicodeString> supportedLanguages = getAnnotatorContext().getGroupNamesForParameter(PARAM_DICTIONARIES);

		BOOST_FOREACH(const UnicodeString& language, supportedLanguages) {
#ifdef DEBUG_UIMA_CPP
			cout << "Initializing resources for language " << language << endl;
#endif

			if (isFirstLanguage) {
				uimaError = getAnnotatorContext().extractValue(PARAM_FORCE_GRAPH_COMPILATION, m_forceGraphCompilation);
				if (uimaError != UIMA_ERR_NONE) {
					if (isLoggingEnabled()) {
						LogStream& ls = getLogStream(LogStream::EnError);
						ls << "Cannot read parameter " << PARAM_FORCE_GRAPH_COMPILATION;
						ls.flush();
					}
					return uimaError;
				}
				if (isLoggingEnabled())
					logMessage("Initializing force graph compilations to %s", m_forceGraphCompilation ? "TRUE" : "FALSE");
			}

			set<UnicodeString> supportedStrategies;
			uimaError = getStrategiesInDescriptor(language, supportedStrategies);
			if (uimaError != UIMA_ERR_NONE) {
				logError("Cannot read analysis strategies from UnitexAnnotatorCpp descriptor!");
				return uimaError;
			}
			if (isLoggingEnabled()) {
				LogStream& ls = getLogStream(LogStream::EnMessage);
				ls << "Supported analysis strategies are: ";
				write(ls, supportedStrategies);
				ls.flush();
			}

			bool manyStrategies = (supportedStrategies.size() > 1); 

			for (set<UnicodeString>::const_iterator itStrategy = supportedStrategies.begin(); itStrategy != supportedStrategies.end(); itStrategy++) {
				const UnicodeString& strategy = *itStrategy;
#ifdef DEBUG_UIMA_CPP
				cout << "Language= " << language << " / Strategy= " << strategy << endl;
#endif

				if (manyStrategies && (strategy == "GENERAL"))
					continue;

				try {
					vector<UnicodeString> graphs;
					getMasterGraphsFromDescriptor(language, manyStrategies ? strategy : UNICODE_STRING_SIMPLE("GENERAL"), graphs);
#ifdef DEBUG_UIMA_CPP
					cout << "Master graphs = ";
					write(cout, graphs);
#endif

					vector<UnicodeString> dictionaries;
					getDictionariesFromDescriptor(language, dictionaries);
#ifdef DEBUG_UIMA_CPP
					cout << "Dictionaries = ";
					write(cout, dictionaries);
#endif

					map<UnicodeString, UnicodeString> morphoDictionaries;
					uimaError = readMorphoDictionariesParameter(language, morphoDictionaries);
					if (uimaError == UIMA_ERR_CONFIG_NAME_VALUE_PAIR_NOT_FOUND) {
						if (isLoggingEnabled()) {
							LogStream& ls = getLogStream(LogStream::EnWarning);
							ls << "Parameter " << PARAM_MORPHO_DICTIONARIES << " is not specified, do not read morphological dictionaries";
							ls.flush();
						}
					}
					else if (uimaError != UIMA_ERR_NONE) {
						if (isLoggingEnabled()) {
							LogStream& ls = getLogStream(LogStream::EnWarning);
							ls << "Cannot read morphological dictionaries for " << language;
							ls.flush();
						}
					}
#ifdef DEBUG_UIMA_CPP
					if (morphoDictionaries.empty())
						cout << "No morphological dictionaries" << endl;
					else {
						cout << "Morphological dictionaries = [";
						for (map<UnicodeString, UnicodeString>::const_iterator mIt = morphoDictionaries.begin(); mIt != morphoDictionaries.end(); mIt++)
							cout << "\t" << mIt->first << " -> " << mIt->second << endl;
						cout << "]" << endl;
					}
#endif

					if (isLoggingEnabled()) {
						LogStream& ls = getLogStream(LogStream::EnMessage);
						ls << "Instantiating a Unitex engine for " << language << " / " << strategy;
						ls.flush();
					}
					UnitexEngine* pEngine = new UnitexEngine(*this, language, m_pathUnitexResourcesDir, dictionaries, graphs, morphoDictionaries);
					if (!pEngine->validResources()) {
						logError("Linguistic resources are not valid!");
						return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
					}

					// Adapt the Sentence graph to the actual graphs in the
					// descriptor
					for (vector<UnicodeString>::const_iterator itGraph = graphs.begin(); itGraph != graphs.end(); itGraph++) {
						const UnicodeString& graph = *itGraph;
						if (graph.indexOf("Sentence") >= 0) {
							path p = pEngine->getGraphsDir() / convertUnicodeStringToRawString(graph);
							p.make_preferred();
							pEngine->setSentenceFstFile(p.string());
							break;
						}
					}

					UnicodeString langKey = language;
					langKey.toLower();
					UnicodeString stratKey = strategy;
					stratKey.toLower();

					unitexInstances[make_pair(langKey, stratKey)] = pEngine;
				} catch (UnitexException& e) {
					getLogger().logError(e.what());
					return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
				}
			}

			isFirstLanguage = false;
		}

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Retrieves the UNITEX_HOME absolute path.
	*/
	path UnitexAnnotatorCpp::getUnitexHomePath() const
	{
		path result;
		const char* szUnitexHome = getenv("UNITEX_HOME");
		if (szUnitexHome) {
			path unitexHome(szUnitexHome);
			result = absolute(system_complete(unitexHome));
		}
		return result;
	}

	/**
	* Checks whether the given path is a folder containing a subdirectory named 'Dela'.
	* This is useful to detect a Unitex resource folder.
	*/
	bool folderContainsDelaDirectory(const path& folderPath)
	{
		if (exists(folderPath) && is_directory(folderPath)) {
			path subpath = folderPath / "Dela";
			return exists(subpath) && is_directory(subpath);
		}
		return false;
	}

	/**
	* Checks if the given directory path is a Unitex resource directory,
	* i.e. if it contains a folder which in its turn contains a 'Dela' subdirectory.
	*/
	bool isUnitexResourceDirectory(const path& directoryPath)
	{
		if (exists(directoryPath) && is_directory(directoryPath)) {
			directory_iterator it(directoryPath), directory_end;
			while (it != directory_end) {
				if (folderContainsDelaDirectory((*it).path()))
					return true;
				it++;
			}
		}
		return false;
	}

	/**
	* Explores the UIMA datapath to look for a Unitex resource directory.
	*/
	path UnitexAnnotatorCpp::getUnitexResourcePath() const
	{
		path result;
		string datapath = ResourceManager::getInstance().getLocationData().getAsCString();
		if (isUnitexResourceDirectory(datapath))
			result = absolute(system_complete(datapath));
		else {
			recursive_directory_iterator it(datapath), directory_end;
			while (it != directory_end) {
				if (isUnitexResourceDirectory((*it).path())) {
					result = absolute(system_complete((*it).path()));
					break;
				}
				it++;
			}
		}
		return result;
	}

	/**
	* Gets the list of supported strategies by browsing the descriptor and
	* looking for the "Graphs" parameter for the given language.
	*
	* @param language
	* 				the language for which we want the strategies
	* @param strategies
	* 				the list where to store the supported strategies
	* @return UIMA_ERR_NONE if all went well
	*/
	TyErrorId UnitexAnnotatorCpp::getStrategiesInDescriptor(const UnicodeString& language, set<UnicodeString>& strategies) const
	{
		strategies.clear();

		vector<UnicodeString> entries;
		if (getAnnotatorContext().extractValue(language, PARAM_GRAPHS, entries) != UIMA_ERR_NONE) {
			ostringstream oss;
			oss << "Cannot extract value of configuration parameter " << PARAM_GRAPHS << " in component descriptor";
			logError(oss.str());
			return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
		}

		for (vector<UnicodeString>::const_iterator it = entries.begin(); it != entries.end(); it++) {
			strategies.insert(getStrategyFromGraphEntry(*it));
		}

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Splits a descriptor's graph entry and extracts the strategy name (GENERAL
	* by default).
	*
	* @param graphEntry
	*            a graph entry <strategy>=<graph path>
	* @return the strategy name
	*/
	string UnitexAnnotatorCpp::getStrategyFromGraphEntry(const std::string& graphEntry)
	{

		vector<string> items;
		split(items, graphEntry, is_any_of("="));
		if (items.size() > 1)
			return boost::to_upper_copy(items[0]);
		else
			return "GENERAL";
	}

	TyErrorId UnitexAnnotatorCpp::getDictionariesFromDescriptor(const UnicodeString& language, vector<UnicodeString>& dictionaries)
	{
		TyErrorId error = UIMA_ERR_NONE;
		string strLanguage = convertUnicodeStringToRawString(language);

		dictionaries.clear();

		// If the SystemDictionaries and UserDictionaries parameters
		// are defined, they supercede the Dictionaries parameter
		UnicodeString systemDictionariesParam;
		error = getAnnotatorContext().extractValue(language, PARAM_SYSTEMDICTIONARIES, systemDictionariesParam);
		if (error != UIMA_ERR_NONE) {
			ostringstream oss;
			oss << "Cannot read value of parameter " << PARAM_SYSTEMDICTIONARIES << endl;
			logError(oss.str());
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Parameter " << PARAM_SYSTEMDICTIONARIES << " = " << systemDictionariesParam << endl;
#endif

		UnicodeString userDictionariesParam;
		error = getAnnotatorContext().extractValue(language, PARAM_USERDICTIONARIES, userDictionariesParam);
		if (error != UIMA_ERR_NONE) {
			ostringstream oss;
			oss << "Cannot read value of parameter " << PARAM_USERDICTIONARIES << endl;
			logError(oss.str());
			return error;
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Parameter " << PARAM_USERDICTIONARIES << " = " << userDictionariesParam << endl;
#endif

		if (!systemDictionariesParam.isEmpty() || !userDictionariesParam.isEmpty()) {
			if (!systemDictionariesParam.isEmpty() && !userDictionariesParam.isEmpty()) {
				path systemDictionariesPath = m_pathUnitexResourcesDir / strLanguage / convertUnicodeStringToRawString(systemDictionariesParam);
#ifdef DEBUG_UIMA_CPP
				cout << "systemDictionariesPath = " << systemDictionariesPath << endl;
#endif
				path userDictionariesPath = m_pathUnitexResourcesDir / strLanguage / convertUnicodeStringToRawString(userDictionariesParam);
#ifdef DEBUG_UIMA_CPP
				cout << "userDictionariesPath = " << userDictionariesPath << endl;
#endif

				if (!exists(systemDictionariesPath))
					logMessage("System dictionaries definition file '%s' does not exist, skip.", systemDictionariesPath.string().c_str());
				else {
					vector<UnicodeString> dicos;
					readDictionariesDefinitionFile(systemDictionariesPath, dicos);
					dictionaries.insert(dictionaries.end(), dicos.begin(), dicos.end());
				}
				if (!exists(userDictionariesPath))
					logMessage("User dictionaries definition file '%s' does not exist, skip.", userDictionariesPath.string().c_str());
				else {
					vector<UnicodeString> dicos;
					readDictionariesDefinitionFile(userDictionariesPath, dicos);
					dictionaries.insert(dictionaries.end(), dicos.begin(), dicos.end());
				}
				if (dictionaries.empty()) {
					logError("The list of dictionaries to be used is empty!");
					return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
				}
			} else {
				ostringstream oss;
				oss << "Both parameters must be defined together: " << PARAM_SYSTEMDICTIONARIES << "=" << systemDictionariesParam << ", " << PARAM_USERDICTIONARIES << "=" << userDictionariesParam;
				logError(oss.str());
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}
		}
		// Otherwise read the Dictionaries parameter
		else
			error = getAnnotatorContext().extractValue(language, PARAM_DICTIONARIES, dictionaries);

		return error;
	}

	void UnitexAnnotatorCpp::readDictionariesDefinitionFile(const path& defFilePath, vector<UnicodeString>& dictionaries)
	{
		UnicodeString fileContents;
		getStringFromFile(defFilePath, fileContents);

		if (!isEmpty(fileContents) && !isBlank(fileContents)) {
			// Split it into lines non empty lines
			splitLines(dictionaries, fileContents, false);
		}
	}

	TyErrorId UnitexAnnotatorCpp::readMorphoDictionariesParameter(const UnicodeString& language, map<UnicodeString, UnicodeString>& morphoDictionaries)
	{
		morphoDictionaries.clear();
		vector<UnicodeString> entries;
		TyErrorId error = getAnnotatorContext().extractValue(language, PARAM_MORPHO_DICTIONARIES, entries);
		if (error == UIMA_ERR_CONFIG_NAME_VALUE_PAIR_NOT_FOUND) {
			if (isLoggingEnabled(LogStream::EnWarning)) {
				LogStream& ls = getLogStream(LogStream::EnWarning);
				ls << "Parameter " << PARAM_MORPHO_DICTIONARIES << " << not specified, do not load morphological dictionaries";
				ls.flush();
			}
			return UIMA_ERR_NONE;
		}
		else if (error != UIMA_ERR_NONE) {
			ostringstream oss;
			oss << "Error while reading parameter " << PARAM_MORPHO_DICTIONARIES << " from descriptor: " << AnalysisEngine::getErrorIdAsCString(error);
			logError(oss.str());
			return error;
		}

		for (vector<UnicodeString>::const_iterator it = entries.begin(); it != entries.end(); it++) {
			const UnicodeString& entry = *it;
			int32_t pos = entry.indexOf(UNICODE_STRING_SIMPLE("="));
			if (pos > 0) {
				UnicodeString left, right;
				entry.extract(0, pos, left);
				entry.extract(pos + 1, entry.length() - (pos + 1), right);
				morphoDictionaries[left] = right;
			}
		}
		return UIMA_ERR_NONE;
	}

	/**
	* Initialization subroutine to get the list of fake tokens, i.e. tokens that
	* may have been introduced in Unitex input but should not be taken into account because
	* they are only marks to help the graphs detect some special places or boundaries.
	*/
	TyErrorId UnitexAnnotatorCpp::initializeUnitexFakeTokens()
	{
		logMessage("Initializing Unitex fake tokens...");

		fakeTokens.clear();
		fakeTokenReplacements.clear();

		if (getAnnotatorContext().isParameterDefined(PARAM_FAKE_TOKENS)) {
			if (getAnnotatorContext().extractValue(PARAM_FAKE_TOKENS, fakeTokens) != UIMA_ERR_NONE) {
				ostringstream oss;
				oss << "Cannot extract value of configuration parameter " << PARAM_FAKE_TOKENS << " in component descriptor";
				getLogger().logError(oss.str());
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
			}

			// Creates artificial strings to protect the fake tokens in the RMB,
			// especially to avoid messing with meta-messages speaking about these fake tokens :-)
			// We take stupid strings with improbable Unicode characters, but of the same length.
			for (size_t i = 0; i < fakeTokens.size(); i++) {
				if (fakeTokens[i].length() < 3) {
					ostringstream oss;
					oss << "Fake token '" << fakeTokens[i] << "' shoult be at least 3 characters!";
					logError(oss.str());
					return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_INIT;
				}
				UnicodeString replacement;
				for (int32_t j = 0; j < fakeTokens[i].length(); j++)
					replacement.append((UChar32) (0x2EB + i));
				fakeTokenReplacements.push_back(replacement);
				if (isLoggingEnabled()) {
					LogStream& ls = getLogStream(LogStream::EnMessage);
					ls << "  fake token '" << fakeTokens[i] << "' => '" << fakeTokenReplacements[i] << "'" << endl;
					ls.flush();
				}
			}
		}

		return (TyErrorId) UIMA_ERR_NONE;
	}

	TyErrorId UnitexAnnotatorCpp::getMasterGraphsFromDescriptor(const UnicodeString& language, const UnicodeString& strategy, vector<UnicodeString>& graphs)
	{
		TyErrorId error = UIMA_ERR_NONE;

		vector<UnicodeString> entries;
		error = getAnnotatorContext().extractValue(language, PARAM_GRAPHS, entries);
		if (error != UIMA_ERR_NONE)
			return error;
#ifdef DEBUG_UIMA_CPP
		cout << "Graphs parameter for " << language << " = ";
		write(cout, entries);
		cout << endl;
#endif

		graphs.clear();
		for (vector<UnicodeString>::const_iterator it = entries.begin(); it != entries.end(); it++) {
			UnicodeString entry = *it;
			if (getStrategyFromGraphEntry(*it) == strategy)
				graphs.push_back(getGraphFromGraphEntry(*it));
		}
#ifdef DEBUG_UIMA_CPP
		cout << "Kept graphs = ";
		write(cout, graphs);
		cout << endl;
#endif

		return UIMA_ERR_NONE;
	}

	UnicodeString UnitexAnnotatorCpp::getStrategyFromGraphEntry(const UnicodeString& entry)
	{
		int32_t pos = entry.indexOf(UNICODE_STRING_SIMPLE("="));
		if (pos > 0) {
			UnicodeString result;
			entry.extract(0, pos, result);
			result.toUpper();
			return result;
		}
		return UNICODE_STRING_SIMPLE("GENERAL");
	}

	UnicodeString UnitexAnnotatorCpp::getGraphFromGraphEntry(const UnicodeString& entry)
	{
		int32_t pos = entry.indexOf(UNICODE_STRING_SIMPLE("="));
		if (pos > 0) {
			UnicodeString result;
			entry.extract(pos + 1, entry.length() - (pos + 1), result);
			return result;
		}
		return entry;
	}

	/** */
	TyErrorId UnitexAnnotatorCpp::typeSystemInit(TypeSystem const & crTypeSystem)
	{
		string tcasUnitexPrefix = "org.gramlab.kwaga.unitex_uima.unitex.tcas.";
		string tcasGeneralPrefix = "org.gramlab.kwaga.unitex_uima.general.tcas.";

		string dynamicDictionaryTypeName = tcasUnitexPrefix + "DynamicDictionary";
		tDynamicDictionary = crTypeSystem.getType(dynamicDictionaryTypeName.c_str());
		if (!tDynamicDictionary.isValid()) {
			logError("Error getting Type %s", dynamicDictionaryTypeName.c_str());
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		fEntries = tDynamicDictionary.getFeatureByBaseName("entries");

		if (UnitexDocumentParameters::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing UnitexDocumentParameters");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (LanguageArea::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing LanguageArea");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (TokenAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing TokenAnnotation");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (ParagraphAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing ParagraphAnnotation");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (SentenceAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing SentenceAnnotation");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (TransductionOutputAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing TransductionOutputAnnotation");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (AnnotatorPerformanceAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing AnnotatorPerformanceAnnotation");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		if (AutomatonLocatePerformanceAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE) {
			logError("Error initializing AutomatonLocatePerformanceAnnotation");
			return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;
		}
		typeSystemInitialized = true;
		logMessage("Type System initialized with success");

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// Destruction
	//
	/////////////////////////////////////////////////////////////////////////

	/** */
	TyErrorId UnitexAnnotatorCpp::destroy()
	{
		logMessage("Calling UnitexAnnotatorCpp::destroy()");

		// Delete the Unitex Logger
		delete pUnitexLogger;
		pUnitexLogger = NULL;

		// Delete Unitex instances
		for (UnitexInstanceMap::const_iterator it = unitexInstances.begin(); it != unitexInstances.end(); it++)
			delete it->second;
		unitexInstances.clear();

		// Delete Language instances
		unitexcpp::Language::destroyStatic();

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// Process
	//
	/////////////////////////////////////////////////////////////////////////

	typedef boost::date_time::c_local_adjustor<boost::system_time> local_adj;

	/** */
	TyErrorId UnitexAnnotatorCpp::process(CAS & tcas, ResultSpecification const & crResultSpecification)
	{
		ProfilingLogger profilingLogger(tcas, *this);

		// Reinitialize the working view for the new document
		m_pWorkingView = NULL;
		initializeWorkingView(tcas);

		pCurrentCAS = &tcas;
		if (isLoggingEnabled(LogStream::EnMessage))
			logMessage("Start UnitexAnnotatorCpp for %s", getDocumentIdAsString().c_str());

		if (skip()) {
			logMessage("Skip processing");
			return UIMA_ERR_NONE;
		}

		nbTokens = 0;
		m_nbTokensForOutputProcessing = 0;

		if (skip()) {
			logMessage("Leave now because skipKwaga is set");
			return UIMA_ERR_NONE;
		}

		// If there is a specific analysis strategy, we use it, otherwise it
		// is a "general" strategy
		UnicodeString strategy = toString(getAnalysisStrategy());
		if (strategy == UNICODE_STRING_SIMPLE("UNKNOWN"))
			strategy = UNICODE_STRING_SIMPLE("GENERAL");

		CAS& view = getView();
		list<LanguageArea> languageAreas;
		try {
			LanguageArea::allLanguageAreasInView(view, languageAreas);
		} catch (UnitexException& e) {
			logError("Unable to retrieve language areas");
		}


		ostringstream oss;
#ifdef WIN32
		oss << "Corpus" << GetCurrentThreadId();
#else
		oss << "Corpus" << pthread_self();
#endif
		path corpusPath = persistedPath(path(oss.str()));

		BOOST_FOREACH(LanguageArea& languageArea, languageAreas) {
			VirtualFolderCleaner vfsCleaner(corpusPath);

			UnicodeString language = toString(languageArea.getLanguage());
			UnitexEngine& unitexEngine = selectUnitexLanguageInstance(language, strategy);
			unitexEngine.clearPerformanceCache();

			if (isLoggingEnabled(LogStream::EnMessage)) {
				LogStream& ls = getLogStream(LogStream::EnMessage);
				ls << "Processing an area in " << language;
				ls.flush();
			}

			UnicodeStringRef rustrInputText = languageArea.getCoveredText();
			if (rustrInputText.length() == 0)
				continue;

			if (isLoggingEnabled(LogStream::EnMessage)) {
				LogStream& ls = getLogStream(LogStream::EnMessage);
				ls << "-----[Start of area]-----" << endl;
				ls << toString(rustrInputText) << endl;
				ls << "-----[End of area]-----";
				ls.flush();
			}

			// Create a temporary input file whose content is the text we want to analyze
			// (either in memory if using a Virtual File System, or as a file on disk).
			path inputPath = prepareInputFile(unitexEngine, languageArea, corpusPath);
#ifdef DEBUG_UIMA_CPP
			cout << "Input for Unitex is ready" << endl;
			UnicodeString ustrInputFileContent;
			getStringFromFile(inputPath, ustrInputFileContent);
			cout << ustrInputFileContent << endl;
#endif
			VirtualFolderCleaner vfsSntCleaner(unitexEngine.getSntDirectory());

			prepareDynamicDictionary(unitexEngine);
#ifdef DEBUG_UIMA_CPP
			cout << "Dynamic dictionaries are built" << endl;
#endif

			try {
				list<QualifiedString> unitexResultMatches;

#ifdef DEBUG_UIMA_CPP
				m_timeoutDelay = 0;
#endif
#ifdef DEBUG_MEMORY_LEAKS
				m_timeoutDelay = 0;
#endif

				// Launch Unitex main annotation process
				// and monitor its state until we reach a time out delay
				if (m_timeoutDelay) {
					boost::posix_time::time_duration delay = boost::posix_time::milliseconds(m_timeoutDelay);
					boost::packaged_task<list<QualifiedString> > task(boost::bind(&UnitexEngine::run, &unitexEngine, inputPath.string()));
					boost::unique_future<list<QualifiedString> > future = task.get_future();
					try {
						logMessage("Running Unitex with a time out delay of %d ms", m_timeoutDelay);
						boost::thread unitexThread(boost::move(task));
#ifdef DEBUG_UIMA_CPP
						cout << "Built thread " << unitexThread.get_id() << " to run Unitex in background" << endl;
#endif

						if (future.timed_wait(delay))
							logMessage("Unitex task complete in acceptable time");
						else
							logWarning("Unitex task not complete before time out");
					} 
					catch (boost::thread_interrupted const&) {
						logError("Time Out in Unitex! Aborting...");
						return UIMA_ERR_USER_ANNOTATOR_ERROR_IN_SUBSYSTEM;
					}
					catch (boost::broken_promise& ex) {
						logError("Broken promise while running Unitex in a background task: %s", ex.what());
						return UIMA_ERR_USER_ANNOTATOR_ERROR_IN_SUBSYSTEM;
					}
					catch (std::exception& ex) {
						logError("Exception while running Unitex in a background task: %s", ex.what());
						return UIMA_ERR_USER_ANNOTATOR_ERROR_IN_SUBSYSTEM;
					}

					if (!future.is_ready()) {
						logError("Future task running Unitex is not complete");
						return UIMA_ERR_USER_ANNOTATOR_ERROR_IN_SUBSYSTEM;
					}
					if (!future.has_value()) {
						logError("Future task running Unitex has no return value");
						return UIMA_ERR_USER_ANNOTATOR_ERROR_IN_SUBSYSTEM;
					}
					if (future.has_exception()) {
						logWarning("Future task running Unitex raised exceptions");
						return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_PROCESS;
					}
					if (future.get_state() != boost::future_state::ready) {
						logError("Future task running Unitex is not ready to be read");
						return UIMA_ERR_USER_ANNOTATOR_ERROR_IN_SUBSYSTEM;
					}

					unitexResultMatches = future.get();
					if (isLoggingEnabled(LogStream::EnMessage))
						logMessage("Unitex task returned %d matches", unitexResultMatches.size());
				} else {
					logMessage("Running Unitex engine without timeout delay");
					unitexResultMatches = unitexEngine.run(inputPath.string());
					if (isLoggingEnabled(LogStream::EnMessage))
						logMessage("Unitex returned %d matches", unitexResultMatches.size());
				}

				if (isLoggingEnabled(LogStream::EnMessage)) {
					UnicodeString normalizedRmb;
					unitexEngine.getNormalizedText(normalizedRmb);
					LogStream& ls = getLogStream(LogStream::EnMessage);
					ls << "Normalized RMB:" << endl << normalizedRmb << endl;
					ls.flush();
				}

				UnitexTokenizer tokenizer(*this, fakeTokens, unitexEngine, languageArea.getBegin(), languageArea.getCoveredText());
#ifdef DEBUG_UIMA_CPP
				cout << "Built a UnitexTokenizer for this area" << endl;
#endif
				nbTokens += tokenizer.tokenize(nbTokens);
#ifdef DEBUG_UIMA_CPP
				cout << "RMB tokenized, we now have " << nbTokens << " tokens" << endl;
#endif
				processUnitexOutput(unitexResultMatches, languageArea, tokenizer);
			} catch (const UnitexException& e) {
				logError("UnitexException during UnitexAnnotatorCpp process: %s", e.what());
				return UIMA_ERR_USER_ANNOTATOR_COULD_NOT_PROCESS;
			}
		}

		if (isLoggingEnabled(LogStream::EnMessage)) {
			LogStream& ls = getLogStream(LogStream::EnMessage);
			ls << "End UnitexAnnotatorCpp processing for " << getDocumentId();
			ls.flush();
		}

		m_nbProcessedDocuments++;

		return (TyErrorId) UIMA_ERR_NONE;
	}

	/**
	* Selects the current running Unitex engine to process the right language.
	*
	* @param uclstrLanguage the language name as a Unicode string
	*/
	UnitexEngine& UnitexAnnotatorCpp::selectUnitexLanguageInstance(const UnicodeString& language, const UnicodeString& strategy)
	{
		UnitexEngine* pUnitexEngine = NULL;

		UnicodeString actualLanguage = language;
		UnicodeString actualStrategy = strategy;

		set<UnicodeString> supportedStrategies;
		getSupportedStrategies(supportedStrategies);
		if (supportedStrategies.size() == 1)
			actualStrategy = *(supportedStrategies.begin());

		actualLanguage.toLower();
		actualStrategy.toLower();
		pair<UnicodeString, UnicodeString> key = make_pair(actualLanguage, actualStrategy);
		UnitexInstanceMap::const_iterator it = unitexInstances.find(key);

		if (it != unitexInstances.end()) {
			ostringstream oss;
			oss << "Using Unitex instance for [" << actualLanguage << ", " << actualStrategy << "]";
			logMessage(oss.str());
		} else {
			// Try with French
			key.first = UNICODE_STRING_SIMPLE("french");
			ostringstream oss;
			oss << "Unsupported language " << actualLanguage << ", trying [" << key.first << ", " << key.second << "]";
			logMessage(oss.str());
			it = unitexInstances.find(key);
			if (it == unitexInstances.end()) {
				pair<UnicodeString, UnicodeString> defaultKey = unitexInstances.begin()->first;
				oss.str("");
				oss << "Unsupported language " << actualLanguage << " and strategy " << actualStrategy << ", defaulting to [" << defaultKey.first << ", " << defaultKey.second << "]";
				logMessage(oss.str());
				key = defaultKey;
			}
		}
		pUnitexEngine = unitexInstances[key];

		if (pUnitexEngine == NULL)
			throw UnitexException("Unable to select a Unitex engine instance!");
		return *pUnitexEngine;
	}

	void UnitexAnnotatorCpp::getSupportedStrategies(set<UnicodeString>& strategies) const
	{
		strategies.clear();
		for (UnitexInstanceMap::const_iterator it = unitexInstances.begin(); it != unitexInstances.end(); it++) {
			strategies.insert(it->first.second);
		}
	}

	/**
	* Virtualizes in VFS or write as a genuine file, according to the system's
	* capabilities, the file that will be processed by Unitex.
	*/
	path UnitexAnnotatorCpp::prepareInputFile(UnitexEngine& unitexEngine, const LanguageArea& languageArea, const path& corpusPath) const
	{
#ifdef DEBUG_UIMA_CPP
		cout << "Preparing input file" << endl;
#endif
		path inputPath;

		// Builds the actual input text:
		// - an optional opening tag (defined in the area)
		// - the area covered text
		// - an optional closing tag (defined in the area)
		UnicodeString content;
		languageArea.getCoveredText(content);
#ifdef DEBUG_UIMA_CPP
		cout << "Initial content:" << endl << content << endl;
#endif
		// Protect {S} before processing with Unitex
		content.findAndReplace(UNICODE_STRING_SIMPLE("{S}"), UNICODE_STRING_SIMPLE("@@@"));

		// Protect { and }
		content.findAndReplace(UNICODE_STRING_SIMPLE("{"), UnicodeString(0x02EA));
		content.findAndReplace(UNICODE_STRING_SIMPLE("}"), UnicodeString(0x02E9));

		// Protect fake tokens in input
		for (size_t i = 0; i < fakeTokens.size(); i++)
			content.findAndReplace(fakeTokens[i], fakeTokenReplacements[i]);

		UnicodeString inputText;
		UnicodeStringRef opening = languageArea.getAdditionalOpeningTag();
		if (!opening.isEmpty())
			inputText.append(toString(opening));
		inputText.append(content);
		UnicodeStringRef closing = languageArea.getAdditionalClosingTag();
		if (!closing.isEmpty())
			inputText.append(toString(closing));

		// Get rid of any blank or line feeds at the end of the text
		int32_t len = inputText.length(), pos = len - 1;
		while ((pos >= 0) && u_isUWhiteSpace(inputText.char32At(pos)))
			pos--;
		if (pos < 0)
			inputText.remove();
		else
			inputText.removeBetween(pos + 1, len);


		inputPath = corpusPath / "unitexInput.txt";
		writeStringToFile(inputPath, inputText);

		return inputPath;
	}

	/**
	* Prepares a dynamic dictionary to be used by the Unitex engine, from the
	* dictionary entries that might have been generated by another annotator
	* earlier in the chain.
	*
	* The dictionary file is either virtualpized in VFS or written as a genuine
	* file, depending on the system's capabilities.
	*/
	void UnitexAnnotatorCpp::prepareDynamicDictionary(UnitexEngine& unitexEngine)
	{
		unitexEngine.clearDynamicDictionaries();

		// put the dynamic dictionary either in a virtual file or in
		// a temporary file, depending on JUnitex functionality
		AnnotationFS fsDynamicDictionary = getDynamicDictionary();
		if (fsDynamicDictionary.isValid()) {

			StringArrayFS entries = fsDynamicDictionary.getStringArrayFSValue(fEntries);
			if (entries.size() > 0) {
				UnicodeString ustring;
				for (size_t i = 0; i < entries.size(); i++) {
					UnicodeStringRef rEntry = entries.get(i);
					UnicodeString uEntry;
					rEntry.extract(0, rEntry.length(), uEntry);
					ustring.append(uEntry);
					ustring.append(UNICODE_STRING_SIMPLE("\n"));
				}

				string strDictFile("*dyndic.dic");
				writeStringToFile(strDictFile, ustring);

				// check and compress the dynamic dictionary
				try {
					unitexEngine.addDynamicDictionary(strDictFile);
				} catch (UnitexException& e) {
					getLogger().logError(e.what());
					throw;
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// Linguistic hints
	//
	/////////////////////////////////////////////////////////////////////////

	/*!
	* Processes all outputs of Unitex, creating a linguistic hint for each of
	* them, with the right offsets and token indexes to remember where they are
	* in the text.
	*/
	void UnitexAnnotatorCpp::processUnitexOutput(list<QualifiedString>& unitexResultMatches, const LanguageArea& languageArea, UnitexTokenizer& tokenizer)
	{
#ifdef DEBUG_UIMA_CPP
		cout << "Processing Unitex output: " << unitexResultMatches.size() << " matches" << endl;
		for (list<QualifiedString>::iterator it = unitexResultMatches.begin(); it != unitexResultMatches.end(); it++)
			cout << "\t" << *it << endl;
#endif
		CAS& view = getView();
		size_t offset = languageArea.getBegin();

#ifdef DEBUG_UIMA_CPP
		cout << "offset=" << offset << "\tnbTokensForOutputProcessing=" << m_nbTokensForOutputProcessing << endl;
#endif
		unitexResultMatches.sort();

		for (list<QualifiedString>::iterator it = unitexResultMatches.begin(); it != unitexResultMatches.end(); it++) {
			UnicodeString& output = it->getString();

			// Reverse protection of fake tokens in input text
			for (size_t i = 0; i < fakeTokens.size(); i++)
				output = output.findAndReplace(fakeTokenReplacements[i], fakeTokens[i]);

#ifdef DEBUG_UIMA_CPP
			cout << "Processing Unitex output " << output << endl;
#endif

			// Compute the start and end offset of the transducer capture
			// by shifting them into the raw representation coming out of Unitex
			// (by default, the offsets provided by Unitex refer to a version of
			// the text without the {S} sentence marks)
			int32_t start = it->getStart() + m_nbTokensForOutputProcessing;
#ifdef DEBUG_UIMA_CPP
			cout << "\tshifted first token index = " << start << endl;
#endif
			int32_t end = it->getEnd() + m_nbTokensForOutputProcessing;
#ifdef DEBUG_UIMA_CPP
			cout << "\tshifted last token index = " << end << endl;
#endif

			TokenAnnotation* pFirstToken = tokenizer.getTokenByIndex(start);
			int32_t originalStart = start;
			while ((pFirstToken == NULL) && (start <= end)) {
				start++;
				pFirstToken = tokenizer.getTokenByIndex(start);
			}
			if ((start > end) || (pFirstToken == NULL)) {
				logError("Cannot find first token with index %d", originalStart);
				continue;
			}
#ifdef DEBUG_UIMA_CPP
			cout << "First token = " << *pFirstToken << endl;
#endif

			TokenAnnotation* pLastToken = tokenizer.getTokenByIndex(end);
			int32_t originalEnd = end;
			while ((pLastToken == NULL) && (end >= start)) {
				end--;
				pLastToken = tokenizer.getTokenByIndex(end);
			}
			if ((end < start) || (pLastToken == NULL)) {
				logError("Cannot find last token with index %d", originalEnd);
				continue;
			}
#ifdef DEBUG_UIMA_CPP
			cout << "Last token = " << *pLastToken << endl;
#endif

			// Find the sentence containing these tokens
			SentenceAnnotation sentence = SentenceAnnotation::findSentenceContainingToken(*pFirstToken);
			if (!sentence.isValid())
				logWarning("Cannot find sentence containing token starting at %d", pFirstToken->getBegin());

			// Create a linguistic hint annotation in the RMB view
			TransductionOutputAnnotation annotation(view, pFirstToken->getBegin(), pLastToken->getEnd(), output);
#ifdef DEBUG_UIMA_CPP
			cout << "create transduction output " << annotation << endl;
#endif
		}

		m_nbTokensForOutputProcessing = nbTokens;
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// UIMA utils
	//
	/////////////////////////////////////////////////////////////////////////

	/**
	* Initializes the current document's "working view", i.e. either
	* "_InitialView" or "realMailBodyView" depending on which one
	* contains a "UnitexDocumentParameters" annotation.
	*
	* If no such view exists, a CASException is raised.
	*/
	void UnitexAnnotatorCpp::initializeWorkingView(CAS& cas)
	{
		CAS* pView = NULL;

		// 1. Try to find a UnitexDocumentParameters annotation in the _InitialView.
		try {
			pView = cas.getView("_InitialView");
		}
		catch (CASException& e) {
			logError("No _InitialView in current document, this is wrong!");
			throw e;
		}
		try {
			UnitexDocumentParameters::getUnitexDocumentParameters(*pView);
		}
		catch (UnitexException& e) {
			if (isLoggingEnabled(LogStream::EnEntryType::EnWarning))
				logWarning("No UnitexDocumentParameters in _InitialView, try with realMailBodyView");
			pView = NULL;
		}

		// 2. Try in realMailBodyView
		if (!pView) {
			try {
				pView = cas.getView("realMailBodyView");
			}
			catch (CASException& e) {
				logError("No realMailBodyView in current document, we cannot find a view with a UnitexDocumentParameters annotation!");
				throw e;
			}
			try {
				UnitexDocumentParameters::getUnitexDocumentParameters(*pView);
			}
			catch (UnitexException& e) {
				logError("No UnitexDocumentParameters in realMailBodyView, we cannot find a view with a UnitexDocumentParameters annotation!");
				pView = NULL;
				throw e;
			}
		}

		m_pWorkingView = pView;
	}

	/**
	* Retrieves the working view for the given document.
	* This is either "_InitialView" or "realMailBodyView" depending on which
	* one contains a "UnitexDocumentParameters" annotation.
	*/
	CAS& UnitexAnnotatorCpp::getView() const
	{
		return *m_pWorkingView;
	}

	UnitexDocumentParameters UnitexAnnotatorCpp::getUnitexDocumentParameters() const
	{
		return UnitexDocumentParameters::getUnitexDocumentParameters(getView());
	}

	UnicodeStringRef UnitexAnnotatorCpp::getDocumentId() const
	{
		return getUnitexDocumentParameters().getUri();
	}

	string UnitexAnnotatorCpp::getDocumentIdAsString() const
	{
		ostringstream oss;
		oss << getDocumentId();
		return oss.str();
	}

	UnicodeStringRef UnitexAnnotatorCpp::getAnalysisStrategy() const
	{
		return getUnitexDocumentParameters().getAnalysisStrategy();
	}

	/**
	* Returns true iff the skip flag is set.
	*/
	bool UnitexAnnotatorCpp::skip() const
	{
		return getUnitexDocumentParameters().getSkip();
	}

	bool UnitexAnnotatorCpp::forceGraphCompilation() const
	{
		return m_forceGraphCompilation;
	}

	/**
	* Retrieves the dynamic dictionary annotation associated to the RMB view.
	*/
	AnnotationFS UnitexAnnotatorCpp::getDynamicDictionary() const
	{
		CAS& view = getView();
		ANIterator iterator = view.getAnnotationIndex(tDynamicDictionary).iterator();
		if (iterator.isValid()) {
			iterator.moveToFirst();
			return iterator.get();
		}
		return AnnotationFS();
	}

	/*!
	* Tests the equality of two strings modulo the character replacements that
	* have been introduced by prepareInputFile to protect special characters by
	* Unitex.
	*
	* \param s1 a string
	* \param s2 another string
	* \return true iff both strings are equal modulo the replacements introduced by prepareInputFile
	*/
	bool UnitexAnnotatorCpp::equalsModuloUnitexSpecialCharacterReplacements(const UnicodeString& s1, const UnicodeString& s2)
	{
		if (s1 == s2)
			return true;
		if (s1 == "{S}")
			return (s2 == "@@@");
		if (s2 == "{S}")
			return (s1 == "@@@");
		if ((s1.indexOf("{") >= 0) || (s1.indexOf("}") >= 0)) {
			UnicodeString a = s1;
			return (a.findAndReplace("\\{", UnicodeString((UChar32) 0x02EA)).findAndReplace("\\}", UnicodeString((UChar32) 0x02E9)) == s2);
		}
		if ((s2.indexOf("{") >= 0) || (s2.indexOf("}") >= 0)) {
			UnicodeString b = s2;
			return (b.findAndReplace("\\{", UnicodeString((UChar32) 0x02EA)).findAndReplace("\\}", UnicodeString((UChar32) 0x02E9)) == s1);
		}
		return false;
	}

	const UnitexAnnotatorCpp::UnitexInstanceMap& UnitexAnnotatorCpp::getUnitexInstances() const
	{
		return unitexInstances;
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// Logging
	//
	/////////////////////////////////////////////////////////////////////////

#define MAX_LOG_MSG_SIZE	2048

	/**
	* Allocates a string buffer with the right size to hold the whole message,
	* and stores the message in it.
	* It shall be deleted after use.
	* \return a string buffer
	*/
	char* UnitexAnnotatorCpp::buildLogBuffer(const char* szFormat, va_list ap) const
	{
		char* szBuffer = new char[MAX_LOG_MSG_SIZE];
		memset(szBuffer, 0, MAX_LOG_MSG_SIZE);

		int need = 1 + vsnprintf(szBuffer, MAX_LOG_MSG_SIZE, szFormat, ap);
		if (need > MAX_LOG_MSG_SIZE) {
			delete[] szBuffer;
			szBuffer = new char[need];
			memset(szBuffer, 0, need);
			vsprintf(szBuffer, szFormat, ap);
		}

		return szBuffer;
	}

	void UnitexAnnotatorCpp::logMessage(const char* szFormat, ...) const
	{
		if (isLoggingEnabled() && (getLoggingLevel() == LogStream::EnMessage)) {
			va_list ap;
			va_start(ap, szFormat);
			char* szMessage = buildLogBuffer(szFormat, ap);
			va_end(ap);

			getLogger().logMessage(szMessage);

			delete[] szMessage;
		}
	}

	void UnitexAnnotatorCpp::logMessage(const string& format, ...) const
	{
		if (isLoggingEnabled() && (getLoggingLevel() == LogStream::EnMessage)) {
			va_list ap;
			va_start(ap, format);
			char* szMessage = buildLogBuffer(format.c_str(), ap);
			va_end(ap);

			getLogger().logMessage(szMessage);

			delete[] szMessage;
		}
	}

	void UnitexAnnotatorCpp::logWarning(const char* szFormat, ...) const
	{
		if (isLoggingEnabled() && (getLoggingLevel() <= LogStream::EnWarning)) {
			va_list ap;
			va_start(ap, szFormat);
			char* szMessage = buildLogBuffer(szFormat, ap);
			va_end(ap);

			getLogger().logWarning(szMessage);

			delete[] szMessage;
		}
	}

	void UnitexAnnotatorCpp::logWarning(const string& format, ...) const
	{
		if (isLoggingEnabled() && (getLoggingLevel() <= LogStream::EnWarning)) {
			va_list ap;
			va_start(ap, format);
			char* szMessage = buildLogBuffer(format.c_str(), ap);
			va_end(ap);

			getLogger().logWarning(szMessage);

			delete[] szMessage;
		}
	}

	void UnitexAnnotatorCpp::logError(const char* szFormat, ...) const
	{
		if (isLoggingEnabled() && (getLoggingLevel() <= LogStream::EnError)) {
			va_list ap;
			va_start(ap, szFormat);
			char* szMessage = buildLogBuffer(szFormat, ap);
			va_end(ap);

			getLogger().logError(szMessage);

			delete[] szMessage;
		}
	}

	void UnitexAnnotatorCpp::logError(const string& format, ...) const
	{
		if (isLoggingEnabled() && (getLoggingLevel() <= LogStream::EnError)) {
			va_list ap;
			va_start(ap, format);
			char* szMessage = buildLogBuffer(format.c_str(), ap);
			va_end(ap);

			getLogger().logError(szMessage);

			delete[] szMessage;
		}
	}

	// This macro exports an entry point that is used to create the annotator.

	MAKE_AE(UnitexAnnotatorCpp);

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

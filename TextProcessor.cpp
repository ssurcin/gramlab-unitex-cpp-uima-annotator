/*
 * TextProcessor.cpp
 *
 *  Created on: 10 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "TextProcessor.h"
#include "UnitexEngine.h"
#include "ConcordCommand.h"
#include "LocateCommand.h"
#include "UnitexException.h"
#include <unicode/ustdio.h>
#include <unicode/regex.h>
#include <unicode/ustream.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include "Utils.h"
#include "FileUtils.h"
#include <xercesc/parsers/SAXParser.hpp>
#include "uima/api.hpp"
#include "UnitexAnnotatorCpp.h"

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
using namespace uima;
using namespace unitexcpp;
using namespace unitexcpp::command;
using namespace boost::filesystem;
using namespace xercesc;

namespace unitexcpp
{
	namespace engine
	{
		/*!
		 * Utility class to measure the time spent in an automaton using RIIA techniques.
		 */
		class AutomatonPerformanceTimer
		{
			boost::timer m_timer;
			TextProcessor& m_textProcessor;
			string m_automaton;

		public:
			AutomatonPerformanceTimer(TextProcessor& textProcessor, string const& automaton)
				: m_textProcessor(textProcessor)
			{
				path autoPath(automaton);
				m_automaton = autoPath.filename().string();
			}
			virtual ~AutomatonPerformanceTimer()
			{
				long locateDuration = m_timer.elapsed() * 1000;
				map<string, long>::const_iterator it = m_textProcessor.m_locateProfileInfo.find(m_automaton);
				if (it != m_textProcessor.m_locateProfileInfo.end())
					locateDuration += it->second;
				m_textProcessor.m_locateProfileInfo[m_automaton] = locateDuration;
			}
		};

		/**
		 * Builds a new instance for an Unitex engine.
		 */
		TextProcessor::TextProcessor(UnitexEngine& unitexEngine) :
					UnitexSubengine(unitexEngine)
		{
			m_isProfilingEnabled = false;

			const AnnotatorContext& annotatorContext = getAnnotatorContext();
			if (annotatorContext.isParameterDefined(UnitexAnnotatorCpp::PARAM_LONGEST_MATCH_OUTPUT)) {
				if (annotatorContext.extractValue(UnitexAnnotatorCpp::PARAM_LONGEST_MATCH_OUTPUT, m_isProfilingEnabled) != UIMA_ERR_NONE) {
					ostringstream oss;
					oss << "Cannot extract value of configuration parameter " << UnitexAnnotatorCpp::PARAM_LONGEST_MATCH_OUTPUT << " in component descriptor";
					logError(oss.str());
					throw UnitexException(oss.str());
				}
			}
		}

		TextProcessor::~TextProcessor()
		{
		}

		/**
		 * Process an already preprocessed text by applying an automaton and
		 * building a concordance file in UIMA mode. Check the validity of automaton
		 * output as XML.
		 *
		 * @param result
		 *            a list of qualified strings corresponding to the automaton output
		 * @param sntFile
		 *            the SNT file name
		 * @param automaton
		 *            the automaton file name (.fst2)
		 * @param locateStartOnSpace
		 *            true if we allow Unitex's Locate to start on morphological
		 *            spaces
		 * @throws UnitexException
		 */
		void TextProcessor::processAndCheck(list<QualifiedString>& result, const string& sntFile, const string& automaton, bool locateStartOnSpace)
		{
			result.clear();

			list<QualifiedString> rawResults;
			process(rawResults, sntFile, automaton, locateStartOnSpace);

#ifdef DEBUG_UIMA_CPP
			cout << "TextProcessor checking XML consistency of results" << endl;
#endif
			// Check the XML consistency of transducers output
			BOOST_FOREACH(QualifiedString& qstring, rawResults) {
#ifdef DEBUG_UIMA_CPP
				cout << "Checking qstring " << qstring << endl;
#endif
				const UnicodeString& trans = protectXmlEntities(qstring.getString());
				try {
					SAXParser parser;
					readXMLString(parser, qstring.getString());
					qstring.setString(trans);
					result.push_back(qstring);
				}
				catch (const xercesc::SAXException& e) {
					ostringstream oss;
					oss << "Error in transducer " << automaton << " output: '" << qstring.getString() << "'" << endl << e.getMessage();
					throw UnitexException(oss.str());
				}
				catch (const xercesc::XMLException& e) {
					ostringstream oss;
					oss << "Error in transducer " << automaton << " output: '" << qstring.getString() << "'" << endl << e.getMessage();
					throw UnitexException(oss.str());
				}
			}
		}

		/**
		 * Process an already preprocessed text by applying an automaton and
		 * building a concordance file in UIMA mode.
		 *
		 * @param result
		 *            a list of qualified strings corresponding to the automaton output
		 * @param sntFile
		 *            the SNT file name
		 * @param automaton
		 *            the automaton file name (.fst2)
		 * @param locateStartOnSpace
		 *            true if we allow Unitex's Locate to start on morphological
		 *            spaces
		 * @throws UnitexException
		 */
		void TextProcessor::process(list<QualifiedString>& result, const string& sntFile, const string& automaton, bool locateStartOnSpace)
		{
#ifdef DEBUG_UIMA_CPP
			cout << "Applying automaton " << automaton << endl;
#endif

			m_sntDirectory = sntFile.substr(0, sntFile.length() - 4) + "_snt";
			path concordIn = path(m_sntDirectory) / "concord.ind";
			path concordOut;

			if (!locate(automaton, sntFile)) {
				ostringstream oss;
				oss << "TextProcessor error while applying locate for automaton " << automaton;
				throw UnitexException(oss.str());
			}
			return getRawConcordanceResults(result, concordIn);
		}

		/**
		 * Builds an in-memory representation of a transducer's outputs by reading the
		 * raw output of transducer.
		 *
		 * \param result
		 *            a list of qualified strings containing the start and end offset
		 *            of the recognized text and the corresponding transducer's output
		 * \param indexPath
		 *            the path to the index file produced by the transducer
		 */
		void TextProcessor::getRawConcordanceResults(list<QualifiedString>& result, const path& indexPath)
		{
#ifdef DEBUG_UIMA_CPP
			cout << "Getting raw concordance results" << endl;
#endif

			static RegexPattern* pPattern = NULL;
			UErrorCode error = U_ZERO_ERROR;

			if (!pPattern) {
				error = U_ZERO_ERROR;
				pPattern = RegexPattern::compile("(\\d+)\\.\\d+\\.\\d+ (\\d+)\\.\\d+\\.\\d+ (.+)", 0, error);
				if (U_FAILURE(error)) {
					ostringstream oss;
					oss << "Error building getRawConcordanceResults regex: " << u_errorName(error);
					throw UnitexException(oss.str());
				}
			}

			// Get the file contents
			UnicodeString contents;
			getUnicodeStringFromUnitexFile(indexPath.string(), contents);
#ifdef DEBUG_UIMA_CPP
			cout << "Contents of index file " << indexPath << "=" << endl << contents << endl;
#endif

			// Split it into lines
			UnicodeStringlist lines;
			splitRegex(lines, contents, "\\n");
#ifdef DEBUG_UIMA_CPP
			cout << lines.size() << " lines" << endl;
#endif

			for (UnicodeStringlist::const_iterator it = lines.begin(); it != lines.end(); it++) {
				// Skips padding line
				UnicodeString line = *it;
				line.trim();
#ifdef DEBUG_UIMA_CPP
				cout << "\tline =" << line << endl;
#endif
				if (line.isEmpty() || line.startsWith("#"))
					continue;

				error = U_ZERO_ERROR;
				RegexMatcher* pMatcher = pPattern->matcher(line, error);
				if (U_FAILURE(error)) {
					ostringstream oss;
					oss << "Error in getRawConcordanceResults matching: " << u_errorName(error);
					throw UnitexException(oss.str());
				}

				error = U_ZERO_ERROR;
				if (pMatcher->find()) {
					error = U_ZERO_ERROR;
					UnicodeString strStart = pMatcher->group(1, error);
					if (U_FAILURE(error)) {
						ostringstream oss;
						oss << "Error in getRawConcordanceResults get match group 1: " << u_errorName(error);
						throw UnitexException(oss.str());
					}
#ifdef DEBUG_UIMA_CPP
					cout << "\tstrStart=" << strStart << endl;
#endif

					error = U_ZERO_ERROR;
					UnicodeString strEnd = pMatcher->group(2, error);
					if (U_FAILURE(error)) {
						ostringstream oss;
						oss << "Error in getRawConcordanceResults get match group 2: " << u_errorName(error);
						throw UnitexException(oss.str());
					}
#ifdef DEBUG_UIMA_CPP
					cout << "\tstrEnd=" << strEnd << endl;
#endif

					error = U_ZERO_ERROR;
					UnicodeString output = pMatcher->group(3, error);
					if (U_FAILURE(error)) {
						ostringstream oss;
						oss << "Error in getRawConcordanceResults get match group 3: " << u_errorName(error);
						throw UnitexException(oss.str());
					}
#ifdef DEBUG_UIMA_CPP
					cout << "\toutput=" << output << endl;
#endif

					int start, end;
					ostringstream oss;
					oss << strStart << " " << strEnd;
					istringstream iss(oss.str());
					iss >> start >> end;
#ifdef DEBUG_UIMA_CPP
					cout << "\tstart=" << start << ", end=" << end << endl;
#endif

					result.push_back(QualifiedString(start, end, output));
				}
			}
		}

		/**
		 * Applies the given automaton to locate the corresponding entities
		 *
		 * @param automaton
		 *        the name of an automaton file (starts with a * if virtual)
		 * @param inputFilename
		 *        the name of a text file to apply the automaton (starts with a * if virtual)
		 * @param startOnSpace
		 *        allows search to start on spaces (default: false)
		 * @param matchMode
		 *        a matching mode (default: longest match)
		 * @param transductionMode
		 *        a transduction output mode (default: replace matched text with transduction output)
		 * @param protectDicChars
		 *        in MERGE and REPLACE transduction, protect certain characters with backslash (default: false)
		 * @param maxMatchNumber
		 *        the maximum number of matches (default: -1 is no limit)
		 * @param alphabet
		 *        complete path to the alphabet file (default: the Unitex engine's alphabet)
		 * @param morphoDics
		 *        a list of morphological dictionaries to apply (default: none)
		 * @param noStartOnSpaceMatching
		 *        allows matching on spaces (default: false)
		 * @param charByChar
		 *        search character by character (default: false)
		 * @param wordByWord
		 *        search word by word (default: true)
		 * @param sntDir
		 *        specifies another directory where to store the output file than the text dictory (default: empty)
		 * @param korean
		 *        language is Korean (default: false)
		 * @param arabicTypoRulesFile
		 *        a file containing arabic typographic rules (default: empty)
		 * @param negOperator
		 *        the negation operator (default: TILDE)
		 * @param allowsAmbiguousOutput
		 *        allows ambiguous output (default: true)
		 * @param errorBehaviour
		 *        specifies the behaviour to adopt when an error of variable occurs (default: ignore)
		 * @param warnTokenCount
		 *        after how many processed tokens a warning will be issued (default: -1 is no limit)
		 * @param stopTokenCount
		 *       after how many processed tokens the process will be interrupted (default: -1 is no limit)
		 *
		 * @throws UnitexException
		 */
		bool TextProcessor::locate(	const string& automaton,
				const string& inputFilename,
				bool startOnSpace,
				const MatchMode& matchMode,
				const TransductionMode& transductionMode,
				bool protectDicChars,
				int maxMatchNumber,
				const string& alphabet,
				const Stringlist& morphoDics,
				bool noStartOnSpaceMatching,
				bool charByChar,
				bool wordByWord,
				const string& sntDir,
				bool korean,
				const string& arabicTypoRulesFile,
				const NegationOperator& negOperator,
				bool allowsAmbiguousOutput,
				const LocateVariableErrorBehaviour& errorBehaviour,
				int warnTokenCount,
				int stopTokenCount)
		{
#ifdef DEBUG_UIMA_CPP
			cout << "Building Locate command" << endl;
#endif
			// Keep track of the time spent in this automaton
			AutomatonPerformanceTimer performanceTimer(*this, automaton);

			Stringlist persistedMorphoDicts;
			getEngine().getMorphologicalDictionaries(automaton, persistedMorphoDicts);
			BOOST_FOREACH(const string& dict, morphoDics) {
				persistedMorphoDicts.push_back(dict);
			}

			LocateCommand command(getEngine(), automaton, inputFilename, matchMode, transductionMode, protectDicChars, maxMatchNumber, alphabet, persistedMorphoDicts, startOnSpace, noStartOnSpaceMatching,
					charByChar, wordByWord, sntDir, korean, arabicTypoRulesFile, negOperator, allowsAmbiguousOutput, errorBehaviour, warnTokenCount, stopTokenCount);
#ifdef DEBUG_UIMA_CPP
			cout << "Executing Locate command" << endl;
#endif
			return command.execute();
		}

		/**
		 * Builds a concordance file and returns this file's name.
		 *
		 * @param concordFile
		 *            a raw concordance as produced by locate
		 * @param mode
		 *            the concordance mode (see Unitex manual)
		 *            (default: HTML)
		 * @param sortedAlphabet
		 *            a sorted alphabet file
		 *            (default: the Unitex instance's sorted alphabet)
		 * @param glossanetScript
		 *            if mode is glossanet, this is the name of a glossanet script
		 *            file to use
		 *            (default: empty)
		 * @param order
		 *            the concordance order (see Unitex manual)
		 * @param nLeftContext
		 *            the number of characters to take in the left context
		 * @param nRightContext
		 *            the number of characters to take in the right context
		 * @param fontName
		 *            the font name to use when mode is HTML
		 * @param nFontSize
		 *            the font size to use when mode is HTML
		 * @param bThai
		 *            true if we are processing thai
		 * @return the name of the processed concordance file
		 * @throws UnitexException
		 */
		path TextProcessor::concordance(const path& concordFile,
				const ConcordanceMode& mode,
				const string& sortedAlphabet,
				const string& glossanetScript,
				const ConcordanceOrder& order,
				int nLeftContext,
				int nRightContext,
				const string& fontName,
				int nFontSize,
				bool bThai)
		{
			ConcordCommand command(getEngine(), concordFile.string(), sortedAlphabet, mode, glossanetScript, order, nLeftContext, nRightContext, fontName, nFontSize, bThai);
			if (!command.execute())
				return "";

			if ((mode == ConcordanceMode::HTML) || (mode == ConcordanceMode::GLOSSANET))
				return change_extension(concordFile, ".html");
			else
				return change_extension(concordFile, ".txt");
		}

		/*!
		 * Clears the automaton performance map so that it does not cumulate
		 * with the processings of another message.
		 */
		void TextProcessor::clearPerformanceMap()
		{
			m_locateProfileInfo.clear();
		}
	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

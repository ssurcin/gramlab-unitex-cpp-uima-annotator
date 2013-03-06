/*
 * TextProcessor.h
 *
 *  Created on: 10 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TEXTPROCESSOR_H_
#define TEXTPROCESSOR_H_

#include "UnitexSubengine.h"
#include "UnitexTypes.h"
#include "Utils.h"

namespace unitexcpp
{

	namespace engine
	{

		class UnitexEngine;
		class QualifiedString;

		/**
		 * Delegate class implementing the operations relative to text processing by
		 * Unitex in class UnitexEngine.
		 *
		 * @author surcin@kwaga.com
		 *
		 */
		class TextProcessor: public unitexcpp::engine::UnitexSubengine
		{
			friend class AutomatonPerformanceTimer;

		public:
			TextProcessor(UnitexEngine& unitexEngine);
			virtual ~TextProcessor();

		public:
			void processAndCheck(std::list<QualifiedString>& result, const std::string& sntFile, const std::string& automaton, bool locateStartOnSpace);
			void process(std::list<QualifiedString>& result, const std::string& sntFile, const std::string& automaton, bool locateStartOnSpace);
		private:
			void getRawConcordanceResults(std::list<QualifiedString>& result, const boost::filesystem::path& indexPath);

		public:
			bool locate(const std::string& automaton,
						const std::string& inputFilename,
						bool startOnSpace = false,
						const unitexcpp::MatchMode& matchMode = unitexcpp::MatchMode::LONGEST,
						const unitexcpp::TransductionMode& transductionMode = unitexcpp::TransductionMode::REPLACE,
						bool protectDicChars = false,
						int maxMatchNumber = -1,
						const std::string& alphabet = "",
						const unitexcpp::Stringlist& morphoDics = EMPTYSTRINGLIST,
						bool noStartOnSpaceMatching = false,
						bool charByChar = false,
						bool wordByWord = false,
						const std::string& sntDir = "",
						bool korean = false,
						const std::string& arabicTypoRulesFile = "",
						const unitexcpp::NegationOperator& negOperator = unitexcpp::NegationOperator::TILDE,
						bool allowsAmbiguousOutput = true,
						const unitexcpp::LocateVariableErrorBehaviour& errorBehaviour = unitexcpp::LocateVariableErrorBehaviour::IGNORE,
						int warnTokenCount = -1,
						int stopTokenCount = -1);
			boost::filesystem::path concordance(const boost::filesystem::path& concordFile,
												const unitexcpp::ConcordanceMode& mode = unitexcpp::ConcordanceMode::HTML,
												const std::string& sortedAlphabet = "",
												const std::string& glossanetScript = "",
												const unitexcpp::ConcordanceOrder& order = unitexcpp::ConcordanceOrder::TEXT_ORDER,
												int nLeftContext = 40,
												int nRightContext = 255,
												const std::string& fontName = "Courier New",
												int nFontSize = 12,
												bool bThai = false);

			void clearPerformanceMap();
			const std::map<std::string, long>& getLocatePerformanceMap() const
			{
				return m_locateProfileInfo;
			}

		private:
			std::string m_sntDirectory;
			bool m_isProfilingEnabled;
			std::map<std::string, long> m_locateProfileInfo;
		};

	}

}

#endif /* TEXTPROCESSOR_H_ */

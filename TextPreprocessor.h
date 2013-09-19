/*
 * TextPreprocessor.h
 *
 *  Created on: 7 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef TEXTPREPROCESSOR_H_
#define TEXTPREPROCESSOR_H_

#include "UnitexSubengine.h"
#include "UnitexTypes.h"
#include "Utils.h"
#include <string>
#include <uima/unistrref.hpp>

namespace unitexcpp
{

	namespace engine
	{

		class TextPreprocessor: public unitexcpp::engine::UnitexSubengine
		{
		public:
			TextPreprocessor(UnitexEngine& unitexEngine);
			virtual ~TextPreprocessor();

			bool preprocess(const std::string& inputFilename);
			bool normalize(const std::string& inputName, const std::string& equivFileName = "", bool bConvertCRtoSpace = false, bool bNormalizeSeparators = false, bool bConvertLFtoCRLF = false, const std::string& offsetsFileName = "normoffsets.txt");
			bool fst2txt(	const std::string& inputName,
							const std::string& alphabet = "",
							bool startOnSpace = false,
							bool charByChar = false,
							const std::string& fstName = "",
							const unitexcpp::Fst2TxtMode& mode = unitexcpp::Fst2TxtMode::MERGE,
							const std::string& inputOffsets = "normoffsets.txt",
							const std::string& outputOffsets = "sntoffsets.txt");
			bool tokenize(const std::string& inputName, const std::string& alphabet = "", bool charByChar = false, const std::string& inputOffsets = "sntoffsets.txt", const std::string& outputOffsets =
					"snttokens.txt");
			bool applyDictionaries(	const std::string& inputName,
									const std::string& alphabet = "",
									const unitexcpp::Stringlist& binDicNames = unitexcpp::EMPTYSTRINGLIST,
									const unitexcpp::Stringlist& lstDictionaries = unitexcpp::EMPTYSTRINGLIST);

		private:
			void addInitialSentenceMark(const std::string& sntFilename);
			void setNormalizedText(const std::string& sntFilename, const uima::UnicodeStringRef& normalizedText);
		};

	}

}

#endif /* TEXTPREPROCESSOR_H_ */

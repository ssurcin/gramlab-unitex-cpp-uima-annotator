/*
 * DictionaryCompiler.h
 *
 *  Created on: 4 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef DICTIONARYCOMPILER_H_
#define DICTIONARYCOMPILER_H_

#include "UnitexSubengine.h"
#include "UnitexTypes.h"
#include <string>

namespace unitexcpp
{

	namespace engine
	{

		class UnitexEngine;

		class DictionaryCompiler: public UnitexSubengine
		{
		public:
			DictionaryCompiler(UnitexEngine& engine);
			virtual ~DictionaryCompiler();

			bool compile(const boost::filesystem::path& dictionaryPath, const boost::filesystem::path& alphabetPath, const DictionaryType& dictionaryType = DictionaryType::DELAF, bool moveFile = false);
			bool checkDictionary(const boost::filesystem::path& dictionaryPath, const boost::filesystem::path& alphabetPath, const DictionaryType& dictionaryType = DictionaryType::DELAF);
			bool compressDictionary(const boost::filesystem::path& dictionaryPath, bool swapInflectedAndLemmaForms = false);

		private:
			static std::set<boost::filesystem::path> compiledDictionaries;		
		};

	}
}

#endif /* DICTIONARYCOMPILER_H_ */

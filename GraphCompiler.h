/*
 * GraphCompiler.h
 *
 *  Created on: 6 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef GRAPHCOMPILER_H_
#define GRAPHCOMPILER_H_

#include "UnitexSubengine.h"
#include <string>
#include <boost/filesystem.hpp>

namespace unitexcpp
{

	namespace engine
	{

		class UnitexEngine;

		/**
		 * Delegate class implementing the operations relative to graph compilation by
		 * UnitexEngine.
		 */
		class GraphCompiler: public unitexcpp::engine::UnitexSubengine
		{
		public:
			GraphCompiler(UnitexEngine& unitexEngine);
			virtual ~GraphCompiler();

			bool compile(const boost::filesystem::path& sourceGraph,
						 bool sentenceGraph =false,
						 const boost::filesystem::path& compiledGraph =boost::filesystem::path());
			bool grf2fst2(const boost::filesystem::path& sourceGraph,
					      bool checkValidSentence =false,
					      const boost::filesystem::path& compiledGraph =boost::filesystem::path(),
					      const boost::filesystem::path& repositoryFolder =boost::filesystem::path(),
					      const boost::filesystem::path& alphabet =boost::filesystem::path(),
					      bool loopCheck =true,
					      bool charByChar =false,
					      bool noEmptyGraphWarning =true) const;
			bool flatten(const boost::filesystem::path& automaton, bool unfold =false, bool recursive =true, int depth =5) const;
			bool fst2check(const boost::filesystem::path& automaton, bool sentenceCheck =false);

		private:
			static std::set<boost::filesystem::path> compiledGraphs;
		};

	}

}

#endif /* GRAPHCOMPILER_H_ */

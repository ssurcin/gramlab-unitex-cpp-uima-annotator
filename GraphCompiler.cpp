/*
 * GraphCompiler.cpp
 *
 *  Created on: 6 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "GraphCompiler.h"
#include "UnitexEngine.h"
#include "UnitexException.h"
#include "Grf2Fst2Command.h"
#include "FlattenCommand.h"
#include "Fst2CheckCommand.h"
#include <boost/filesystem.hpp>
#include "Utils.h"
#include "FileUtils.h"

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
using namespace boost::filesystem;
using namespace unitexcpp::command;

namespace unitexcpp
{

	namespace engine
	{

		set<path> GraphCompiler::compiledGraphs;

		GraphCompiler::GraphCompiler(UnitexEngine& unitexEngine) :
			UnitexSubengine(unitexEngine)
		{
		}

		GraphCompiler::~GraphCompiler()
		{
		}

		/**
		 * Compile and flatten a graph into a finite state transducer (.fst2),
		 * applying all default parameters.
		 *
		 * \param sourceGraph
		 *            the source graph path
		 * \param sentenceGraph
		 *            true if this is a sentence splitting graph
		 * \param compiledGraph
		 *            the target compiled graph path, if empty, just give an .fst2 extension to the source path

		 * \returns true if everything went well
		 */
		bool GraphCompiler::compile(const path& sourceGraph, bool sentenceGraph, const path& compiledGraph)
		{
			getAnnotator().logMessage("Compiling %s graph: %s", sentenceGraph ? "sentence" : "regular", sourceGraph.string().c_str());

			set<path>::const_iterator it = compiledGraphs.find(sourceGraph);
			if (it != compiledGraphs.end())
				return true;

			if (!grf2fst2(sourceGraph, false)) { // we never want to check if the graph is valid for sentence (it does not do what we think it does!)
				getAnnotator().logError("Error compiling graph: %s", sourceGraph.string().c_str());
				return false;
			}

			path fst2Path = change_extension(sourceGraph, ".fst2");

			if (!fst2check(fst2Path))
				return false;

			compiledGraphs.insert(sourceGraph);

			return true;
		}

		/**
		 * Compile a graph (.grf) into a finite state automaton (.fst2) and stores
		 * the resulting file in the specified directory.
		 *
		 * @param graph
		 *            the graph file name
		 * @param alphabet
		 *            an optional alphabet file name (or null)
		 * @param repositoryFolder
		 *            an optional repository folder (or null)
		 * @param loopCheck
		 *            set to true if we want to check the graph against errors
		 * @param charByChar
		 *            set to true if we want the graph to be processed char by char
		 *            (2.1 and above)
		 * @param noEmptyGraphWarning
		 *            set to true if we want to suppress empty graph warnings (2.1
		 *            and above)
		 * @param checkValidSentence
		 *            set to true if we want to check if the graph is a valid
		 *            Sentence graph (2.1 and above)
		 * @return true if everything went well
		 * @throws UnitexException
		 */
		bool GraphCompiler::grf2fst2(
				const path& sourceGraph,
				bool checkValidSentence,
				const path& compiledGraph,
				const path& repositoryFolder,
				const path& alphabet,
				bool loopCheck,
				bool charByChar,
				bool noEmptyGraphWarning) const
		{
			Grf2Fst2Command command(getEngine(), sourceGraph, compiledGraph, alphabet, repositoryFolder, loopCheck, charByChar, noEmptyGraphWarning, checkValidSentence, true, true);
			return command.execute();
		}

		/**
		 * This method takes a .fst2 grammar as its parameter, and tries to
		 * transform it into a finite state transducer.
		 *
		 * @param automaton
		 *            the .fst2 file name
		 * @param unfold
		 *            set to true if we want to unfold the graph to the maximum
		 *            possible depth
		 * @param recursive
		 *            set to true if we want to leave the remaining subgraph calls
		 *            after the specified depth
		 * @param depth
		 *            the depth relative to the recursive parameter
		 * @return true if everything went well
		 * @throws UnitexException
		 */
		bool GraphCompiler::flatten(const path& automaton, bool unfold, bool recursive, int depth) const
		{
			FlattenCommand command(getEngine(), automaton, unfold, recursive, depth);
			return command.execute();
		}

		bool GraphCompiler::fst2check(const path& automaton, bool sentenceCheck)
		{
			Fst2CheckCommand command(getEngine(), automaton, sentenceCheck, true, true, true);
			return command.execute();
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

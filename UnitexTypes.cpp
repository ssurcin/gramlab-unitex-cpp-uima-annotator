/*
 * DictionaryType.cpp
 *
 *  Created on: 4 janv. 2011
 *      Author: sylvainsurcin
 */

#include "UnitexTypes.h"

using namespace std;

namespace unitexcpp
{

	// Dictionary types
	const DictionaryType DictionaryType::DELAF("DELAF");
	const DictionaryType DictionaryType::DELAS("DELAS");

	// Modes of applying a FST2 to a text
	const Fst2TxtMode Fst2TxtMode::MERGE("merge");
	const Fst2TxtMode Fst2TxtMode::REPLACE("replace");

	// Matching modes
	const MatchMode MatchMode::SHORTEST("-S");
	const MatchMode MatchMode::LONGEST("-L");
	const MatchMode MatchMode::ALL("-A");

	// Transduction modes
	const TransductionMode TransductionMode::IGNORE("-I");
	const TransductionMode TransductionMode::MERGE("-M");
	const TransductionMode TransductionMode::REPLACE("--replace");

	// Behaviour in front of a variable error in commande Locate
	const LocateVariableErrorBehaviour LocateVariableErrorBehaviour::IGNORE("-Y");
	const LocateVariableErrorBehaviour LocateVariableErrorBehaviour::EXIT("-X");
	const LocateVariableErrorBehaviour LocateVariableErrorBehaviour::BACKTRACK("-Z");

	// Possible Locate negation operators
	const NegationOperator NegationOperator::MINUS("minus");
	const NegationOperator NegationOperator::TILDE("tilde");

	// Modes of building a concordance
	const ConcordanceMode ConcordanceMode::HTML("html");
	const ConcordanceMode ConcordanceMode::TEXT("text");
	const ConcordanceMode ConcordanceMode::GLOSSANET("glossanet");
	const ConcordanceMode ConcordanceMode::INDEX("index");
	const ConcordanceMode ConcordanceMode::UIMA("uima");
	const ConcordanceMode ConcordanceMode::AXIS("axis");
	const ConcordanceMode ConcordanceMode::XALIGN("xalign");
	const ConcordanceMode ConcordanceMode::REPLACE("replace");

	// Orders of building a concordance
	const ConcordanceOrder ConcordanceOrder::NONE("");
	const ConcordanceOrder ConcordanceOrder::TEXT_ORDER("TO");
	const ConcordanceOrder ConcordanceOrder::LEFT_CENTER("LC");
	const ConcordanceOrder ConcordanceOrder::LEFT_RIGHT("LR");
	const ConcordanceOrder ConcordanceOrder::CENTER_LEFT("CL");
	const ConcordanceOrder ConcordanceOrder::CENTER_RIGHT("CR");
	const ConcordanceOrder ConcordanceOrder::RIGHT_LEFT("RL");
	const ConcordanceOrder ConcordanceOrder::RIGHT_CENTER("RC");
}


/*
 * DictionaryType.h
 *
 *  Created on: 4 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef UNITEXTYPES_H_
#define UNITEXTYPES_H_

#include "JavaLikeEnum.h"
#include <string>

#undef IGNORE

namespace unitexcpp
{

	/**
	 * This class enumerates all possible dictionary types.
	 */
	class DictionaryType: public JavaLikeEnum<std::string>
	{
	protected:
		DictionaryType(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~DictionaryType()
		{
		}

	public:
		static const DictionaryType DELAF;
		static const DictionaryType DELAS;
	};

	/**
	 * This class enumerates all possible modes of applying a FST2 to a text.
	 */
	class Fst2TxtMode: public JavaLikeEnum<std::string>
	{
	protected:
		Fst2TxtMode(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~Fst2TxtMode()
		{
		}

	public:
		static const Fst2TxtMode MERGE;
		static const Fst2TxtMode REPLACE;
	};

	/**
	 * This class enumerates al possible matching modes when applying an automaton.
	 */
	class MatchMode: public JavaLikeEnum<std::string>
	{
	protected:
		MatchMode(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~MatchMode()
		{
		}

	public:
		static const MatchMode SHORTEST;
		static const MatchMode LONGEST;
		static const MatchMode ALL;
	};

	/**
	 * This class enumerates all possible output modes for automaton randuction.
	 */
	class TransductionMode: public JavaLikeEnum<std::string>
	{
	protected:
		TransductionMode(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~TransductionMode()
		{
		}

	public:
		static const TransductionMode IGNORE;
		static const TransductionMode MERGE;
		static const TransductionMode REPLACE;
	};

	/**
	 * This class enumerates all behaviours of Locale command when a variable error occurs.
	 */
	class LocateVariableErrorBehaviour: public JavaLikeEnum<std::string>
	{
	protected:
		LocateVariableErrorBehaviour(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~LocateVariableErrorBehaviour()
		{
		}

	public:
		static const LocateVariableErrorBehaviour IGNORE;
		static const LocateVariableErrorBehaviour EXIT;
		static const LocateVariableErrorBehaviour BACKTRACK;
	};

	/**
	 * This class enumerates the negation operators in locate rules.
	 */
	class NegationOperator: public JavaLikeEnum<std::string>
	{
	protected:
		NegationOperator(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~NegationOperator()
		{
		}

	public:
		static const NegationOperator MINUS;
		static const NegationOperator TILDE;
	};

	/**
	 * This class enumerates all possible concordance modes when building the concordance.
	 */
	class ConcordanceMode: public JavaLikeEnum<std::string>
	{
	protected:
		ConcordanceMode(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~ConcordanceMode()
		{
		}

	public:
		static const ConcordanceMode HTML;
		static const ConcordanceMode TEXT;
		static const ConcordanceMode GLOSSANET;
		static const ConcordanceMode INDEX;
		static const ConcordanceMode UIMA;
		static const ConcordanceMode AXIS;
		static const ConcordanceMode XALIGN;
		static const ConcordanceMode REPLACE;
	};

	/**
	 * This class enumerates all possible concordance order when building the concordance.
	 */
	class ConcordanceOrder: public JavaLikeEnum<std::string>
	{
	protected:
		ConcordanceOrder(const std::string& aString) :
			JavaLikeEnum<std::string> (aString)
		{
		}
		virtual ~ConcordanceOrder()
		{
		}

	public:
		static const ConcordanceOrder NONE;
		static const ConcordanceOrder TEXT_ORDER;
		static const ConcordanceOrder LEFT_CENTER;
		static const ConcordanceOrder LEFT_RIGHT;
		static const ConcordanceOrder CENTER_LEFT;
		static const ConcordanceOrder CENTER_RIGHT;
		static const ConcordanceOrder RIGHT_LEFT;
		static const ConcordanceOrder RIGHT_CENTER;
	};
}

#endif /* UNITEXTYPES_H_ */

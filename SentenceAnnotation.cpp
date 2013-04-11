/*
 * SentenceAnnotation.cpp
 *
 *  Created on: 15 janv. 2011
 *      Author: sylvainsurcin
 */

#include "SentenceAnnotation.h"
#include "ParagraphAnnotation.h"

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

namespace unitexcpp
{

	namespace annotation
	{

		Type SentenceAnnotation::tSentenceAnnotation;
		Feature SentenceAnnotation::fParagraph;

		/*!
		 * Initializes the typesystem types and features
		 */
		TyErrorId SentenceAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			if (ContextAreaAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE)
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			tSentenceAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.SentenceAnnotation");
			if (!tSentenceAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fParagraph = tSentenceAnnotation.getFeatureByBaseName("paragraph");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		/*!
		 * Default constructor builds an invalid instance
		 */
		SentenceAnnotation::SentenceAnnotation()
		{
		}

		SentenceAnnotation::SentenceAnnotation(CAS& aCas, size_t begin, size_t end, const TokenAnnotation& firstToken, const TokenAnnotation& lastToken) :
				ContextAreaAnnotation(aCas)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tSentenceAnnotation, begin, end);
			setFirstToken(firstToken);
			setLastToken(lastToken);
			indexRep.addFS(annotation);
		}

		SentenceAnnotation::SentenceAnnotation(uima::CAS& aCas, size_t begin, size_t end, const TokenAnnotation& firstToken, const TokenAnnotation& lastToken, const ParagraphAnnotation& paragraph)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tSentenceAnnotation, begin, end);
			setFirstToken(firstToken);
			setLastToken(lastToken);
			setParagraph(paragraph);
			indexRep.addFS(annotation);
		}

		SentenceAnnotation::SentenceAnnotation(AnnotationFS& anAnnotation) :
				ContextAreaAnnotation(anAnnotation)
		{
		}

		SentenceAnnotation::~SentenceAnnotation()
		{
		}

		SentenceAnnotation& SentenceAnnotation::operator =(const SentenceAnnotation& model)
		{
			pCas = model.pCas;
			annotation = model.annotation;
			return *this;
		}

		void SentenceAnnotation::setParagraph(const ParagraphAnnotation& paragraph)
		{
			annotation.setFSValue(fParagraph, paragraph.getAnnotation());
		}

		ParagraphAnnotation SentenceAnnotation::getParagraph() const
		{
			AnnotationFS fs(annotation.getFSValue(fParagraph));
			return ParagraphAnnotation(fs);
		}

		/**
		 * Finds the sentence annotation containing the given token.
		 *
		 * \param token the token
		 * \return the sentence or an invalid sentence if not found
		 */
		SentenceAnnotation SentenceAnnotation::findSentenceContainingToken(TokenAnnotation& token)
		{
			FSIterator iterator = token.getView().getAnnotationIndex(tSentenceAnnotation).iterator();
			if (iterator.isValid()) {
				iterator.moveToFirst();
				while (iterator.isValid()) {
					AnnotationFS annotation(iterator.get());
					SentenceAnnotation sentence(annotation);
					if ((sentence.getFirstToken().getIndex() <= token.getIndex()) && (sentence.getLastToken().getIndex() >= token.getIndex()))
						return sentence;
					iterator.moveToNext();
				}
			}
			return SentenceAnnotation();
		}

		ostream& operator <<(ostream& os, const SentenceAnnotation& sentence)
		{
			return os << "SentenceAnnotation(begin=" << sentence.getBegin() << ", end=" << sentence.getEnd() << ", \"" << sentence.annotation.getCoveredText() << "\"" << ", firstToken="
					<< sentence.getFirstToken() << ", lastToken=" << sentence.getLastToken() << ")" << endl;
		}

	}

}

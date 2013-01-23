/*
 * ParagraphAnnotation.cpp
 *
 *  Created on: 14 janv. 2011
 *      Author: sylvainsurcin
 */

#include "ParagraphAnnotation.h"
#include <list>
#include <boost/foreach.hpp>

using namespace uima;
using namespace std;

namespace unitexcpp
{

	namespace annotation
	{

		Type ParagraphAnnotation::tParagraphAnnotation;
		Feature ParagraphAnnotation::fSentences;

		TyErrorId ParagraphAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			if (ContextAreaAnnotation::initializeTypeSystem(crTypeSystem) != UIMA_ERR_NONE)
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			tParagraphAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.ParagraphAnnotation");
			if (!tParagraphAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fSentences = tParagraphAnnotation.getFeatureByBaseName("sentences");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		ParagraphAnnotation::ParagraphAnnotation() :
				ContextAreaAnnotation()
		{

		}

		ParagraphAnnotation::ParagraphAnnotation(CAS& aCas, int begin, int end, const TokenAnnotation& firstToken, const TokenAnnotation& lastToken) :
				ContextAreaAnnotation(aCas)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tParagraphAnnotation, begin, end);
			setFirstToken(firstToken);
			setLastToken(lastToken);
			indexRep.addFS(annotation);
		}

		ParagraphAnnotation::ParagraphAnnotation(AnnotationFS& anAnnotation) :
				ContextAreaAnnotation(anAnnotation)
		{
		}

		ParagraphAnnotation::~ParagraphAnnotation()
		{
		}

		ParagraphAnnotation& ParagraphAnnotation::operator =(const ParagraphAnnotation& model)
		{
			pCas = model.pCas;
			annotation = model.annotation;
			return *this;
		}

		/*!
		 * Gets all the paragraph annotations in a view.
		 *
		 * \param aCas a view
		 * \param aContainer an STL container of ParagraphAnnotation where to store the paragraphs
		 */
		template<typename ParagraphContainer>
		void ParagraphAnnotation::getAllParagraphsInView(uima::CAS& aCas, ParagraphContainer& aContainer)
		{
			FSIterator iterator = aCas.getAnnotationIndex(tParagraphAnnotation).iterator();
			iterator.moveToFirst();
			while (iterator.isValid()) {
				AnnotationFS annotation = AnnotationFS(iterator.get());
				ParagraphAnnotation paragraph(annotation);
				aContainer.push_back(paragraph);
				iterator.moveToNext();
			}
		}

		ParagraphAnnotation ParagraphAnnotation::getParagraphCovering(const AnnotationWrapper& annotationWrapper, CAS& aCas)
		{
#ifdef _DEBUG
			cout << "getParagraphCovering annotation [" << annotationWrapper.getBegin() << "," << annotationWrapper.getEnd() << "]" << endl;
#endif
			list<ParagraphAnnotation> paragraphs;
			getAllParagraphsInView(aCas, paragraphs);

			int32_t annotationBegin = annotationWrapper.getBegin();
			int32_t annotationEnd   = annotationWrapper.getEnd();

			BOOST_FOREACH(ParagraphAnnotation& paragraph, paragraphs) {
#ifdef _DEBUG
				cout << "\ttesting paragraph [" << paragraph.getBegin() << "," << paragraph.getEnd() << "]" << endl;
#endif
				if ((paragraph.getBegin() <= annotationBegin) && (paragraph.getEnd() >= annotationEnd))
					return paragraph;
			}
			return ParagraphAnnotation();
		}

	}

}

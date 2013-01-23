/*
 * Token.cpp
 *
 *  Created on: 13 janv. 2011
 *      Author: sylvainsurcin
 */

#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning(disable:4005)
#endif

#include "TokenAnnotation.h"
#include "Utils.h"
#include "unicode/ustdio.h"

using namespace uima;
using namespace std;
using namespace icu;

namespace unitexcpp
{

	namespace annotation
	{

		uima::Type TokenAnnotation::tTokenAnnotation;
		uima::Feature TokenAnnotation::fToken;
		uima::Feature TokenAnnotation::fIndex;

		TyErrorId TokenAnnotation::initializeTypeSystem(TypeSystem const& crTypeSystem)
		{
			TyErrorId error = TextAreaAnnotation::initializeTypeSystem(crTypeSystem);
			if (error != UIMA_ERR_NONE)
				return error;

			tTokenAnnotation = crTypeSystem.getType("org.gramlab.kwaga.unitex_uima.unitex.tcas.TokenAnnotation");
			if (!tTokenAnnotation.isValid())
				return (TyErrorId) UIMA_ERR_RESMGR_INVALID_RESOURCE;

			fToken = tTokenAnnotation.getFeatureByBaseName("token");
			fIndex = tTokenAnnotation.getFeatureByBaseName("index");

			return (TyErrorId) UIMA_ERR_NONE;
		}

		/*!
		 * Default constructor builds an invalid instance.
		 */
		TokenAnnotation::TokenAnnotation() :
			TextAreaAnnotation()
		{
		}

		TokenAnnotation::TokenAnnotation(uima::CAS& aCas, int32_t begin, int32_t end, const UnicodeString& text, size_t nIndex) :
			TextAreaAnnotation(aCas)
		{
			FSIndexRepository& indexRep = aCas.getIndexRepository();
			annotation = aCas.createAnnotation(tTokenAnnotation, begin, end);
			token(text);
			setIndex(nIndex);
			indexRep.addFS(annotation);
		}

		TokenAnnotation::TokenAnnotation(AnnotationFS& anAnnotation) :
			TextAreaAnnotation(anAnnotation)
		{
		}

		TokenAnnotation::~TokenAnnotation()
		{
		}

		TokenAnnotation& TokenAnnotation::operator =(const TokenAnnotation& model)
		{
			this->TextAreaAnnotation::operator=(model);
			return *this;
		}

		UnicodeStringRef TokenAnnotation::token() const
		{
			return annotation.getStringValue(fToken);
		}

		void TokenAnnotation::token(const UnicodeString& text)
		{
			annotation.setStringValue(fToken, text);
		}

		size_t TokenAnnotation::getIndex() const
		{
			return annotation.getIntValue(fIndex);
		}

		void TokenAnnotation::setIndex(size_t n)
		{
			annotation.setIntValue(fIndex, n);
		}

		/*!
		 * Retrieves in a SOFA view, the TokenAnnotation starting at a given offset.
		 *
		 * \param offset an offset inside the view
		 * \param view a SOFA view containing offsets
		 * \return the TokenAnnotation whose annotation starts at this offset, or an invalid TokenAnnotation if not found
		 */
		TokenAnnotation TokenAnnotation::tokenStartingAtOffsetInView(uint32_t offset, CAS& view)
		{
			FSIterator iterator = view.getAnnotationIndex(tTokenAnnotation).iterator();
			iterator.moveToFirst();
			while (iterator.isValid()) {
				AnnotationFS annotation = AnnotationFS(iterator.get());
				if ((annotation.getBeginPosition() >= offset) || ((annotation.getBeginPosition() <= offset) && (annotation.getEndPosition() > offset)))
					return TokenAnnotation(annotation);
				iterator.moveToNext();
			}
			return TokenAnnotation();
		}

		/*!
		 * Retrieves in a SOFA view, the TokenAnnotation ending at a given offset.
		 *
		 * \param offset an offset inside the view
		 * \param view a SOFA view containing offsets
		 * \return the TokenAnnotation whose annotation ends at this offset, or an invalid TokenAnnotation if not found
		 */
		TokenAnnotation TokenAnnotation::tokenEndingAtOffsetInView(uint32_t offset, CAS& view)
		{
			FSIterator iterator = view.getAnnotationIndex(tTokenAnnotation).iterator();
			iterator.moveToLast();
			while (iterator.isValid()) {
				AnnotationFS annotation = AnnotationFS(iterator.get());
				if ((annotation.getEndPosition() <= offset) || ((annotation.getBeginPosition() <= offset) && (annotation.getEndPosition() >= offset))) {
					//cout << "return this token" << endl;
					return TokenAnnotation(annotation);
				}
				iterator.moveToPrevious();
			}
			return TokenAnnotation();
		}

		void TokenAnnotation::getTokensInView(CAS& view, list<TokenAnnotation>& tokens)
		{
			tokens.clear();
			FSIterator iterator = view.getAnnotationIndex(tTokenAnnotation).iterator();
			iterator.moveToFirst();
			while (iterator.isValid()) {
				AnnotationFS annotation = AnnotationFS(iterator.get());
				tokens.push_back(TokenAnnotation(annotation));
				iterator.moveToNext();
			}
		}

		ostream& operator <<(ostream& os, const TokenAnnotation& token)
		{
			return os << "TokenAnnotation(begin=" << token.getBegin() << ", end=" << token.getEnd()
					  << ", id=" << token.getIndex()
					  << ", form=\"" << token.token() << "\") over \""
					  << token.annotation.getCoveredText() << "\"";
		}

	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

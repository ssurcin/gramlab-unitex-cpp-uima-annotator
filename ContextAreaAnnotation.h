/*
 * ContextAreaAnnotation.h
 *
 *  Created on: 14 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef CONTEXTAREAANNOTATION_H_
#define CONTEXTAREAANNOTATION_H_

#include "TextAreaAnnotation.h"

namespace unitexcpp
{

	namespace annotation
	{

		class TokenAnnotation;

		class ContextAreaAnnotation: public unitexcpp::annotation::TextAreaAnnotation
		{
		protected:
			static uima::TyErrorId initializeTypeSystem(uima::TypeSystem const& crTypeSystem);

		protected:
			ContextAreaAnnotation();
			ContextAreaAnnotation(uima::CAS& aCas);
			ContextAreaAnnotation(uima::AnnotationFS fs);
			virtual ~ContextAreaAnnotation();

		public:
			ContextAreaAnnotation& operator =(const ContextAreaAnnotation& model);

			void setFirstToken(const TokenAnnotation& token);
			TokenAnnotation getFirstToken() const;

			void setLastToken(const TokenAnnotation& token);
			TokenAnnotation getLastToken() const;

			//void getCoveredText(icu::UnicodeString& ustring) const;

		private:
			static uima::Type tContextArea;
			static uima::Feature fFirstToken;
			static uima::Feature fLastToken;
		};

	}

}

#endif /* CONTEXTAREAANNOTATION_H_ */

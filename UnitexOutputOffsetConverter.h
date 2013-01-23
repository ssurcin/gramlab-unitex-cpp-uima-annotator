/*
 * UnitexOutputOffsetConverter.h
 *
 *  Created on: 18 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef UNITEXOUTPUTOFFSETCONVERTER_H_
#define UNITEXOUTPUTOFFSETCONVERTER_H_

#include "uima/api.hpp"

namespace unitexcpp
{

	/**
	 * The UnitexOutputOffsetConverter class helps converting the offsets provided by
	 * Unitex "UIMA" output into actual offsets inside the RMB or inside the whole body.
	 *
	 * Unitex "UIMA" offsets are counted in a version of the normalized source document
	 * (i.e. the RMB without blank line repetitions) <i>without</i> sentence marks "{S}"
	 * nor custom marks (e.g. "[SUBJECT]").
	 *
	 * @author surcin@kwaga.com
	 *
	 */
	class UnitexOutputOffsetConverter
	{
	public:
		typedef enum { REALMAILBODYVIEW, BODYVIEW } TargetView;

	public:
		UnitexOutputOffsetConverter(uima::UnicodeStringRef rustrNormalizedText, uima::CAS& aCas);
		virtual ~UnitexOutputOffsetConverter();

		size_t convert(size_t offset) const;

	private:
		uima::CAS& rmbView;
		int* offsets;
		std::size_t count;
	};

}

#endif /* UNITEXOUTPUTOFFSETCONVERTER_H_ */

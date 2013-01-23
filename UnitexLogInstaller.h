/*
 * UnitexLogInstaller.h
 *
 *  Created on: 28 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef UNITEXLOGINSTALLER_H_
#define UNITEXLOGINSTALLER_H_

#include <string>
#include "UniLogger.h"

namespace unitexcpp
{

	class UnitexLogInstaller
	{
	public:
		UnitexLogInstaller(bool bStoreFileOutContent = true);
		UnitexLogInstaller(std::string const & strPathLog, bool bStoreFileOutContent = true);
		virtual ~UnitexLogInstaller();

		int SelectNextLogName(std::string const & strLogName, std::string const & strPortionIgnorePathname);

	private:
		struct unitex::logger::UniLoggerSpace ule;
		int init_done;
	};

}

#endif /* UNITEXLOGINSTALLER_H_ */

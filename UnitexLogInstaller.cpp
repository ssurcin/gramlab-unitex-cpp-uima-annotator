/*
 * UnitexLogInstaller.cpp
 *
 *  Created on: 28 déc. 2010
 *      Author: sylvainsurcin
 */

#include "UnitexLogInstaller.h"
#include <string.h>
#include <stdlib.h>

using namespace std;

namespace unitexcpp
{

	UnitexLogInstaller::UnitexLogInstaller(bool bStoreFileOutContent)
	{
		init_done = 0;

		/* we want "mini log" with only list */

		ule.privateUnloggerData = NULL;
		ule.szPathLog = NULL;
		ule.store_file_out_content = bStoreFileOutContent ? 1 : 0;
		ule.store_list_file_out_content = 1;

		ule.store_file_in_content = 0 + 1;
		ule.store_list_file_in_content = 1;

		/* we dont want "auto generate" log */
		ule.auto_increment_logfilename = 0;

		if (AddActivityLogger(&ule) != 0)
			init_done = 1;
	}

	UnitexLogInstaller::UnitexLogInstaller(string const & strPathLog, bool bStoreFileOutContent)
	{
		init_done = 0;

		/* we want "mini log" with only list */

		ule.privateUnloggerData = NULL;
		ule.szPathLog = strdup(strPathLog.c_str());
		ule.store_file_out_content = bStoreFileOutContent ? 1 : 0;
		ule.store_list_file_out_content = 1;

		ule.store_file_in_content = 0 + 1;
		ule.store_list_file_in_content = 1;

		/* we dont want "auto generate" log */
		ule.auto_increment_logfilename = 0;

		if (AddActivityLogger(&ule) != 0)
			init_done = 1;
	}

	UnitexLogInstaller::~UnitexLogInstaller()
	{
		if (init_done != 0)
			RemoveActivityLogger(&ule);

		if (ule.szPathLog != NULL) {
			free((void *) ule.szPathLog);
			ule.szPathLog = NULL;
		}
	}

	int UnitexLogInstaller::SelectNextLogName(string const & strLogName, string const & strPortionIgnorePathname)
	{
		return unitex::logger::SelectNextLogName(&ule, strLogName.c_str(), strPortionIgnorePathname.c_str());
	}

}

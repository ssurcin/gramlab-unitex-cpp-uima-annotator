/*
 * UnitexException.cpp
 *
 *  Created on: 29 déc. 2010
 *      Author: sylvainsurcin
 */

#include "UnitexException.h"

using namespace std;

namespace unitexcpp
{

	UnitexException::UnitexException(const string& what_arg)
	: runtime_error(what_arg)
	{
	}

	UnitexException::~UnitexException() throw()
	{
	}

}

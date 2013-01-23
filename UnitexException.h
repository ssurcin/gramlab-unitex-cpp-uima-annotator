/*
 * UnitexException.h
 *
 *  Created on: 29 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef UNITEXEXCEPTION_H_
#define UNITEXEXCEPTION_H_

#include <stdexcept>

namespace unitexcpp
{

	class UnitexException: public std::runtime_error
	{
	public:
		explicit UnitexException(const std::string& what_arg);
		virtual ~UnitexException() throw();
	};

}

#endif /* UNITEXEXCEPTION_H_ */

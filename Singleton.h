/*
 * Singleton.h
 *
 *  Created on: 28 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include "Incopiable.h"
template<typename T>
class Singleton: Incopiable
{
protected:
	Singleton()
	{
	}
	virtual ~Singleton()
	{
	}

public:
	static T& getInstance()
	{
		static T theSingleInstance;
		return theSingleInstance;
	}
};

#endif /* SINGLETON_H_ */

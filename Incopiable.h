/*
 * Incopiable.h
 *
 *  Created on: 29 déc. 2010
 *      Author: sylvainsurcin
 */

#ifndef INCOPIABLE_H_
#define INCOPIABLE_H_

class __basevide
{
};

//
// Forme qui permet le Base Class Chaining
//
template<class B = __basevide> class Incopiable : B
{
	//
	// Bloquer la copie
	//
	Incopiable (const Incopiable &);
	Incopiable & operator= (const Incopiable &);
protected:
	//
	//
	// Permettre l'instanciation des enfants!
	//
	Incopiable () throw()
	{}
	~Incopiable () throw()
	{}
};

#endif /* INCOPIABLE_H_ */

/*
 * JavaLikeEnum.h
 *
 *  Created on: 3 janv. 2011
 *      Author: sylvainsurcin
 */

#ifndef JAVALIKEENUM_H_
#define JAVALIKEENUM_H_

#include <map>
#include <list>
#include <string>
#include <typeinfo>
#include <algorithm>

template<typename T>
class JavaLikeEnum
{
private:
	T value;
	typedef std::list<T> ARRAY;
	typedef typename std::map<std::string, typename JavaLikeEnum::ARRAY> MAP;
	//static typename JavaLikeEnum::MAP instances;

	static typename JavaLikeEnum::MAP& getInstances()
	{
		static typename JavaLikeEnum::MAP instances;
		return instances;
	}

protected:
	explicit JavaLikeEnum(const T& aValue)
	{
		value = aValue;
		typename MAP::iterator it = getInstances().find(typeid(T).name());
		if (it == getInstances().end()) {
			typename JavaLikeEnum::ARRAY array;
			array.push_back(aValue);
			getInstances()[typeid(T).name()] = array;
		}
		else {
			typename JavaLikeEnum::ARRAY& array = it->second;
			typename ARRAY::iterator it2 = std::find(array.begin(), array.end(), aValue);
			if (it2 == array.end()) {
				array.push_back(aValue);
			}
		}
	}
	virtual ~JavaLikeEnum()
	{
		typename MAP::iterator it = getInstances().find(typeid(T).name());
		if (it != getInstances().end()) {
			typename JavaLikeEnum::ARRAY& array = it->second;
			typename ARRAY::iterator it2 = std::find(array.begin(), array.end(), value);
			if (it2 != array.end()) {
				array.erase(it2);
			}
		}
	}

public:
	bool operator==(const JavaLikeEnum<T>& other) const
	{
		return (value == other.value);
	}
	bool operator!=(const JavaLikeEnum<T>& other) const
	{
		return (value != other.value);
	}
	bool operator<(const JavaLikeEnum<T>& other) const
	{
		typename MAP::iterator it = getInstances().find(typeid(T).name());
		if (it != getInstances().end()) {
			typename JavaLikeEnum::ARRAY& array = it->second;
			typename JavaLikeEnum::ARRAY::const_iterator it1 = std::find(array.begin(), array.end(), value);
			typename JavaLikeEnum::ARRAY::const_iterator it2 = std::find(array.begin(), array.end(), other.value);
			return (it1 < it2);
		}
		return false;
	}
	const T& getValue() const
	{
		return value;
	}

};

#endif /* JAVALIKEENUM_H_ */

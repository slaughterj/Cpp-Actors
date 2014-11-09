/*
 * Type.h
 *
 * Class for retrieving runtime ID numbers of types.
 * Used by Variant.h and Process.h.
 *
 * Created by Jason Slaughter 10 October 2014.
 */

#ifndef TYPE_H_
#define TYPE_H_

#include <type_traits>

namespace
{
    int& count()
    {
        static int count = 0;
        return count;
    }
}

template <typename T>
class Type
{
public:
    const unsigned int m_id;
    static const unsigned int Id;
    
    Type() : m_id(Type<T>::getId()) {} //= delete;
    Type(const Type& other) = delete;
    Type& operator=(const Type& other) = delete;

    static unsigned int getId();
};

template <typename T>
/*
 * For the purposes of this class, types with const specifier or references to type are
 * considered the same. Array types and pointer types are also equivalent.
 */
const unsigned int Type<T>::Id = Type<typename std::decay<T>::type>::getId();

template <typename T>
unsigned int Type<T>::getId()
{
    static unsigned int counter = 0;
    if(counter == 0)
    {
        counter = ++count();
        return counter;
    }
    else
    {
        return counter;
    }
};

#endif

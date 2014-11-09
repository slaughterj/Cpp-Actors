/*
 * Variant.h
 *
 * Variant class that can store any C++ type and holds it's runtime id.
 * Needed by Process.h.
 *
 * Created by Jason Slaughter 10 October 2014.
 */

#ifndef VARIANT_H_
#define VARIANT_H_

#include <cstdio>
#include <cstdlib>
#include "Type.h"

class Variant
{
private:
    const void* data; 
    unsigned int m_id;

public:
    Variant() = default;

    template <typename T>
    Variant(T&& t) : data(&t), m_id(Type<T>::Id)
    {
    }

     template<typename T>
     const T& getMember()   
     {
         if (Type<T>::Id != m_id)
         {
             fprintf(stderr, "Wrong type for Variant\n");
             exit(1);
         }
         return *(T*)data;
     }
	 
	 unsigned int getId() const
	 {
		 return m_id;
	 }

     template <typename T>
     /*explicit*/ operator T() const
     {
         if(Type<T>::Id != m_id)
         {
             fprintf(stderr, "Wrong type for Variant\n");
             exit(1);
         }
         return *(T*)data;
     }   

    ~Variant()
    {
    }
};

#endif

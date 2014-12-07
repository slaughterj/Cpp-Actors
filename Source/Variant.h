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
namespace proc
{
enum class PROC_CONTROL : unsigned int;
}

class Variant
{
private:
    const void* data; 
    unsigned int m_id;
    proc::PROC_CONTROL m_control;

public:
    Variant() = default;

    Variant(Variant& other) : data(other.data), m_id(other.m_id)
    {
    }

    Variant& operator=(const Variant& other) = default;

    Variant(std::nullptr_t np) : data(nullptr), m_id(Type<std::nullptr_t>::Id)
    {
    }

    Variant(proc::PROC_CONTROL control) : data(nullptr), m_id(Type<proc::PROC_CONTROL>::Id)
    {
    }

    template <typename T>
    Variant(T t) : data(new T(t)), m_id(Type<T>::Id)
    {
    }

    template <typename T>
    Variant(T* t) : data(&t), m_id(Type<T*>::Id)
    {
    }

     template<typename T>
     const T& get() const  
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

    ~Variant()
    {
    }
};

#endif

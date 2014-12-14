Actor
=====

A lightweight C++ actor implementation inspired by Erlang.

###Example

```C++

#include "Process.h"
#include <tuple>

using std::tuple;
using std::get;
using std::make_tuple;
using namespace proc;

typedef tuple<ProcessId, ProcessId> idTuple;

void function1(const Variant& var, ProcessId self)
{
	int num = var.get<int>();
	printf("Hi from function1. I got an int '%d'. It's type id is: %d\n", num, Type<int>::Id);
}

void function2(const Variant& var, ProcessId self)
{
	char ch = var.get<char>();
	printf("Hi from function2. I got a char '%c'. It's type id is: %d\n", ch, Type<char>::Id);
}

void function3(const Variant& var, ProcessId self)
{
	idTuple ids = var.get<idTuple>();

	printf("Hi from function3. I got a tuple with 2 ProcessId's. It's type id is: %d\n",
        Type<idTuple>::Id);
		
	auto id1 = get<0>(ids);	
	auto id2 = get<1>(ids);	
	
	sendMsg(id1, PROC_CONTROL::KILL);
	sendMsg(id2, PROC_CONTROL::KILL);	
	sendMsg(self, PROC_CONTROL::KILL);		
}

int main()
{
	auto id1 = spawn([] { printf("This is an init function.\n"); }, {{Type<int>::Id, function1}});
	auto id2 = spawn({{Type<char>::Id, function2}});
	auto id3 = spawn({{Type<int>::Id, function1}, {Type<char>::Id, function2}, {Type<idTuple>::Id, function3}});
	idTuple ids(id1, id2);
	
	sendMsg(id1, 12);
	sendMsg(id2, 'C');
	sendMsg(id3, 'Z');
	sendMsg(id3, 500);
	
	sendMsg(id3, ids); 
	
	return 0;
} 
```

###Notes
	Although the implementation itself is lightweight, the actors themselves are not, that is their underlying
implementation uses OS threads.
	
###Requirements
	The code uses several features of C++11 such as variadic templates and lambda functions. The current releases
of Clang, Visual Studio and GCC will compile this just fine.

# NTSerialize library

This library can help you to save your C++ programm data into compact binary file. Just follow the steps below:

Include the header file:

```cpp
#include "NTSerialize.hpp"
```

Define mutex for thread-safe console output:

```cpp
std::mutex console_mtx;
```

Create NTSerialize:

```cpp
NTSerialize NTS( console_mtx );
```

Write something:

```cpp
NTS << my_data;
```

And save it into file:

```cpp
NTS.save( "data.bin" );
```

If you want to read data from a file:

```cpp
NTSerialize NTS( console_mtx );
NTS.load( "data.bin" );
NTS >> my_data;
```

For more control see the source code.

Also, you can write your own structures. For example, you have something like this:

```cpp
struct MyStruct {
	unsigned int x1;
	unsigned int x2;
	std::fstream fs;
};
```

Add two operators into your class:

```cpp
struct MyStruct {
	unsigned int x1;
	unsigned int x2;
	std::fstream fs;
	
	friend NTSerialize& operator<<( NTSerialize& bnz,
									const MyStruct& ms ) {
		bnz << ms.x1 << ms.x2;
		return( bnz );
	}
	friend NTSerialize& operator>>( NTSerialize& bnz,
									MyStruct& ms ) {
		bnz >> ms.x1 >> ms.x2;
		return( bnz );
	}
};
```

Then you can easily write it:

```cpp
MyStruct st;
NTS << st;
```

Or read:

```cpp
MyStruct st;
NTS >> st;
```

# Compilation:

```bash
g++ -std=c++14 -m64 -O2 NTSerialize_test.cpp -o NTStest
```

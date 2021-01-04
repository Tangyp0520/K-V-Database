#pragma once
#include<iostream>
#include<list>
#include"DataStruct.h"
#include"ErrorReturnValue.h"
using namespace std;
#define HASHMAX 127

class Hash
{
public:
	list<HashDataNode> root[HASHMAX];
	~Hash();
	int GetHashCode(string str);
	void set(string key, int offset);
	int get(string key);
	void clear();
};
#pragma once
#include<iostream>
#include<list>
using namespace std;
#define HASHMAX 127

struct HashDataNode
{
	string key = "";
	int offset = 0;
};

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


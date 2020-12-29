#pragma once
#include<iostream>
using namespace std;
//Hash
struct HashDataNode
{
	string key = "";
	int offset = 0;
};
//MinHeap
struct TimeNode
{
	string key = "";
	int time = 0;
};
struct cmp
{
	bool operator()(const TimeNode& a, const TimeNode& b)
	{
		return a.time > b.time;
	}
};
//KVDBHandler
struct KVDBData
{
	int keyLen = 0;
	int valueLen = 0;
	string key = "";
	string value = "";
	void set(int keyLen1, int valueLen1, string key1, string value1)
	{
		keyLen = keyLen1; valueLen = valueLen1;
		key = key1; value = value1;
	}
};

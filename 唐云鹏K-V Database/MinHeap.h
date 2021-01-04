#pragma once
#include<iostream>
#include<fstream>
#include<ctime>
#include<queue>
#include"Hash.h"
#include"DataStruct.h"
#include"ErrorReturnValue.h"
using namespace std;

class MinHeap
{
private:
	priority_queue<TimeNode, vector<TimeNode>, cmp> minHeap;
	Hash index;
	string MinHeap_filename;
public:
	MinHeap();
	~MinHeap();
	void reset();
	void set(string key, int time);
	bool get(string key);
	void del();
};
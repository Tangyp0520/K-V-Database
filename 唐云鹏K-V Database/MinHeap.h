#pragma once
#include<iostream>
#include<fstream>
#include<ctime>
#include<queue>
#include"ErrorReturnValue.h"
using namespace std;
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
class MinHeap
{
private:
	priority_queue<TimeNode, vector<TimeNode>, cmp> minHeap;
	string MinHeap_filename;
public:
	MinHeap();
	~MinHeap();
	void reset();
	void set(string key, int time);
	int get(string key);
	void del(string key);
};


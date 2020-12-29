#pragma once
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include"ErrorReturnValue.h"
#include"Hash.h"
#include"MinHeap.h"
#include"DataStruct.h"

class KVDBHandler
{
public:
	string KVDB;
	Hash index;
	MinHeap minHeap;
	int offset;

	KVDBHandler(const string& db_file);
	~KVDBHandler();
	void resetIndex();
	void readKVDBData(fstream& file, KVDBData& s);
	void writeKVDBData(fstream& file, KVDBData& s);
	friend int set(KVDBHandler* handler, const string& key, const string& value);
	friend int get(KVDBHandler* handler, const string& key, string& value);
	friend int del(KVDBHandler* handler, const std::string& key);
	friend int purge(KVDBHandler* handler);
	friend int purgeSubfunction(KVDBHandler* handler, KVDBHandler* save_handler);
	friend int expires(KVDBHandler* handler, const string key, int n);
};


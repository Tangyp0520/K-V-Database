#pragma once
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include"ErrorReturnValue.h"
#include"Hash.h"

struct KVDBData
{
	int keyLen = 0;
	int valueLen = 0;
	string key = "";
	string value = "";
	int del = 0;
	int time = 0;
};

class KVDBHandler
{
public:
	string KVDB;
	Hash index;
	int offset;

	KVDBHandler(const string& db_file);
	~KVDBHandler();
	long long getLength();
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


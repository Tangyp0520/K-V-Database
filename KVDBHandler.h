#pragma once
#include<iostream>
#include<fstream>
#include<cstdlib>
#include"AbnomalReturn.h"
#include"Hash.h"
using namespace std;
class KVDBHandler
{
public:
	string KVDB;
	Hash index;
	int offset;
	KVDBHandler(const string& db_file);
	~KVDBHandler();
	void resetIndex();
	friend int set(KVDBHandler* handler, const string& key, const string& value);
	friend int get(KVDBHandler* handler, const string& key, string& value);
	friend int del(KVDBHandler* handler, const std::string& key);
	friend int purge(KVDBHandler* handler);
	friend int purgeSubfunction(KVDBHandler* handler, KVDBHandler* save_handler);
	friend string char_to_string(char* str, int n);
	friend char* int_to_char(int n);
	friend int char_to_int(char* ch);
};
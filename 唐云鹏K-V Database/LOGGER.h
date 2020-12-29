#pragma once
#include<iostream>
#include<cstring>
#include<fstream>
#include<cstdlib>
using namespace std;
class LOGGER
{
public:
	string file_name;
	LOGGER();
	~LOGGER();
	void Push();
	void set();
	void get();
	void del();
	void purge();
	void pergeSubfunctioon();
	void HashGetHashCode();
};


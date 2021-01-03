#pragma once
# include <iostream>
# include <fstream>
# include <string>
# include <time.h>
# include <stdio.h>
# include <stdlib.h>
#include"ErrorReturnValue.h"
#pragma warning(disable : 4996)
using namespace std;
#define Debug 1
#define Error 0
class LOGGER
{
private:
    ofstream outfile;
    string path;
    void output(string text, int act_level);
public:
    LOGGER(string path);
    void DEBUG(string text);
    void ERROR(string text);
    string getAns(int flag);
    string currTime();
};
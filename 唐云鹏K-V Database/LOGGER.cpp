#include "LOGGER.h"

LOGGER::LOGGER(string path)
{
    this->path = path;
    string tmp = "";
    string welcome_dialog = tmp + "[Welcome] " + __FILE__ + " " + currTime() + " : " + "=== Start logging ===\n";
    this->outfile.open(path, ios::out | ios::app);
    this->outfile << welcome_dialog;
}
void LOGGER::output(string text, int act_level)
{
    string prefix;
    if (act_level == Debug) prefix = "[DEBUG]   ";
    else if (act_level == Error) prefix = "[ERROR]   ";
    else prefix = "";
    prefix += __FILE__;
    prefix += " ";
    string output_content = prefix + currTime() + " : " + text + "\n";
    outfile << output_content;
}
void LOGGER::DEBUG(string text) 
{
    this->output(text, Debug);
}
void LOGGER::ERROR(string text) 
{
    this->output(text, Error);
}

string LOGGER::getAns(int flag)
{
    string ans;
    if (flag == SUCCESS)
        ans = "Success!";
    else if (flag == FAIL)
        ans = "Failed! ";
    else if (flag == KEY_NOT_EXIST)
        ans = "Failed! Key does not exist";
    else if (flag == KEY_HAS_BEEN_DELETED)
        ans = "Failed! Key has been deleted";
    else if (flag == ERROR_EXECUTING_SUBFUNCTION)
        ans = "Failed! Purge subroutine failed";
    else if (flag == OVERDUE_KEY)
        ans = "Failed! Key has expireed";
    return ans;
}
string LOGGER::currTime()
{
    char tmp[64];
    time_t ptime;
    time(&ptime);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&ptime));
    return tmp;
}
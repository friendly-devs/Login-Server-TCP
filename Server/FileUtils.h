#pragma once
#include <list>
#include <fstream>
#include <string>
#include "User.h"

#define FILE_NAME "account.txt"

using namespace std;

#define LOGIN_SUCCESS 0
#define LOGIN_FAILED 1
#define LOGIN_OVER_TIME 2

void saveToFile(std::list<User>);
std::list<User> readUsersFromFile();

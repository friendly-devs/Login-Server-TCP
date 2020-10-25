#pragma once
#define LENGHT 50
#define STATUS_ACTIVE 0
#define STATUS_INACTIVE 1

struct User
{
	char userId[LENGHT];
	char password[LENGHT];
	int status;
	int countLoginFailed;
};
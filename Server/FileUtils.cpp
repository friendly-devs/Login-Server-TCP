#include "FileUtils.h"

void saveToFile(std::list<User> users)
{
    list<User>::iterator user = users.begin();
    ofstream oFile(FILE_NAME);

    while (user != users.end())
    {
        oFile << user->userId;
        oFile << " ";
        oFile << user->password;
        oFile << " ";
        oFile << user->status;

        user++;
        if (user != users.end())
        {
            oFile << endl;
        }
    }

    oFile.close();
}

std::list<User> readUsersFromFile()
{
    list<User> users;
    ifstream iFile(FILE_NAME);

    if (!iFile.is_open())
    {
        printf("Unable open file!\n");
        return std::list<User>();
    }

    const int MAX_LENGTH = 256;
    string line;

    while (std::getline(iFile, line))
    {
        User user;
        const char *data = line.c_str();

        // copy userId
        int i = 0;
        while (data[i] != ' ')
        {
            user.userId[i] = data[i];
            i++;
        }
        user.userId[i] = '\0';

        // copy password
        i++;
        int offset = i;

        while (data[i] != ' ')
        {
            user.password[i - offset] = data[i];
            i++;
        }
        user.password[i - offset] = '\0';

        // copy status
        i++;
        user.status = data[i] - '0';

        user.countLoginFailed = STATUS_ACTIVE;

        users.push_back(user);
    }

    iFile.close();

    return users;
}


﻿#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "FileUtils.h"

#pragma comment (lib, "Ws2_32.lib")

// Định nghĩa message
#define DEFAULT_BUFLEN 1024
#define RESPOND_INCORRECT_PASSWORD "Incorrect password"
#define RESPOND_SUCCESS "Login success"
#define RESPOND_BLOCKED_USER "User was blocked"
#define RESPOND_USERID_OR_PASSWORD_FAILD "UserId or password incorrect"
#define RESPOND_EXIT "exit"

// Khai báo list<User>
list<User> users;

// Hàm kiểm tra đăng nhập
const char* vetifyLogin(const char* data);

// Hàm login, sẽ được chạy trong thread
DWORD WINAPI doLogin(LPVOID lpParam);

//int main(int argc, char** argv)
//{
//    // validate argument
//    if (argc != 2) {
//        printf("Please enter the port\n");
//        return 1;
//    }
//
//    // import data from file
//    users = readUsersFromFile();
//    if (users.empty())
//    {
//        printf("File is empty\n");
//        return 1;
//    }
//
//    printf("Server is running in port %s\n", argv[1]);
//
//    WSADATA wsaData;
//    int iResult;
//
//    SOCKET ListenSocket = INVALID_SOCKET;
//
//    struct addrinfo* result = NULL;
//    struct addrinfo hints;
//
//    // Initialize Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        printf("WSAStartup failed with error: %d\n", iResult);
//        return 1;
//    }
//
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//    hints.ai_flags = AI_PASSIVE;
//
//    // Resolve the server address and port
//    iResult = getaddrinfo(NULL, argv[1], &hints, &result);
//    if (iResult != 0) {
//        printf("getaddrinfo failed with error: %d\n", iResult);
//        WSACleanup();
//        return 1;
//    }
//
//    // Create a SOCKET for connecting to server
//    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//    if (ListenSocket == INVALID_SOCKET) {
//        printf("socket failed with error: %ld\n", WSAGetLastError());
//        freeaddrinfo(result);
//        WSACleanup();
//        return 1;
//    }
//
//    // Setup the TCP listening socket
//    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//    if (iResult == SOCKET_ERROR) {
//        printf("bind failed with error: %d\n", WSAGetLastError());
//        freeaddrinfo(result);
//        closesocket(ListenSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    freeaddrinfo(result);
//
//    iResult = listen(ListenSocket, SOMAXCONN);
//    if (iResult == SOCKET_ERROR) {
//        printf("listen failed with error: %d\n", WSAGetLastError());
//        closesocket(ListenSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    DWORD thread;
//    SOCKET clientSocket = INVALID_SOCKET;
//
//    while (true)
//    {
//        // Accept a client socket
//        clientSocket = accept(ListenSocket, NULL, NULL);
//
//        // Sau khi accept một client thì tạo một thread
//        CreateThread(NULL, 0, doLogin, (LPVOID)clientSocket, 0, &thread);
//    }
//
//    closesocket(ListenSocket);
//    WSACleanup();
//
//    return 0;
//}

const char* vetifyLogin(const char* data) {
    const int LENGTH = 256;
    char userId[LENGTH];
    char password[LENGTH];

    // Tách userId
    int i = 0;
    while (data[i] != ',')
    {
        userId[i] = data[i];
        i++;
    }
    userId[i] = '\0';

    // Tách password
    i++;
    int offset = i;
    while (data[i] != '\0')
    {
        password[i - offset] = data[i];
        i++;
    }
    password[i - offset] = '\0';

    // Duyệt trong list users
    std::list<User>::iterator pUser = users.begin();
    while (pUser != users.end())
    {
        if (strcmp(pUser->userId, userId) == 0)
        {
            // Nếu user đã bị block thì trả về message tương ứng
            if (pUser->status == STATUS_INACTIVE)
            {
                return RESPOND_BLOCKED_USER;
            }

            // Nếu password đúng thì trả về đăng nhập thành công
            if (strcmp(pUser->password, password) == 0)
            {
                return RESPOND_SUCCESS;
            }
            
            // Nhập password sai
            // Tăng biến đếm nhập password sai lên
            pUser->countLoginFailed++;

            if (pUser->countLoginFailed >= 3)
            {
                // Nếu từ 3 lần đổ lên thì khóa user, lưu lại vào file
                pUser->status = STATUS_INACTIVE;
                
                saveToFile(users);
                return RESPOND_BLOCKED_USER;
            }
            else
            {
                return RESPOND_INCORRECT_PASSWORD;
            }
        }
        
        pUser++;
    }

    // Nếu userId không trùng thì trả về mesage tương ứng
    return RESPOND_USERID_OR_PASSWORD_FAILD;
}

DWORD WINAPI doLogin(LPVOID lpParam)
{
    // Nhận lại socket
    SOCKET clientSocket = (SOCKET)lpParam;

    if (clientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        ExitThread(1);
    }

    char recvbuf[DEFAULT_BUFLEN + 1];
    int recvbuflen = DEFAULT_BUFLEN;
    int count = 0;
    int maxCount = 3;
    bool isLoginSuccess = false;

    while (true)
    {
        if (count >= maxCount)
        {
            break;
        }
        count++;

        // Nhận dữ liệu từ client
        int bytesReceived = recv(clientSocket, recvbuf, recvbuflen, 0);

        if (bytesReceived == SOCKET_ERROR) {
            break;
        }

        // Chèn kí tự kết thúc string vào cuối xâu
        if (bytesReceived < recvbuflen) {
            recvbuf[bytesReceived] = '\0';
        }

        // Repond the buffer back to the sender
        const char* sendBuf = vetifyLogin(recvbuf);
        send(clientSocket, sendBuf, strlen(sendBuf), 0);
        
        // Nếu login thành công thì thoát khỏi vòng đăng nhập
        if (0 == strcmp(sendBuf, RESPOND_SUCCESS))
        {
            isLoginSuccess = true;
            //printf("login success: %s\n", recvbuf);
            break;
        }
    }

    // Chờ nhận mesage đăng xuất từ client
    while (isLoginSuccess)
    {
        int bytesReceived = recv(clientSocket, recvbuf, recvbuflen, 0);

        if (bytesReceived == SOCKET_ERROR) {
            break;
        }

        if (bytesReceived < recvbuflen) {
            recvbuf[bytesReceived] = '\0';
        }

        // Kiểm tra message có phải là đăng xuất không
        if (0 == strcmp(recvbuf, RESPOND_EXIT))
        {
            isLoginSuccess = false;
            //printf("logout: %s\n", recvbuf);
        }
    }

    // Đóng socket
    closesocket(clientSocket);
    // Thoát khỏi thread
    ExitThread(0);
}

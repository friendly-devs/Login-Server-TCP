#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 1024
#define RESPOND_SUCCESS "Login success"
#define RESPOND_EXIT "exit"

// argv[1] ip of server
// argv[2] port of server
int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Please enter ip and port server\n");
        return 1;
    }

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN + 1];
    int recvbuflen = DEFAULT_BUFLEN;
    
    int countLogin = 3;
    const int LENGTH_BUF = 256;
    char userId[LENGTH_BUF];
    char password[LENGTH_BUF];

    bool isLoginSuccess = false;

    // chạy vòng lặp để user nhập pass và id
    for(int i=0; i<countLogin; i++) {
        printf("Enter the userId: ");
        fgets(userId, LENGTH_BUF, stdin);
        userId[strlen(userId) - 1] = '\0';

        printf("Enter the password: ");
        fgets(password, LENGTH_BUF, stdin);
        password[strlen(password) - 1] = '\0';

        // send data to server
        strcpy_s(sendbuf, userId);
        strcat_s(sendbuf, ",");
        strcat_s(sendbuf, password);

        // Send an initial buffer
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            printf("Send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        int bytesReceived = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        if (bytesReceived < recvbuflen) {
            recvbuf[bytesReceived] = '\0';
        }

        printf("%s\n", recvbuf);

        if (0 == strcmp(recvbuf, RESPOND_SUCCESS)) {
            isLoginSuccess = true;
            break;
        }
    }

    int answer;

    while (isLoginSuccess)
    {
        printf("Are you log out?\n");
        printf("1. Yes\n");
        printf("2. No\n");
        printf("Enter answer: ");

        scanf_s("%d", &answer);

        if (1 == answer)
        {
            strcpy_s(sendbuf, RESPOND_EXIT);
            send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            isLoginSuccess = false;
        }
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
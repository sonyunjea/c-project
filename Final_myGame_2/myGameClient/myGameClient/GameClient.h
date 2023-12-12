//GameClient.h 파일
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

char getKeyPress() {
    if (_kbhit()) return _getch();
    return 0;
}

void clearCursor() {
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1; //커서 굵기 (1 ~ 100)
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

typedef enum {
    EMPTY,
    WALL,

    OCCUPATING_P1,
    OCCUPATING_P2,

    PLACE_P1,
    PLACE_P2,

    ENTER_P1,
    ENTER_P2,

    EXIT_P1,
    EXIT_P2
}MAPSTATE;

class GameClient {
public:
    GameClient(const char* serverIp, int serverPort) : serverIp(serverIp), serverPort(serverPort) {
        // Windows 소켓 초기화
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock\n";
            exit(EXIT_FAILURE);
        }

        // 소켓 초기화
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket\n";
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // 서버 주소 설정
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);

        if (inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr)) <= 0) {
            std::cerr << "Invalid address/ Address not supported\n";
            closesocket(clientSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // 서버에 연결
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error connecting to server\n";
            closesocket(clientSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        std::cout << "서버와 연결되었습니다. 상대방을 기다리는 중..." << std::endl;

    }
    ~GameClient() {}

    void start() {
        system("cls");
        while (true) {

            std::string s;

            // 키 입력
            if (_kbhit()) {
                s = _getch();
                
                if ((int)s[0] < 0) s = _getch();
                
                sendMessage(s);
            }
          
            char buf[1024];

            // 상대 visible 받기
            int visible;
            recv(clientSocket, buf, sizeof(int), 0);
            visible = (int)buf[0];

            // 모드 정보 받기
            int mode;
            recv(clientSocket, buf, sizeof(int), 0);
            mode = (int)buf[0];

            // 맵 정보 받기
            int rows, cols;
            recv(clientSocket, buf, sizeof(int), 0);
            rows = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            cols = (int)buf[0];

            std::vector<std::vector<int>> map(rows, std::vector<int>(cols, 0));
            
            for (int i = 0; i < 12; i++) {
                for (int j = 0; j < 12; j++) {
                    recv(clientSocket, buf, sizeof(int), 0);
                    map[i][j] = (int)buf[0];
                }
            }

            // 플레이어 정보 받기
            int playerX, playerY, playerIndex, playerState;
            std::string msg = "";

            recv(clientSocket, buf, sizeof(int), 0);
            playerX = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            playerY = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            playerState = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            playerIndex = (int)buf[0];

            // 상대 플레이어 정보 받기
            int enemyX, enemyY, enemyIndex, enemyState;
            recv(clientSocket, buf, sizeof(int), 0);
            enemyX = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            enemyY = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            enemyState = (int)buf[0];
            recv(clientSocket, buf, sizeof(int), 0);
            enemyIndex = (int)buf[0];

            // 입출구 정보 받기
            int size;
            recv(clientSocket, buf, sizeof(int), 0);
            size = (int)buf[0];

            std::vector<std::vector<int>> enter;
            
            for (int i = 0; i < size; i++) {
                int y, x;
                recv(clientSocket, buf, sizeof(int), 0);
                y = (int)buf[0];
                recv(clientSocket, buf, sizeof(int), 0);
                x = (int)buf[0];

                enter.push_back({ y, x });
            }

            recv(clientSocket, buf, sizeof(int), 0);
            size = (int)buf[0];

            std::vector<std::vector<int>> exit;

            for (int i = 0; i < size; i++) {
                int y, x;
                recv(clientSocket, buf, sizeof(int), 0);
                y = (int)buf[0];
                recv(clientSocket, buf, sizeof(int), 0);
                x = (int)buf[0];

                exit.push_back({ y, x });
            }

            printMap(map, 
                playerX, playerY, playerIndex, 
                enemyX, enemyY, enemyIndex,
                enter, 
                exit,
                mode, visible);

            gotoxy(0, 20);
            std::cout << "                                                                                                                  ";
    
            // 게임 메세지 수신
            recv(clientSocket, buf, 1024, 0);
            msg.copy(buf, 1024);
            gotoxy(0, 20);
            std::cout << buf;

            // 종료조건
            if (mode == 3) {
                break;
            }

        }

        // 클라이언트 소켓 종료
        closesocket(clientSocket);
        WSACleanup();
    }

    void printMap(std::vector<std::vector<int>> map, 
        int playerX, int playerY, int playerIndex, 
        int enemyX, int enemyY, int enemyIndex,
        std::vector<std::vector<int>> enter, 
        std::vector<std::vector<int>> exit,
        int mode, int visible) {

        clearCursor();
        gotoxy(0, 0);

        int i = 0, j = 0;
        for (const auto& row : map) {
            j = 0;
            for (int cell : row) {
                char ch = ' ';
                switch (cell) {
                case MAPSTATE::WALL: ch = '*'; break;

                case MAPSTATE::PLACE_P1: 
                    if (playerIndex == 0) ch = 'a'; 
                    if (enemyIndex == 0 && visible) ch = 'a';
                    break;
                case MAPSTATE::PLACE_P2: 
                    if (playerIndex == 1) ch = 'b'; 
                    if (enemyIndex == 1 && visible) ch = 'b';
                    break;

                }

                for (auto& e : enter) {
                    if (e[0] == i && e[1] == j)
                        ch = 'i';
                }

                for (auto& e : exit) {
                    if (e[0] == i && e[1] == j)
                        ch = 'o';
                }

                if (i == playerY && j == playerX) {
                    ch = 'A' + playerIndex;
                }

                if (i == enemyY && j == enemyX) {
                    if (mode != 2 && visible) ch = 'A' + enemyIndex;

                    else if (mode == 2) {
                        if (playerX == enemyX) {
                            ch = 'A' + enemyIndex;
                        }
                        else if (playerY == enemyY) {
                            ch = 'A' + enemyIndex;
                        }
                        else if (playerX - 1 <= enemyX && enemyX <= playerX + 1 && playerY - 1 <= enemyY && enemyY <= playerY + 1) {
                            ch = 'A' + enemyIndex;
                        }
                    }
                }
                    
                std::cout << ch;
                j++;
            }
            i++;
            std::cout << '\n';
        }

    }

private:
    const char* serverIp;
    int serverPort;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    WSADATA wsaData;

    void sendMessage(const std::string& message) {
        int bytesSent = send(clientSocket, message.c_str(), message.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending message to server\n";
        }
    }

};

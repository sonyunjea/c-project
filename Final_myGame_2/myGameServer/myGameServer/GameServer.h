//GameServer.h ����

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <list>

#include "GameInfo.h"

#pragma comment(lib, "ws2_32.lib")

class GameServer {

private:

    int port;

    SOCKET serverSocket;
    sockaddr_in serverAddr;
    WSADATA wsaData;

    std::list<SOCKET> clientSockets;

    GameInfo game;
    std::string msg = "";

public:

    GameServer(int port) : port(port) {
        // Windows ���� �ʱ�ȭ
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock\n";
            exit(EXIT_FAILURE);
        }

        // ���� �ʱ�ȭ
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket\n";
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // ���� �ּ� ����
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        // ���ε�
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error binding socket\n";
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // ������
        if (listen(serverSocket, 2) == SOCKET_ERROR) {
            std::cerr << "Error listening on socket\n";
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        std::cout << "Server is listening on port " << port << std::endl;

    }
    ~GameServer() {
        game.~GameInfo();
    }

    void start() {

        // ���� �̺�Ʈ ����
        WSAEVENT serverEvent = WSACreateEvent();
        if (serverEvent == WSA_INVALID_EVENT) {
            std::cerr << "Error creating event\n";
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // ���� �̺�Ʈ ����
        if (WSAEventSelect(serverSocket, serverEvent, FD_ACCEPT) == SOCKET_ERROR) {
            std::cerr << "Error associating event with socket\n";
            WSACloseEvent(serverEvent);
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Ŭ���̾�Ʈ ���Ӻ�
        while (clientSockets.size() < 2) {
            
            DWORD index = WSAWaitForMultipleEvents(1, &serverEvent, FALSE, WSA_INFINITE, FALSE);
            if (index == WSA_WAIT_FAILED) {
                std::cerr << "Error waiting for events\n";
                break;
            }

            // ���ο� Ŭ���̾�Ʈ�� ���� ����
            if (index == WSA_WAIT_EVENT_0) {
                
                WSANETWORKEVENTS networkEvents;
                if (WSAEnumNetworkEvents(serverSocket, serverEvent, &networkEvents) == SOCKET_ERROR) {
                    std::cerr << "Error enumerating network events\n";
                    break;
                }

                if (networkEvents.lNetworkEvents & FD_ACCEPT) {
                    if (networkEvents.iErrorCode[FD_ACCEPT_BIT] == 0 && clientSockets.size() < 2) {
                        int clientSocket = accept(serverSocket, nullptr, nullptr);
                        if (clientSocket == INVALID_SOCKET) {
                            std::cerr << "Error accepting client connection\n";
                        }
                        else {
                            clientSockets.push_back(clientSocket);
                            std::cout << " �÷��̾� " << clientSockets.size() << " / 2 ���� �Ϸ�" << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Error in FD_ACCEPT: " << networkEvents.iErrorCode[FD_ACCEPT_BIT] << std::endl;
                    }
                }
            }

        }

        
        // ON OFF ����
        char visible;

        while (true) {

            std::string answer;
            
            std::cout << " �÷��̾� �� visible ���� (on/off)" << std::endl;
            std::cout << " >> "; std::cin >> answer;

            if (answer.compare("on") == 0) {
                visible = 1;
                break;
            }
            else if (answer.compare("off") == 0) {
                visible = 0;
                break;
            }

            std::cout << " �߸��� �Է��Դϴ�. " << std::endl;
            system("pause");
            system("cls");

        }

        system("cls");

        // ���� ����
        while (true) {
            // Ŀ�� ����
            clearCursor();
            // �� ���
            game.printGame();

            // ������ ���ź�
            int playerIndex = 0;
            for (auto& clientSocket : clientSockets) {

                char buffer[1024];
                std::memset(buffer, 0, sizeof(buffer));

                int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                
                if (bytesRead > 0) {

                    std::string s = " ";

                    switch ((int)buffer[0]) {
                    case 72: s = "ȭ��ǥ ��"; game.playerMove(playerIndex, 72);  break;
                    case 77: s = "ȭ��ǥ ������"; game.playerMove(playerIndex, 77); break;
                    case 80: s = "ȭ��ǥ �Ʒ�"; game.playerMove(playerIndex, 80); break;
                    case 75: s = "ȭ��ǥ ����"; game.playerMove(playerIndex, 75); break;
                    case 32: s = "�����̽���"; game.playerAction(playerIndex, 32);  break;
                    default: s = buffer[0]; game.playerAction(playerIndex, (int)buffer[0]);
                    }

                    gotoxy(0, 15 + playerIndex);
                    std::cout << (int)buffer[0] << "                                                                       " << std::endl;
                    gotoxy(0, 15 + playerIndex);
                    std::cout << playerIndex + 1 << "�� �÷��̾ " << s << "Ű�� �������ϴ�. " << std::endl;
                
                }
                playerIndex++;
            }
            
            if (game.getMode() == MODE::AREACAPTURE) {
                msg = "���� �������� ����Դϴ�.";
            }

            // �Ѱ��� ��ȯ�� ������ ��, �Ѱ��� ��ȯ
            if (game.isAbleGunFightMode() && game.getMode() != MODE::END) {
                msg = "�Ѱ��� ������ �����մϴ�. aŰ�� ���� �Ѱ��� ���� �غ��ϼ���.";
                game.checkGunFightMode();
                if (game.getMode() == MODE::READY) {
                    msg = "�Ѱ��� �غ� �� �÷��̾ �ֽ��ϴ�. �� �̻� ���������� �Ұ����մϴ�. aŰ�� ���� �Ѱ��� ���� �غ��ϼ���.";
                }
                if (game.getMode() == MODE::GUNFIGHT) {
                    msg = "���� �Ѱ��� ����Դϴ�.";
                }
            }

            char nowMode = game.getMode();

            // ������ �۽ź�
            playerIndex = 0;
            for (auto& clientSocket : clientSockets) {
               
                // �������� �� �� �޼���
                if (game.getMode() == MODE::END) {
                    if (game.getPlayers()[playerIndex].getState() == 3) {
                        msg = "����� ����߽��ϴ�. �й��Ͽ����ϴ�.";
                    }
                    else {
                        msg = "��� �÷��̾ ����߽��ϴ�. �¸��Ͽ����ϴ�!";
                    }
                }
                // visible ���� �۽�
                send(clientSocket, &visible, sizeof(int), 0);

                // ��� ���� �۽�
                send(clientSocket, &nowMode, sizeof(int), 0);
                
                // �� ���� �۽�
                std::vector<std::vector<int>> map = game.getMap();

                char rows = 12; //map.size();
                char cols = 12; //map[0].size();

                send(clientSocket, &rows, sizeof(int), 0);
                send(clientSocket, &cols, sizeof(int), 0);

                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < cols; j++) {
                        char data = map[i][j];
                        send(clientSocket, &data, sizeof(int), 0);
                    }
                }

                // �÷��̾� ���� �۽�
                GamePlayer p = game.getPlayers()[playerIndex];
                char playerX = p.getX();
                char playerY = p.getY();
                char playerState = p.getState();
                char playerIndexS = playerIndex;

                send(clientSocket, &playerX, sizeof(int), 0);
                send(clientSocket, &playerY, sizeof(int), 0);
                send(clientSocket, &playerState, sizeof(int), 0);
                send(clientSocket, &playerIndexS, sizeof(int), 0);

                // ��� �÷��̾� ���� �۽�
                p = game.getPlayers()[(playerIndex + 1) % 2];
                playerX = p.getX();
                playerY = p.getY();
                playerState = p.getState();
                playerIndexS = (playerIndex + 1) % 2;

                send(clientSocket, &playerX, sizeof(int), 0);
                send(clientSocket, &playerY, sizeof(int), 0);
                send(clientSocket, &playerState, sizeof(int), 0);
                send(clientSocket, &playerIndexS, sizeof(int), 0);
                
                // ���ⱸ ���� �۽�
                std::vector<std::vector<int>> enter = game.getEnters()[playerIndex];
                
                char size = enter.size();

                send(clientSocket,  &size, sizeof(int), 0);
                for (int i = 0; i < enter.size(); i++) {
                    char y = enter[i][0];
                    char x = enter[i][1];

                    send(clientSocket, &y, sizeof(int), 0);
                    send(clientSocket, &x, sizeof(int), 0);
                }

                // �ⱸ
                std::vector<std::vector<int>> exit = game.getExits()[playerIndex];

                size = exit.size();

                send(clientSocket, &size, sizeof(int), 0);
                for (int i = 0; i < enter.size(); i++) {
                    char y = exit[i][0];
                    char x = exit[i][1];

                    send(clientSocket, &y, sizeof(int), 0);
                    send(clientSocket, &x, sizeof(int), 0);
                }


                // ���� �޼��� �۽�    
                char* c = const_cast<char*>(msg.c_str());
                send(clientSocket, c, 1024, 0);

                playerIndex++;
            }

            if (game.getMode() == MODE::END) {
                break;
            }

            // �ʴ� 60������
            Sleep(1000 / game.getFps());

        }

        // ���� �̺�Ʈ �� ���� ����
        WSACloseEvent(serverEvent);
        closesocket(serverSocket);
        WSACleanup();
    }

};
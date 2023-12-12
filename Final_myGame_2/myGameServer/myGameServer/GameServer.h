//GameServer.h 파일

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
        // Windows 소켓 초기화
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock\n";
            exit(EXIT_FAILURE);
        }

        // 소켓 초기화
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket\n";
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // 서버 주소 설정
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        // 바인딩
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error binding socket\n";
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // 리스닝
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

        // 소켓 이벤트 생성
        WSAEVENT serverEvent = WSACreateEvent();
        if (serverEvent == WSA_INVALID_EVENT) {
            std::cerr << "Error creating event\n";
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // 소켓 이벤트 연결
        if (WSAEventSelect(serverSocket, serverEvent, FD_ACCEPT) == SOCKET_ERROR) {
            std::cerr << "Error associating event with socket\n";
            WSACloseEvent(serverEvent);
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // 클라이언트 접속부
        while (clientSockets.size() < 2) {
            
            DWORD index = WSAWaitForMultipleEvents(1, &serverEvent, FALSE, WSA_INFINITE, FALSE);
            if (index == WSA_WAIT_FAILED) {
                std::cerr << "Error waiting for events\n";
                break;
            }

            // 새로운 클라이언트의 연결 수락
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
                            std::cout << " 플레이어 " << clientSockets.size() << " / 2 입장 완료" << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Error in FD_ACCEPT: " << networkEvents.iErrorCode[FD_ACCEPT_BIT] << std::endl;
                    }
                }
            }

        }

        
        // ON OFF 여부
        char visible;

        while (true) {

            std::string answer;
            
            std::cout << " 플레이어 간 visible 설정 (on/off)" << std::endl;
            std::cout << " >> "; std::cin >> answer;

            if (answer.compare("on") == 0) {
                visible = 1;
                break;
            }
            else if (answer.compare("off") == 0) {
                visible = 0;
                break;
            }

            std::cout << " 잘못된 입력입니다. " << std::endl;
            system("pause");
            system("cls");

        }

        system("cls");

        // 게임 시작
        while (true) {
            // 커서 제거
            clearCursor();
            // 맵 출력
            game.printGame();

            // 데이터 수신부
            int playerIndex = 0;
            for (auto& clientSocket : clientSockets) {

                char buffer[1024];
                std::memset(buffer, 0, sizeof(buffer));

                int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                
                if (bytesRead > 0) {

                    std::string s = " ";

                    switch ((int)buffer[0]) {
                    case 72: s = "화살표 위"; game.playerMove(playerIndex, 72);  break;
                    case 77: s = "화살표 오른쪽"; game.playerMove(playerIndex, 77); break;
                    case 80: s = "화살표 아래"; game.playerMove(playerIndex, 80); break;
                    case 75: s = "화살표 왼쪽"; game.playerMove(playerIndex, 75); break;
                    case 32: s = "스페이스바"; game.playerAction(playerIndex, 32);  break;
                    default: s = buffer[0]; game.playerAction(playerIndex, (int)buffer[0]);
                    }

                    gotoxy(0, 15 + playerIndex);
                    std::cout << (int)buffer[0] << "                                                                       " << std::endl;
                    gotoxy(0, 15 + playerIndex);
                    std::cout << playerIndex + 1 << "번 플레이어가 " << s << "키를 눌렀습니다. " << std::endl;
                
                }
                playerIndex++;
            }
            
            if (game.getMode() == MODE::AREACAPTURE) {
                msg = "현재 영역설정 모드입니다.";
            }

            // 총격전 변환이 가능할 때, 총격전 변환
            if (game.isAbleGunFightMode() && game.getMode() != MODE::END) {
                msg = "총격전 시작이 가능합니다. a키를 눌러 총격전 모드로 준비하세요.";
                game.checkGunFightMode();
                if (game.getMode() == MODE::READY) {
                    msg = "총격전 준비를 한 플레이어가 있습니다. 더 이상 영역설정이 불가능합니다. a키를 눌러 총격전 모드로 준비하세요.";
                }
                if (game.getMode() == MODE::GUNFIGHT) {
                    msg = "현재 총격전 모드입니다.";
                }
            }

            char nowMode = game.getMode();

            // 데이터 송신부
            playerIndex = 0;
            for (auto& clientSocket : clientSockets) {
               
                // 종료조건 일 때 메세지
                if (game.getMode() == MODE::END) {
                    if (game.getPlayers()[playerIndex].getState() == 3) {
                        msg = "당신은 사망했습니다. 패배하였습니다.";
                    }
                    else {
                        msg = "상대 플레이어가 사망했습니다. 승리하였습니다!";
                    }
                }
                // visible 정보 송신
                send(clientSocket, &visible, sizeof(int), 0);

                // 모드 정보 송신
                send(clientSocket, &nowMode, sizeof(int), 0);
                
                // 맵 정보 송신
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

                // 플레이어 정보 송신
                GamePlayer p = game.getPlayers()[playerIndex];
                char playerX = p.getX();
                char playerY = p.getY();
                char playerState = p.getState();
                char playerIndexS = playerIndex;

                send(clientSocket, &playerX, sizeof(int), 0);
                send(clientSocket, &playerY, sizeof(int), 0);
                send(clientSocket, &playerState, sizeof(int), 0);
                send(clientSocket, &playerIndexS, sizeof(int), 0);

                // 상대 플레이어 정보 송신
                p = game.getPlayers()[(playerIndex + 1) % 2];
                playerX = p.getX();
                playerY = p.getY();
                playerState = p.getState();
                playerIndexS = (playerIndex + 1) % 2;

                send(clientSocket, &playerX, sizeof(int), 0);
                send(clientSocket, &playerY, sizeof(int), 0);
                send(clientSocket, &playerState, sizeof(int), 0);
                send(clientSocket, &playerIndexS, sizeof(int), 0);
                
                // 입출구 정보 송신
                std::vector<std::vector<int>> enter = game.getEnters()[playerIndex];
                
                char size = enter.size();

                send(clientSocket,  &size, sizeof(int), 0);
                for (int i = 0; i < enter.size(); i++) {
                    char y = enter[i][0];
                    char x = enter[i][1];

                    send(clientSocket, &y, sizeof(int), 0);
                    send(clientSocket, &x, sizeof(int), 0);
                }

                // 출구
                std::vector<std::vector<int>> exit = game.getExits()[playerIndex];

                size = exit.size();

                send(clientSocket, &size, sizeof(int), 0);
                for (int i = 0; i < enter.size(); i++) {
                    char y = exit[i][0];
                    char x = exit[i][1];

                    send(clientSocket, &y, sizeof(int), 0);
                    send(clientSocket, &x, sizeof(int), 0);
                }


                // 게임 메세지 송신    
                char* c = const_cast<char*>(msg.c_str());
                send(clientSocket, c, 1024, 0);

                playerIndex++;
            }

            if (game.getMode() == MODE::END) {
                break;
            }

            // 초당 60프레임
            Sleep(1000 / game.getFps());

        }

        // 소켓 이벤트 및 소켓 정리
        WSACloseEvent(serverEvent);
        closesocket(serverSocket);
        WSACleanup();
    }

};
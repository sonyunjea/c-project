#pragma once
#include <iostream>
#include <time.h>
#include "GameInfo.h"

typedef enum {
    NORMAL,
    OCCUPATE,
    GUN,
    DEAD
}STATE;

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
}DIRECTION;


class GamePlayer {
private:
    int dir;    // 플레이어의 현재 바라보고 있는 방향
    int x;  // 플레이어의 x 좌표
    int y;  // 플레이어의 y 좌표

    int state; // 플레이어의 현 상태
    /*
    
    0 : 일반 이동
    1 : 땅따먹기 중 이동
    2 : 총격전 모드 (준비)

    */

    std::vector<std::vector<int>> occupatingPos;      // 점령을 시도하고 있는 중인 위치들

public:
    // 생성자: GamePlayer 객체를 초기화합니다.
    GamePlayer(int startX, int startY) : x(startX), y(startY), state(STATE::NORMAL), dir(DIRECTION::UP) {}
    ~GamePlayer() {}

    // 플레이어 이동
    void movePlayer(std::vector<std::vector<int>> map, int d, int area) {

        int newX = x;
        int newY = y;
        int newDir = dir;

        switch (d) {
        case 72: 
            newY = max(1, y - 1);
            newDir = DIRECTION::UP;
            break;
        case 77: 
            newX = min(map[0].size() - 2, x + 1);
            newDir = DIRECTION::RIGHT;
            break;
        case 80: 
            newY = min(map.size() - 2, y + 1);
            newDir = DIRECTION::DOWN;
            break;
        case 75: 
            newX = max(1, x - 1);
            newDir = DIRECTION::LEFT;
            break;
        }

        // 점령 이동
        if (state == STATE::OCCUPATE) {
            // 점령할 공간 누적
            occupatingPos.push_back({ newY, newX });

            x = newX;
            y = newY;
            dir = newDir;
        }

        // 일반 이동
        else {
            // 일반 이동일 때는 자기 구역일 때만 이동
            if (map[newY][newX] == area) {
                x = newX;
                y = newY;
                dir = newDir;
            }
        }

    }

    // 점령 시도
    bool isAbleOccupation(int playerIndex, std::vector<std::vector<int>> map, int other) {

        for (auto& myP : occupatingPos) {

            int place = other;

            if (map[myP[0]][myP[1]] == 4 || map[myP[0]][myP[1]] == 5) return false;

        }

        // 시작점 끝점이 맞지 않으면 false
        if (occupatingPos[0][1] != x || occupatingPos[0][0] != y) return false;

        return true;

    }

    void clearOccuPos() {
        occupatingPos.clear();
    }

    // 상태 변환
    void setState(int s) {
        state = s;

        if (state == STATE::OCCUPATE) {
            occupatingPos.push_back({ y, x });
        }
    }

    // 위치 변환
    void setPos(int newX, int newY) {
        x = newX;
        y = newY;
    }

    int getX() { return x; }
    int getY() { return y; }
    int getDir() { return dir; }
    int getState() { return state; }
  
    std::vector<std::vector<int>> getOccuPos() { return occupatingPos; }

};
#pragma once

struct PlayerConfig {
    bool isAI    = false;
    int  aiDepth = 2; // 1=Easy, 2=Medium, 3=Hard — ignored if isAI==false
};

struct GameConfig {
    PlayerConfig players[3]; // 0=P1 Blancs, 1=P2 Bleus, 2=P3 Rouges
};

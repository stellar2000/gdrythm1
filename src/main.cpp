#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <fstream>
using namespace geode::prelude;

static bool s_levelStarted = false;
static int  s_seq          = 0;   // increments on every write so Python never misses one

static void writeState(const char* state) {
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    std::string path = std::string(tmp) + "gd_overlay_state.txt";
    std::ofstream f(path, std::ios::trunc);
    f << state << '\n' << ++s_seq;
}

class $modify(GDOverlayBGL, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        if (!typeinfo_cast<PlayLayer*>(this)) return;
        if (!s_levelStarted && m_player1 && !m_player1->m_isDead
                && m_gameState.m_levelTime > 0.05) {
            s_levelStarted = true;
            writeState("START");
        }
    }
};

class $modify(GDOverlayPL, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        s_levelStarted = false;
        writeState("RESET");
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        s_levelStarted = false;
        writeState("RESET");
    }

    void pauseGame(bool p0) {
        PlayLayer::pauseGame(p0);
        writeState("PAUSE");
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (player != m_player1 && player != m_player2)
            return PlayLayer::destroyPlayer(player, object);
        PlayLayer::destroyPlayer(player, object);
        if (object != m_anticheatSpike && s_levelStarted) {
            s_levelStarted = false;
            writeState("DEATH");
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        s_levelStarted = false;
        writeState("QUIT");
    }
};

class $modify(GDOverlayPause, PauseLayer) {
    void onResume(CCObject* sender) {
        writeState("RESUME");
        PauseLayer::onResume(sender);
    }
};

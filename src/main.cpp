#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <fstream>
#include <cstdio>
using namespace geode::prelude;

static bool s_levelStarted = false;
// Blocks processCommands from re-writing START after a death until
// resetLevel actually fires. Without this, noclip causes a tight
// DEATH→START→DEATH loop at 240fps that kills performance.
static bool s_pendingReset = false;

static void enqueue(const char* msg) {
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    std::string path = std::string(tmp) + "gd_overlay_queue.txt";
    std::ofstream f(path, std::ios::app);
    f << msg << '\n';
}

class $modify(GDOverlayBGL, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        if (!typeinfo_cast<PlayLayer*>(this)) return;
        // s_pendingReset blocks this until the player actually restarts
        if (!s_levelStarted && !s_pendingReset
                && m_player1 && !m_player1->m_isDead
                && m_gameState.m_levelTime > 0.05) {
            s_levelStarted = true;
            // Send the exact level time so Python can correct for poll lag
            char buf[64];
            std::snprintf(buf, sizeof(buf), "START:%.5f",
                          (double)m_gameState.m_levelTime);
            enqueue(buf);
        }
    }
};

class $modify(GDOverlayPL, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        s_levelStarted = false;
        s_pendingReset = false;
        enqueue("RESET");
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        s_levelStarted = false;
        s_pendingReset = false;   // clear the noclip guard so START can fire again
        enqueue("RESET");
    }

    void pauseGame(bool p0) {
        PlayLayer::pauseGame(p0);
        enqueue("PAUSE");
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (player != m_player1 && player != m_player2)
            return PlayLayer::destroyPlayer(player, object);
        PlayLayer::destroyPlayer(player, object);
        if (object != m_anticheatSpike && s_levelStarted) {
            s_levelStarted = false;
            s_pendingReset = true;    // noclip guard: block START until real reset
            enqueue("DEATH");
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        s_levelStarted = false;
        s_pendingReset = false;
        enqueue("QUIT");
    }
};

class $modify(GDOverlayPause, PauseLayer) {
    void onResume(CCObject* sender) {
        enqueue("RESUME");
        PauseLayer::onResume(sender);
    }
};

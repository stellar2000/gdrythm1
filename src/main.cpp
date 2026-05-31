#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <fstream>
#include <cstdio>
#include <windows.h>
using namespace geode::prelude;

static bool  s_levelStarted = false;
static bool  s_pendingReset = false;

// ── Append queue (state events) ───────────────────────────────────────────────
static void enqueue(const char* msg) {
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    std::ofstream f(std::string(tmp) + "gd_overlay_queue.txt", std::ios::app);
    f << msg << '\n';
}

// ── Shared memory (live m_levelTime, read every frame by Python) ──────────────
// Python opens "GDOverlayTime" and reads a float32:
//   >= 0.0  → game is running, value is current m_levelTime
//   -1.0    → not playing (reset / death / quit / between attempts)
static HANDLE  s_hMap  = NULL;
static float*  s_pTime = NULL;

static void shmWrite(float v) {
    if (!s_hMap) {
        s_hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
                                     PAGE_READWRITE, 0, 4, "GDOverlayTime");
        if (s_hMap)
            s_pTime = static_cast<float*>(
                MapViewOfFile(s_hMap, FILE_MAP_WRITE, 0, 0, 4));
    }
    if (s_pTime) *s_pTime = v;
}

// ── Game-tick hook ─────────────────────────────────────────────────────────────
class $modify(GDOverlayBGL, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        if (!typeinfo_cast<PlayLayer*>(this)) return;

        float lt = m_gameState.m_levelTime;

        if (!s_levelStarted && !s_pendingReset
                && m_player1 && !m_player1->m_isDead && lt > 0.05f) {
            s_levelStarted = true;
            char buf[64];
            std::snprintf(buf, sizeof(buf), "START:%.5f", (double)lt);
            enqueue(buf);
        }

        // Write live level-time to shared memory every frame.
        // Python reads this directly — no polling lag, no drift.
        if (s_levelStarted)
            shmWrite(lt);
    }
};

// ── PlayLayer hooks ────────────────────────────────────────────────────────────
class $modify(GDOverlayPL, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        s_levelStarted = false; s_pendingReset = false;
        shmWrite(-1.0f);
        enqueue("RESET");
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        s_levelStarted = false; s_pendingReset = false;
        shmWrite(-1.0f);
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
            s_levelStarted = false; s_pendingReset = true;
            shmWrite(-1.0f);
            enqueue("DEATH");
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        s_levelStarted = false; s_pendingReset = false;
        shmWrite(-1.0f);
        enqueue("QUIT");
    }
};

class $modify(GDOverlayPause, PauseLayer) {
    void onResume(CCObject* sender) {
        enqueue("RESUME");
        PauseLayer::onResume(sender);
    }
};

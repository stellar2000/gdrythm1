#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <fstream>
using namespace geode::prelude;

// Shared flag — set true once the level is actually running,
// cleared on reset/death/quit so destroyPlayer only fires when live
static bool s_levelStarted = false;

static void writeState(const char* state) {
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    std::string path = std::string(tmp) + "gd_overlay_state.txt";
    std::ofstream f(path, std::ios::trunc);
    f << state;
}

// Hook GJBaseGameLayer::processCommands — this is the real game tick,
// called every frame during gameplay. Eclipse uses this exact approach
// for 2.2081. PlayLayer::update has no hookable vtable entry in 2.2081.
class $modify(GDOverlayBGL, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        // Only run inside PlayLayer, not LevelEditorLayer etc.
        if (!typeinfo_cast<PlayLayer*>(this)) return;
        // m_gameState.m_levelTime counts up from 0 once gameplay begins.
        // At 0.05s the black loading screen is long gone and music is playing.
        if (!s_levelStarted && m_player1 && !m_player1->m_isDead
                && m_gameState.m_levelTime > 0.05) {
            s_levelStarted = true;
            writeState("START");
        }
    }
};

class $modify(GDOverlayPL, PlayLayer) {
    // Fires when the level object is created (before loading screen)
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        s_levelStarted = false;
        writeState("RESET");
        return true;
    }

    // Fires on each attempt start (death restart, practice restart, etc.)
    void resetLevel() {
        PlayLayer::resetLevel();
        s_levelStarted = false;
        writeState("RESET");
    }

    // Fires when the player hits something fatal.
    // Filter: ignore the anticheat spike (fires during loading) and
    // ignore non-player objects. Only write DEATH if we were actually live.
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (player != m_player1 && player != m_player2)
            return PlayLayer::destroyPlayer(player, object);
        PlayLayer::destroyPlayer(player, object);
        if (object != m_anticheatSpike && s_levelStarted) {
            s_levelStarted = false;
            writeState("DEATH");
        }
    }

    // Fires when the player exits the level
    void onQuit() {
        PlayLayer::onQuit();
        s_levelStarted = false;
        writeState("QUIT");
    }
};

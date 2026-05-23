#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <fstream>

using namespace geode::prelude;

// Writes to %TEMP%\gd_overlay_state.txt
// Python overlay polls this file every 50ms
static void writeState(const char* state) {
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    std::string path = std::string(tmp) + "gd_overlay_state.txt";
    std::ofstream f(path, std::ios::trunc);
    f << state;
}

class $modify(PlayLayer) {

    // Called every attempt start (including first)
    void resetLevel() {
        PlayLayer::resetLevel();
        writeState("START");
    }

    // Called when player dies
    void destroyPlayer(PlayerObject* p, GameObject* o) {
        PlayLayer::destroyPlayer(p, o);
        writeState("DEATH");
    }

    // Called when leaving the level
    void onQuit() {
        PlayLayer::onQuit();
        writeState("QUIT");
    }

};

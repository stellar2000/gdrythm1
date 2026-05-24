#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <fstream>
using namespace geode::prelude;

static void writeState(const char* state) {
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    std::string path = std::string(tmp) + "gd_overlay_state.txt";
    std::ofstream f(path, std::ios::trunc);
    f << state;
}

class $modify(PlayLayer) {
    bool m_levelStarted = false;

    void resetLevel() {
        PlayLayer::resetLevel();
        m_levelStarted = false;
        writeState("RESET");
    }

    void startGame() {
        PlayLayer::startGame();
        m_levelStarted = true;
        writeState("START");
    }

    void destroyPlayer(PlayerObject* p, GameObject* o) {
        PlayLayer::destroyPlayer(p, o);
        if (m_levelStarted) {
            writeState("DEATH");
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        m_levelStarted = false;
        writeState("QUIT");
    }
};

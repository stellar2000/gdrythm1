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
    struct Fields {
        bool m_levelStarted = false;
    };

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->m_levelStarted = false;
        writeState("RESET");
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        writeState("RESET");
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_levelStarted && !m_isDead && m_time > 0.05f) {
            m_fields->m_levelStarted = true;
            writeState("START");
        }
    }

    void destroyPlayer(PlayerObject* p, GameObject* o) {
        PlayLayer::destroyPlayer(p, o);
        if (m_fields->m_levelStarted) {
            m_fields->m_levelStarted = false;
            writeState("DEATH");
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        m_fields->m_levelStarted = false;
        writeState("QUIT");
    }
};

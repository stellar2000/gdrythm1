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

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        m_fields->m_levelStarted = false;
        writeState("RESET");
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->m_levelStarted = false;
        writeState("RESET");
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_levelStarted && m_player1 && !m_player1->m_isDead && m_time > 0.1f) {
            m_fields->m_levelStarted = true;
            writeState("START");
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Filter anticheat and non-player deaths exactly like Eclipse does
        if (player != m_player1 && player != m_player2) {
            return PlayLayer::destroyPlayer(player, object);
        }
        PlayLayer::destroyPlayer(player, object);
        if (object != m_anticheatSpike && m_fields->m_levelStarted) {
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

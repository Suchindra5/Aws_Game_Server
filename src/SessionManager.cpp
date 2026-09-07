#include "SessionManager.hpp"

void SessionManager::update_activity(uint32_t player_id, const sockaddr_in& addr) {
    sessions[player_id] = SessionInfo{player_id, addr, std::chrono::steady_clock::now()};
}

void SessionManager::remove_player(uint32_t player_id) {
    sessions.erase(player_id);
}

std::vector<SessionInfo> SessionManager::get_active_sessions() const {
    std::vector<SessionInfo> active;
    for (const auto& pair : sessions) {
        active.push_back(pair.second);
    }
    return active;
}

std::vector<uint32_t> SessionManager::get_timed_out_players(double timeout_seconds) {
    std::vector<uint32_t> timed_out;
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = sessions.begin(); it != sessions.end(); ) {
        std::chrono::duration<double> elapsed = now - it->second.last_active;
        if (elapsed.count() > timeout_seconds) {
            timed_out.push_back(it->first);
            it = sessions.erase(it);
        } else {
            ++it;
        }
    }
    return timed_out;
}
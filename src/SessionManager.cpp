#include "SessionManager.hpp"

void SessionManager::update_activity(uint32_t player_id) {
    last_seen[player_id] = std::chrono::steady_clock::now();
}

void SessionManager::remove_player(uint32_t player_id) {
    last_seen.erase(player_id);
}

std::vector<uint32_t> SessionManager::get_timed_out_players(double timeout_seconds) {
    std::vector<uint32_t> timed_out;
    auto now = std::chrono::steady_clock::now();

    for (auto it = last_seen.begin(); it != last_seen.end(); ) {
        std::chrono::duration<double> elapsed = now - it->second;
        if (elapsed.count() > timeout_seconds) {
            timed_out.push_back(it->first);
            it = last_seen.erase(it); // Erase safely while iterating
        } else {
            ++it;
        }
    }
    
    return timed_out;
}
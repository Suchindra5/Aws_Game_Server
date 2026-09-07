#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <chrono>

class SessionManager {
private:
    std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> last_seen;
    
public:
    // Update or record the last active timestamp for a player
    void update_activity(uint32_t player_id);

    // Explicitly remove a player session
    void remove_player(uint32_t player_id);

    // Check all sessions and return IDs of players who exceeded the timeout threshold
    std::vector<uint32_t> get_timed_out_players(double timeout_seconds);
};

#endif // SESSION_MANAGER_HPP
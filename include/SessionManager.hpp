#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <netinet/in.h>
#endif

struct SessionInfo {
    uint32_t player_id;
    sockaddr_in address;
    std::chrono::steady_clock::time_point last_active;
};

class SessionManager {
private:
    std::unordered_map<uint32_t, SessionInfo> sessions;
    
public:
    void update_activity(uint32_t player_id, const sockaddr_in& addr);
    void remove_player(uint32_t player_id);
    std::vector<SessionInfo> get_active_sessions() const;
    std::vector<uint32_t> get_timed_out_players(double timeout_seconds);
};

#endif // SESSION_MANAGER_HPP
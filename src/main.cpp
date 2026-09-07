#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>

// Cross-Platform Networking Headers
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    
    // Define Windows socket types/macros for Linux compatibility
    typedef int SOCKET;
    const SOCKET INVALID_SOCKET = -1;
    const int SOCKET_ERROR = -1;
    inline int closesocket(SOCKET s) { return close(s); }
#endif

#include "SpatialGrid.hpp"
#include "SessionManager.hpp"

struct ClientPacket {
    uint32_t player_id;
    uint32_t sequence;
    float x;
    float y;
};

// Structure matching the server-to-client response packet (8 bytes total)
struct ServerResponsePacket {
    uint32_t packet_type = 1; 
    uint32_t nearby_count;
};

int main() {
    // 1. Initialize Network Stack (Winsock on Windows, no-op on Linux)
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Error] WSAStartup failed.\n";
        return 1;
    }
#endif

    SpatialGrid spatial_grid(50.0f); // 50x50 unit grid cells
    SessionManager session_manager;
    
    const double TIMEOUT_SECONDS = 5.0; 
    const auto TICK_RATE = std::chrono::milliseconds(50); // 20 TPS loop (50ms per tick)
    const int PORT = 8080;

    // 2. Create UDP Socket
    SOCKET server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "[Error] Failed to create socket.\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // 3. Bind Socket to Port
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[Error] Failed to bind socket to port " << PORT << ".\n";
        closesocket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // 4. Set Socket to Non-Blocking Mode (Cross-platform)
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(server_fd, FIONBIO, &mode) != 0) {
        std::cerr << "[Error] Failed to set non-blocking mode.\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
#else
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "[Error] Failed to set non-blocking mode.\n";
        closesocket(server_fd);
        return 1;
    }
#endif

    std::cout << "[Server] Listening for UDP packets on port " << PORT << " at 20 TPS (Bidirectional)...\n";

    char buffer[1024];
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    while (true) {
        auto tick_start = std::chrono::steady_clock::now();

        // 5. Ingestion Loop: Read all available non-blocking UDP packets for this tick
        while (true) {
            client_addr_len = sizeof(client_addr); 
            int bytes_received = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&client_addr, &client_addr_len);
            
            if (bytes_received == SOCKET_ERROR) {
#ifdef _WIN32
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) { break; }
#else
                if (errno == EWOULDBLOCK || errno == EAGAIN) { break; }
#endif
                break; 
            }

            // Validate packet size matches our expected structure
            if (bytes_received == sizeof(ClientPacket)) {
                ClientPacket packet;
                std::memcpy(&packet, buffer, sizeof(ClientPacket));

                // Register endpoint and coordinates
                session_manager.update_activity(packet.player_id, client_addr);
                spatial_grid.update_player(packet.player_id, packet.x, packet.y);
            }
        }

        // 6. Outbound Broadcast Loop: Send state back to all active connected clients
        auto active_sessions = session_manager.get_active_sessions();
        for (const auto& session : active_sessions) {
            float px = spatial_grid.get_player_x(session.player_id);
            float py = spatial_grid.get_player_y(session.player_id);
            auto nearby = spatial_grid.get_nearby_players(px, py, 1);

            ServerResponsePacket response;
            response.nearby_count = static_cast<uint32_t>(nearby.size());

            // Send state back to the client's endpoint via UDP
            sendto(server_fd, (char*)&response, sizeof(response), 0,
                   (struct sockaddr*)&session.address, sizeof(session.address));
        }

        // 7. Timeout Cleanup: Prune silent/disconnected players
        std::vector<uint32_t> dropped_players = session_manager.get_timed_out_players(TIMEOUT_SECONDS);
        for (uint32_t dropped_id : dropped_players) {
            spatial_grid.remove_player(dropped_id);
            std::cout << "[Server] Player " << dropped_id << " timed out and was removed from the grid.\n";
        }

        // 8. Maintain 20 TPS Tick Rate
        auto tick_end = std::chrono::steady_clock::now();
        auto elapsed = tick_end - tick_start;
        if (elapsed < TICK_RATE) {
            std::this_thread::sleep_for(TICK_RATE - elapsed);
        }
    }

    closesocket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
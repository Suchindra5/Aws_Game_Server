#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>

// Windows Networking Headers
#include <winsock2.h>
#include <ws2tcpip.h>

// Tell the compiler to link the Winsock library automatically (MinGW specific)
#pragma comment(lib, "ws2_32.lib")

#include "SpatialGrid.hpp"
#include "SessionManager.hpp"

struct ClientPacket {
    uint32_t player_id;
    uint32_t sequence;
    float x;
    float y;
};

int main() {
    // 1. Initialize Winsock (Required on Windows before using any sockets)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Error] WSAStartup failed.\n";
        return 1;
    }

    SpatialGrid spatial_grid(50.0f); // 50x50 unit grid cells
    SessionManager session_manager;
    
    const double TIMEOUT_SECONDS = 5.0; 
    const auto TICK_RATE = std::chrono::milliseconds(50); // 20 TPS loop (50ms per tick)
    const int PORT = 8080;

    // 2. Create UDP Socket (On Windows, socket descriptors are of type 'SOCKET')
    SOCKET server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "[Error] Failed to create socket. Error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // 3. Bind Socket to Port
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[Error] Failed to bind socket to port " << PORT << ". Error: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 4. Set Socket to Non-Blocking Mode on Windows using ioctlsocket
    u_long mode = 1; // 1 to enable non-blocking, 0 for blocking
    if (ioctlsocket(server_fd, FIONBIO, &mode) != 0) {
        std::cerr << "[Error] Failed to set non-blocking mode.\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "[Server] Listening for UDP packets on port " << PORT << " at 20 TPS (Windows Winsock)...\n";

    char buffer[1024];
    sockaddr_in client_addr{};
    int client_addr_len = sizeof(client_addr); // Note: int on Windows, socklen_t on Linux

    while (true) {
        auto tick_start = std::chrono::steady_clock::now();

        // 5. Ingestion Loop: Read all available non-blocking UDP packets for this tick
        while (true) {
            int bytes_received = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                                            (struct sockaddr*)&client_addr, &client_addr_len);
            
            if (bytes_received == SOCKET_ERROR) {
                int err = WSAGetLastError();
                // WSAEWOULDBLOCK means no more packets are waiting right now
                if (err == WSAEWOULDBLOCK) {
                    break; 
                } else {
                    break; // Other read error
                }
            }

            // Validate packet size matches our expected structure
            if (bytes_received == sizeof(ClientPacket)) {
                ClientPacket packet;
                std::memcpy(&packet, buffer, sizeof(ClientPacket));

                // Update Session & Spatial Grid with real client data
                session_manager.update_activity(packet.player_id);
                spatial_grid.update_player(packet.player_id, packet.x, packet.y);

                auto nearby = spatial_grid.get_nearby_players(packet.x, packet.y, 1);

                std::cout << "[Packet Received] ID: " << packet.player_id 
                          << " | Pos: (" << packet.x << ", " << packet.y << ")"
                          << " | Nearby Peers: " << nearby.size() << "\n";
            }
        }

        // 6. Timeout Cleanup: Prune silent/disconnected players
        std::vector<uint32_t> dropped_players = session_manager.get_timed_out_players(TIMEOUT_SECONDS);
        for (uint32_t dropped_id : dropped_players) {
            spatial_grid.remove_player(dropped_id);
            std::cout << "[Server] Player " << dropped_id << " timed out and was removed from the grid.\n";
        }

        // 7. Maintain 20 TPS Tick Rate
        auto tick_end = std::chrono::steady_clock::now();
        auto elapsed = tick_end - tick_start;
        if (elapsed < TICK_RATE) {
            std::this_thread::sleep_for(TICK_RATE - elapsed);
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
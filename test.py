import socket
import struct
import time
import threading

SERVER_IP = "127.0.0.1"
SERVER_PORT = 8080

def simulate_client(lobby_id, player_id, start_x, start_y):
    # Each thread gets its own UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2.0)
    
    print(f"[Client {player_id}] Starting simulation for Lobby '{lobby_id}'...")

    sequence = 0
    x_pos = start_x
    y_pos = start_y

    try:
        for i in range(15):  # Send 15 ticks per client
            sequence += 1
            x_pos += 1.5
            y_pos += 1.0

            # Pack data matching C++ struct ClientPacket (<32sIIff)
            encoded_lobby_id = lobby_id.encode('utf-8').ljust(32, b'\x00')
            packet_data = struct.pack('<32sIIff', encoded_lobby_id, player_id, sequence, x_pos, y_pos)

            sock.sendto(packet_data, (SERVER_IP, SERVER_PORT))
            print(f"[Lobby: {lobby_id}] Player {player_id} sent Seq {sequence} | Pos: ({x_pos:.1f}, {y_pos:.1f})")

            try:
                response_data, _ = sock.recvfrom(1024)
                if len(response_data) == 8:
                    pkt_type, nearby_count = struct.unpack('<II', response_data)
                    print(f"[Lobby: {lobby_id}] Player {player_id} received response -> Nearby Peers: {nearby_count}")
            except socket.timeout:
                print(f"[Lobby: {lobby_id}] Player {player_id} response timed out.")

            time.sleep(0.05)  # 20 TPS tick rate

    finally:
        sock.close()
        print(f"[Client {player_id}] Finished and closed socket.")

if __name__ == "__main__":
    print(f"[Main] Spawning two clients for two different lobbies...")

    # Create two threads representing players in distinct lobbies
    thread1 = threading.Thread(target=simulate_client, args=("lobby_ALPHA_100", 101, 10.0, 20.0))
    thread2 = threading.Thread(target=simulate_client, args=("lobby_BETA_200", 201, 500.0, 600.0))

    # Start both threads concurrently
    thread1.start()
    thread2.start()

    # Wait for both simulations to finish
    thread1.join()
    thread2.join()

    print("[Main] Both lobby simulations completed successfully!")
import socket
import struct
import time
import threading
import random

SERVER_IP = "13.203.224.151"  # Your AWS EC2 IP
SERVER_PORT = 8080
NUM_CLIENTS = 10         # Number of concurrent players to simulate
TICK_RATE = 20           # 20 packets per second per client
INTERVAL = 1.0 / TICK_RATE

def run_client(player_id):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.settimeout(0.1)
    
    # Start each client at a random initial position
    x = random.uniform(0.0, 500.0)
    y = random.uniform(0.0, 500.0)
    sequence = 0

    print(f"[Client {player_id}] Started, streaming & listening to {SERVER_IP}:{SERVER_PORT}")

    try:
        while True:
            # Simulate slight movement
            x += random.uniform(-1.5, 1.5)
            y += random.uniform(-1.5, 1.5)
            sequence += 1

            # 1. Send packet matching C++ ClientPacket structure ("IIff")
            packet = struct.pack("IIff", player_id, sequence, x, y)
            client_socket.sendto(packet, (SERVER_IP, SERVER_PORT))
            
            # 2. Receive response matching C++ ServerResponsePacket structure ("II")
            try:
                data, _ = client_socket.recvfrom(1024)
                if len(data) == 8:  # 2 uint32 fields = 8 bytes
                    packet_type, nearby_count = struct.unpack("II", data)
                    # Print only for Client 1 to avoid flooding the screen too fast
                    if player_id == 1 and sequence % 20 == 0:
                        print(f"[Client {player_id}] Sent Pos ({x:.1f}, {y:.1f}) | Server Response -> Nearby Peers: {nearby_count}")
            except socket.timeout:
                pass  
            
            time.sleep(INTERVAL)
            
    except KeyboardInterrupt:
        client_socket.close()

if __name__ == "__main__":
    print(f"[Spawner] Launching {NUM_CLIENTS} concurrent bidirectional clients...")
    threads = []

    for i in range(1, NUM_CLIENTS + 1):
        t = threading.Thread(target=run_client, args=(i,))
        threads.append(t)
        t.start()
        time.sleep(0.05)

    for t in threads:
        t.join()
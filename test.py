import socket
import struct
import time
import threading
import random

SERVER_IP = "127.0.0.1"  # Change to your AWS EC2 IP later when deployed!
SERVER_PORT = 8080
NUM_CLIENTS = 10        # Number of concurrent players to simulate
TICK_RATE = 20           # 20 packets per second per client
INTERVAL = 1.0 / TICK_RATE

def run_client(player_id):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Start each client at a random initial position
    x = random.uniform(0.0, 500.0)
    y = random.uniform(0.0, 500.0)
    sequence = 0

    print(f"[Client {player_id}] Started, streaming to {SERVER_IP}:{SERVER_PORT}")

    try:
        while True:
            # Simulate slight movement
            x += random.uniform(-1.5, 1.5)
            y += random.uniform(-1.5, 1.5)
            sequence += 1

            # Pack binary data matching your C++ ClientPacket structure:
            # uint32_t player_id, uint32_t sequence, float x, float y -> Format: "IIff"
            packet = struct.pack("IIff", player_id, sequence, x, y)
            
            client_socket.sendto(packet, (SERVER_IP, SERVER_PORT))
            
            time.sleep(INTERVAL)
    except KeyboardInterrupt:
        client_socket.close()

if __name__ == "__main__":
    print(f"[Spawner] Launching {NUM_CLIENTS} concurrent clients...")
    threads = []

    # Spawn each client in its own background thread
    for i in range(1, NUM_CLIENTS + 1):
        t = threading.Thread(target=run_client, args=(i,))
        threads.append(t)
        t.start()
        time.sleep(0.05) # Stagger starts slightly

    # Keep main thread alive
    for t in threads:
        t.join()
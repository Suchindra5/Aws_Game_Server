import socket
import struct
import time
import threading

SERVER_IP = "13.203.224.151"
SERVER_PORT = 8080
LOBBY_ID = "lobby_test_broadcast_123"

def client_simulation(player_id, start_x, start_y):
    # Create a unique UDP socket for this player
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.5) # Non-blocking short timeout for listening
    
    x = start_x
    y = start_y
    sequence = 0

    print(f"🚀 [Player {player_id}] Started and listening for peers...")

    try:
        for i in range(20): # Run for 20 ticks
            sequence += 1
            x += 1.0
            y += 0.5

            # Pack struct: 32s (lobby_id), I (player_id), I (sequence), f (x), f (y)
            encoded_lobby = LOBBY_ID.encode('utf-8').ljust(32, b'\x00')
            packet_data = struct.pack('<32sIIff', encoded_lobby, player_id, sequence, x, y)

            # Send movement to EC2 server
            sock.sendto(packet_data, (SERVER_IP, SERVER_PORT))
            print(f"📡 [Player {player_id}] Sent Pos -> ({x:.1f}, {y:.1f})")

            # Try to receive incoming data (either server ack or broadcasted peer packet)
            try:
                while True:
                    data, addr = sock.recvfrom(1024)
                    
                    # Check if it's an 8-byte server acknowledgment response
                    if len(data) == 8:
                        pkt_type, nearby = struct.unpack('<II', data)
                        # print(f"📥 [Player {player_id}] Ack received. Nearby peers: {nearby}")
                    
                    # Check if it's a broadcasted ClientPacket from another player (size is 48 bytes)
                    elif len(data) == struct.calcsize('<32sIIff'):
                        p_lobby, p_id, p_seq, p_x, p_y = struct.unpack('<32sIIff', data)
                        p_lobby_clean = p_lobby.decode('utf-8').rstrip('\x00')
                        
                        if p_id != player_id:
                            print(f"🎯 MATCH! [Player {player_id}] heard from [Player {p_id}] at Pos: ({p_x:.1f}, {p_y:.1f})")
            except socket.timeout:
                pass

            time.sleep(0.1) # 10 ticks per second for test

    finally:
        sock.close()
        print(f"🏁 [Player {player_id}] Test finished.")

if __name__ == "__main__":
    print("==================================================")
    print("Starting Multi-Client Broadcast Test (2 Players)")
    print("==================================================")

    # Spin up Player 777 and Player 888 in parallel threads
    t1 = threading.Thread(target=client_simulation, args=(777, 100.0, 100.0))
    t2 = threading.Thread(target=client_simulation, args=(888, 200.0, 200.0))

    t1.start()
    t2.start()

    t1.join()
    t2.join()

    print("\n✅ Multi-client test completed! Check your C++ server logs.")
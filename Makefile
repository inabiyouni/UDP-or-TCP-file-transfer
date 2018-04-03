all: TCP_client TCP_server_thr TCP_server UDP_client UDP_server 
TCP_client: main_TCP_client.cpp
	g++ -pthread -std=gnu++11 main_TCP_client.cpp -o main_TCP_client
TCP_server_thr: main_TCP_server_multi_thread.cpp
	g++ -pthread -std=gnu++11 main_TCP_server_multi_thread.cpp -o main_TCP_server_multi_thread
TCP_server: main_TCP_server_single.cpp
	g++ -pthread -std=gnu++11 main_TCP_server_single.cpp -o main_TCP_server_single
UDP_client: main_UDP_client.cpp
	g++ -pthread -std=gnu++11 main_UDP_client.cpp -o main_UDP_client
UDP_server: main_UDP_server.cpp
	g++ -pthread -std=gnu++11 main_UDP_server.cpp -o main_UDP_server

clean:
	rm -rf main_TCP_client main_TCP_server_multi_thread main_UDP_server main_UDP_client main_TCP_server_single

all : game_client game_server

game_client: game_client.o
	g++ game_client.o -std=c++11 -o game_client

game_server: game_server.o
	g++ game_server.o -std=c++11 -lpthread -o game_server

game_client.o : game_client.cpp
	g++ -c -Wall -std=c++11 -g game_client.cpp

game_server.o: game_server.cpp
	g++ -c -Wall -std=c++11 -g game_server.cpp

clean:
	rm *.o game_client game_server

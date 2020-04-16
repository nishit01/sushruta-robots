#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <bits/stdc++.h>
#include <chrono>
#include "utils.h"

using namespace std;


// Global Variables
Robot myDetails;
int robot_count;
int station_count;
int item_count;
Robot* robots;
Station* stations;
Item* items;
int grid_size;
int** grid;

// Function Prototype
void createServer();
void connectToInitiator();


/*
Function to create server thread
*/
void createServer() {

	int portNo;

	portNo = generatePortNo();
	myDetails.networkInfo.portNo = portNo;

	int server_fd, new_socket;
	int opt = 1;
	int recv_msg;

	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// create socket descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("robot.cpp:createServer() -> socket descriptor failed\n");
		exit(EXIT_FAILURE);
	}

	// socket settings
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("robot.cpp:createServer() -> setsockopt failed\n");
		exit(EXIT_FAILURE);
	}

	// create socket with port number as PortNo
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portNo);


	// bind socket with the ipv4 address and port no
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("robot.cpp:createServer() -> bind failed\n");
		exit(EXIT_FAILURE);
	}


	// listen to port
	if (listen(server_fd, 1000) < 0) {
		perror("robot.cpp:createServer() -> listen failed\n");
		exit(EXIT_FAILURE);
	}


	thread th1(connectToInitiator);
	th1.detach();

	while (1) {

		// accept new connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("station.cpp:createServer() -> accept failed\n");
			exit(EXIT_FAILURE);
		}

		int msg;
		msg = -1;
		read(new_socket, &msg, sizeof(msg));
		if (msg == STATION) {
			cout << "Got order from station" << endl;
			Order order;
			read(new_socket, &order, sizeof(Order));
			cout << "Station id is: " << order.stationId << endl;
		}
		// send(new_socket, &msg, sizeof(int), 0);
		// cout << "Sent Welcome Message\n";

		close(new_socket);
	}
}



void connectToInitiator() {

	int sock;
	struct sockaddr_in serv_addr;
	int initiatorPortNo = 9000;

	int recv_msg;
	int msg;

	// create socket descriptor
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("station.cpp:connectToInitiator() -> socket descriptor failed\n");
		exit(EXIT_FAILURE);
	}


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(initiatorPortNo);


	// convert ipv4 address from text to binary info
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("station.cpp:connectToInitiator() -> invalid address not supported\n");
		exit(EXIT_FAILURE);
	}

	// connect to server 127.0.0.1:9000
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("station.cpp:connectToInitiator() -> connection failed\n");
		exit(EXIT_FAILURE);
	}

	// send initial message about being robot
	msg = ROBOT;

	// match-1
	send(sock, &msg, sizeof(int), 0);


	// receive robot id from robot
	// match-2
	recv_msg = read(sock, &myDetails.robotId, sizeof(int));
	// printRobotInfo(&myDetails, 1);

	printMsg("robot.cpp:connectToInitiator() -> received robot id " + to_string(myDetails.robotId));

	// send myDetails i.e. struct Robot
	// match-3
	send(sock, &myDetails.networkInfo.portNo, sizeof(int), 0);
	recv_msg = read(sock, &myDetails, sizeof(Robot));
	printRobotInfo(&myDetails, 1);

	// receive broadcast message from initiator about number of robots, stations
	// match-4
	recv_msg = read(sock, &robot_count, sizeof(int));
	recv_msg = read(sock, &station_count, sizeof(int));
	recv_msg = read(sock, &item_count, sizeof(int));
	recv_msg = read(sock, &grid_size, sizeof(int));

	robots = (Robot *)malloc(robot_count * sizeof(Robot));
	stations = (Station *)malloc(station_count * sizeof(Station));
	items = (Item *)malloc(item_count * sizeof(Item));

	// receive broadcast message from initiator about other robot, station information
	// match-5
	printMsg("At match-5 step begin");
	recv_msg = read(sock, robots, robot_count * sizeof(Robot));
	recv_msg = read(sock, stations, station_count * sizeof(Station));
	recv_msg = read(sock, items, item_count * sizeof(Item));
	printMsg("At match-5 step end");

	printMsg(to_string(myDetails.robotId) + ":robot.cpp:connectToInitiator() -> receive broadcast information from initiator");

	printRobotInfo(robots, robot_count);
	printStationInfo(stations, station_count);
	printItemInfo(items, item_count);


	initializeGrid();
	cout << "here\n";
	printGrid();

	close(sock);

}

/*
Function to get details (pre-defined) about the picking station
*/
// struct Station* getStationInfo() {
//
//   int no_of_stations;
//
//   cout << "Enter Number of Stations in the Grid: ";
//   cin >> no_of_stations;
//
//   struct Station* stations;
//   stations = (struct Station *)malloc(no_of_stations*sizeof(struct Station));
//
//   int i;
//   for(i=0;i<no_of_stations;i++) {
//
//     cout << "Enter Station ID: ";
//     cin >> stations[i]->stationId;
//     cout << "Enter Station PortNo: ";
//     cin >> stations[i]->networkInfo.portNo;
//
//   }
//
//   return stations;
// }


int main() {

	// get robot id provided from station

	// myDetails.state = PASSIVE;

	thread th1(createServer);
	th1.join();



	return 0;
}

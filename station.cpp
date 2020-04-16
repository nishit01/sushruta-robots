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
struct Station myDetails;
struct Order order;
int robot_count;
int station_count;
int item_count;
Robot* robots;
Station* stations;
Item* items;
int grid_size;
int** grid;
bool isDone = false;


// Function Prototypes
void createServer();
void connectToInitiator();
void BroadCastOrderInfoToRobots(Order order);


/*
Function that will be running as separate thread
It is server listening port
*/
void createServer() {

	int portNo;

	portNo = generatePortNo();
	myDetails.networkInfo.portNo = portNo;

//  printMsg("station.cpp:createServer() -> Listening on Port " + to_string(portNo));

	int server_fd, new_socket;
	int opt = 1;
	int recv_msg;

	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// create socket description
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("station.cpp:createServer() -> socket descriptor failed\n");
		exit(EXIT_FAILURE);
	}


	// socket settings
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("station.cpp:createServer() -> setsockopt failed\n");
		exit(EXIT_FAILURE);
	}


	// create socket with port number as PortNo
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portNo);

	// bind socket with the ipv4 address and port no
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("station.cpp:createServer() -> bind failed\n");
		exit(EXIT_FAILURE);
	}


	// listen to port
	if (listen(server_fd, 1000) < 0) {
		perror("station.cpp:createServer() -> listen failed\n");
		exit(EXIT_FAILURE);
	}

	// printStationInfo(&myDetails);
	thread th1(connectToInitiator);
	th1.detach();
	while (1) {

		// accept new connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("station.cpp:createServer() -> accept failed\n");
			exit(EXIT_FAILURE);
		}

		int msg;
		recv_msg = read(new_socket, &msg, sizeof(msg));
		if (msg == 1) {
			cout << "New Station is Connected\n";
		} else {
			cout << "New Robot is Connected\n";
		}
		msg = -1;
		send(new_socket, &msg, sizeof(int), 0);
		cout << "Sent Welcome Message\n";

		close(new_socket);
	}
}

int connectToRobot(int robotPortNo) {
	int sock;
	struct sockaddr_in serv_addr;
	int recv_msg;
	int msg;

	// create socket descriptor
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("station.cpp:connectToRobot() -> socket descriptor failed\n");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(robotPortNo);

	// convert ipv4 address from text to binary info
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("station.cpp:connectToRobot() -> invalid address not supported\n");
		exit(EXIT_FAILURE);
	}

	// connect to server 127.0.0.1:robotPortNo
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("station.cpp:connectToRobot() -> connection failed\n");
		exit(EXIT_FAILURE);
	}
	return sock;
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

	// send initial message about being a station
	msg = STATION;
	// match-1
	send(sock, &msg, sizeof(int), 0);

	// receive station id from station
	// match-2
	recv_msg = read(sock, &myDetails.stationId, sizeof(int));
	// printStationInfo(&myDetails, 1);


	// send myDetails i.e. struct Station
	// match-3
	send(sock, &myDetails.networkInfo.portNo, sizeof(int), 0);
	recv_msg = read(sock, &myDetails, sizeof(Station));
	printStationInfo(&myDetails, 1);

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
	int val;
	recv_msg = read(sock, robots, robot_count * sizeof(Robot));
	recv_msg = read(sock, stations, station_count * sizeof(Station));
	recv_msg = read(sock, items, item_count * sizeof(Item));
	printMsg("At match-5 step end");

	printMsg(to_string(myDetails.stationId) + ":station.cpp:connectToInitiator() -> received broadcast information from initiator");

	printRobotInfo(robots, robot_count);
	printStationInfo(stations, station_count);

	initializeGrid();
	printGrid();

	close(sock);

	isDone = true;
}



void BroadCastOrderInfoToRobots(Order order) {
	int i;
	int msg = STATION;
	for (i = 0; i < robot_count; i++) {
		int sock = connectToRobot(robots[i].networkInfo.portNo);
		cout << "Connected to Robot" << endl;
		send(sock, &msg, sizeof(int), 0);
		send(sock, &order, sizeof(Order), 0);
		close(sock);
		printMsg("station.cpp:BroadCastOrderInfoToRobots() -> send broadcast message to robot " + to_string(i));
	}

}



/*
Function to Place Order from Customer
*/
void placeOrder() {

}


/*
Function to Show List of Items
*/
void showItems() {

}



/*
Dummy Function for Items
*/
struct Item* insertItems() {
	struct Item items[5];
	struct Item *tmp;
	int i;
	for (i = 0; i < 5; i++) {
		tmp = createItem(i + 1, 10, i + 1, i + 1);
		items[i].itemId = tmp->itemId;
		items[i].currentCount = tmp->currentCount;
		items[i].coords = tmp->coords;
		//placeItemOnGrid(grid, items[i].coords);
	}
}


int main() {

	srand(time(0));
//  myDetails.stationId = generateStationId();
//  cout << "My Station ID: " << myDetails.stationId << "\n";

//  insertItems();
	isDone = false;

	myDetails.orderQueue = 10;
	myDetails.exitQueue = 10;

	thread th1(createServer);
	th1.detach();

	while (!isDone);

	Order order;
	order.stationId = myDetails.stationId;
	BroadCastOrderInfoToRobots(order);

	// th1.join();
	while (1);
	return 0;

}

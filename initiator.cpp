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

int robot_count = 0;
int station_count = 0;
int item_count = 0;
Robot* robots;
Station* stations;
Item* items;
int grid_size;
int **grid;


// Function Prototype
void sendBroadcastInfoToStation(int *);
void sendBroadcastInfoToRobot(int *);
void createItems();


/* Function that inserts items */
void createItems() {
	int i;
	int n;
	cout << "Enter Number of Items to be inserted: ";
	cin >> item_count;
	n = item_count;

	// items = (Item *)malloc(item_count*sizeof(Item));
	// Item items[n];
	//Coords coords;

//  items = (struct Item *)malloc(n*sizeof(struct Item));
	items = new Item[n];

	for (i = 0; i < n; i++) {
		cout << "Enter Item Name: ";
		cin >> items[i].name;
		cout << "Enter Item Quantity: ";
		cin >> items[i].currentCount;
		cout << "Enter location of item in grid: ";
		cin >> items[i].coords.x >> items[i].coords.y;
		items[i].itemId = i;
		placeOnGrid(items[i].coords, ITEM);
		printMsg("initiator.cpp:createItem() -> Item-" + to_string(items[i].itemId) + " successfully placed");
		printMsg("==============================================================");
	}
	// printGrid();
}



void placeRobots() {

	int i;
	Coords coord;
//  cout << "in place robots\n";

//  cout << "size " << (sizeof(robots)/sizeof(robots[0])) << "\n";

	for (i = 0; i < robot_count; i++) {
		robots[i].robotId = i;
		robots[i].state = PASSIVE;
		cout << "enter coords for robot-" << robots[i].robotId << ": ";
		cin >> coord.x >> coord.y;
		robots[i].currentCoords = coord;
		placeOnGrid(coord, ROBOT);
		printMsg("initiator.cpp:placeRobots() -> Robot-" + to_string(robots[i].robotId) + " successfully placed");
		printMsg("==============================================================");
	}

}



void placeStation() {

	int i;
	Coords coord;

	for (i = 0; i < station_count; i++) {

		stations[i].stationId = i;
		stations[i].orderQueue = 10;
		stations[i].exitQueue = 10;

		cout << "enter coords for station-" << stations[i].stationId << ": ";
		cin >> coord.x >> coord.y;
		stations[i].coords = coord;
		placeOnGrid(coord, STATION);
		printMsg("initiator.cpp:placeStation() -> Station-" + to_string(stations[i].stationId) + " successfully placed");
		printMsg("==============================================================");
	}
}


void sendBroadcastInfoToStation(int* station_socket_info) {
	int i;
	for (i = 0; i < station_count; i++) {
		send(station_socket_info[i], robots, robot_count * sizeof(Robot), 0);
		send(station_socket_info[i], stations, station_count * sizeof(Station), 0);
		send(station_socket_info[i], items, item_count * sizeof(Item), 0);
		printMsg("initiator.cpp:sendBroadcastInfoToStation() -> send broadcast message to station " + to_string(i));
	}
}


void sendBroadcastInfoToRobot(int* robot_socket_info) {
	int i;
	for (i = 0; i < robot_count; i++) {
		send(robot_socket_info[i], robots, robot_count * sizeof(Robot), 0);
		send(robot_socket_info[i], stations, station_count * sizeof(Station), 0);
		send(robot_socket_info[i], items, item_count * sizeof(Item), 0);
		printMsg("initiator.cpp:sendBroadcastInfoToStation() -> send broadcast message to robot " + to_string(i));
	}
}

/* function to present the initial grid scenario */
void sendBroadcastToAll(int* station_socket_info, int* robot_socket_info) {
	int i;
	for (i = 0; i < robot_count; i++) {

	}
	for (i = 0; i < station_count; i++) {

	}
}


/*
Funtion to create server thread that will perform the initial communication in the system
*/
void createServer() {

	int server_fd, new_socket;
	int opt = 1;
	int recv_msg, msg;
	int portNo = 9000;

	int robot_socket_info[robot_count];
	int station_socket_info[station_count];


	Robot robot;
	Station station;

	int robot_index = 0;
	int station_index = 0;

	int robotId;
	int stationId;


	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// create socket descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("initiator.cpp:createServer() -> socket descriptor failed\n");
		exit(EXIT_FAILURE);
	}

	// socket settings
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int))) {
		perror("initiator.cpp:createServer() -> setsockopt failed\n");
		exit(EXIT_FAILURE);
	}

	// create socket with port number as port no
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portNo);

	// bind socket with the ipv4 address and port no
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("initiator.cpp:createServer() -> bind failed\n");
		exit(EXIT_FAILURE);
	}

	// listen to port
	if (listen(server_fd, 1000) < 0) {
		perror("initiator.cpp:createServer() -> listen failed\n");
		exit(EXIT_FAILURE);
	}

	printMsg("===== Initiator =====");
	printMsg("Connected to Port: " + to_string(portNo));
	printMsg("Waiting for stations/robots to connect");

	// connect with robots and stations and send id to them
	while (1) {

		// accept new connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("initiator.cpp:createServer() -> accept failed\n");
			exit(EXIT_FAILURE);
		}

		// receive first message from robot/station to discover its identity i.e. whether its robot/station
		// match-1
		recv_msg = read(new_socket, &msg, sizeof(int));

		if (msg == ROBOT) {

			robotId = robot_index;

			// send robot id information to the robot
			// match-2
			send(new_socket, &robotId, sizeof(int), 0);

			// receive server port details from the robot
			// match-3
			recv_msg = read(new_socket, &robot.networkInfo.portNo, sizeof(int));
			robots[robot_index].networkInfo.portNo = robot.networkInfo.portNo;
			send(new_socket, &robots[robot_index], sizeof(Robot), 0);


			// send robot, station count information
			// match-4
			send(new_socket, &robot_count, sizeof(int), 0);
			send(new_socket, &station_count, sizeof(int), 0);
			send(new_socket, &item_count, sizeof(int), 0);
			send(new_socket, &grid_size, sizeof(int), 0);


			printMsg("===== New Robot Connected =====");
			printMsg("Robot ID: " + to_string(robotId));
			printMsg("Robot Server Port: " + to_string(robot.networkInfo.portNo));
			printMsg("==================================================================");
			printRobotInfo(&robots[robot_index], 1);


			// save robot socket info for sending broadcast message later on
			robot_socket_info[robot_index] = new_socket;

			robot_index++;

			if (robot_index == robot_count) {

				//sendBroadcastInfoToRobot(robots, robot_count);
				// socket info, msg
				//sendBroadcastInfoToRobot(robot_socket_info);
				printMsg("initiator.cpp:createServer() -> Waiting for Station to get Connected");

			}
		}

		else {

			int stationId = station_index;

			// send station id information to the station
			// match-2
			send(new_socket, &stations[station_index].stationId, sizeof(int), 0);


			// receive station details i.e. struct Station
			// match-3
			recv_msg = read(new_socket, &station.networkInfo.portNo, sizeof(int));
			stations[station_index].networkInfo.portNo = station.networkInfo.portNo;
			send(new_socket, &stations[station_index], sizeof(Station), 0);


			// send robot, station count information
			// match-4
			send(new_socket, &robot_count, sizeof(int), 0);
			send(new_socket, &station_count, sizeof(int), 0);
			send(new_socket, &item_count, sizeof(int), 0);
			send(new_socket, &grid_size, sizeof(int), 0);


			printMsg("=================== New Station Connected ========================");
			printMsg("Station ID: " + to_string(stations[station_index].stationId));
			printMsg("Station Server Port: " + to_string(stations[station_index].networkInfo.portNo));
			printMsg("===================================");
			printStationInfo(&stations[station_index], 1);
			printMsg("==================================================================");

			// save station socket information for later use
			station_socket_info[station_index] = new_socket;

			station_index++;

			if (station_index == station_count) {
				// match-5
				// sendBroadcastInfoToStation(station_socket_info);
				printMsg("initiator.cpp:createServer() -> Waiting for Robots to get Connected");
			}
		}

		if (robot_index == robot_count && station_index == station_count) {

			sendBroadcastInfoToRobot(robot_socket_info);
			sendBroadcastInfoToStation(station_socket_info);

			printRobotInfo(robots, robot_count);
			printStationInfo(stations, station_count);
			printGrid();

			printMsg("initiator.cpp:createServer() -> send broadcast message to all robots/stations ..... ");
			printMsg("initiator.cpp:createServer() -> System is online now ....");

			break;

		}
	}
	// need to find proper way to close the socket connection
	// close(socket);
}



int main() {

	robot_count = 4;
	station_count = 2;
//  item_count = 5;
	grid_size = 10;

	//int grid[10][10];
	grid = (int **)malloc(grid_size * sizeof(int *));
	int i;
	for (i = 0; i < grid_size; i++)
		grid[i] = (int *)malloc(grid_size * sizeof(int));

	//robots = (Robot *)malloc(robot_count*sizeof(Robot));
	robots = new Robot[robot_count];
	stations = (Station *)malloc(station_count * sizeof(Station));
	// items = (Item *)malloc(item_count*sizeof(Item));


	createItems();
	placeRobots();
	placeStation();

	printRobotInfo(robots, robot_count);
	printStationInfo(stations, station_count);
	printGrid();



	thread th1(createServer);
	th1.join();

	return 0;
}

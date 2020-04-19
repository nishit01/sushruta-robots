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
int robot_count;
int station_count;
int item_count;
Robot* robots;
Station* stations;
Item* items;
int grid_size;
int** grid;


// Function Prototypes
void createServer();
void connectToInitiator();
void placeOrder();
void showItems(Item* items);
//void insertItems();



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
  th1.join();

  // order placing thread
  thread th2(placeOrder);
  th2.detach();

  printMsg("station.cpp:createServer() -> " + to_string(myDetails.stationId) + " is up and running ");
  while(1) {

    // accept new connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
      perror("station.cpp:createServer() -> accept failed\n");
      exit(EXIT_FAILURE);
    }

    int msg;
    recv_msg = read(new_socket, &msg, sizeof(msg));
    if (msg == 1) {
      cout << "New Station is Connected\n";
    }
    else {
      cout << "New Robot is Connected\n";
    }
    msg = -1;
    send(new_socket, &msg, sizeof(int), 0);
    cout << "Sent Welcome Message\n";

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


  robots = (Robot *)malloc(robot_count*sizeof(Robot));
  stations = (Station *)malloc(station_count*sizeof(Station));
  items = (Item *)malloc(item_count*sizeof(Item));

  // receive broadcast message from initiator about other robot, station information
  // match-5
  printMsg("At match-5 step begin");
  int val;
  recv_msg = read(sock, robots, robot_count*sizeof(Robot));
  recv_msg = read(sock, stations, station_count*sizeof(Station));
  recv_msg = read(sock, items, item_count*sizeof(Item));
  printMsg("At match-5 step end");

  printMsg(to_string(myDetails.stationId) + ":station.cpp:connectToInitiator() -> received broadcast information from initiator");

//  printRobotInfo(robots, robot_count);
//  printStationInfo(stations, station_count);
//  printItemInfo(items);

  initializeGrid();
//  printGrid();

  // thread th1(showItems);
  // th1.detach();

  close(sock);

}


/*
Function to Show List of Items
*/
void showItems(Item* items) {
	int i;
	printMsg("========= Item Details =========");
	for(i=0;i<item_count;i++) {
		printMsg(to_string(i) + ". " + string(items[i].name));
	}
}




/*
 * Function to place order from customer
 */
void placeOrder() {

	int itemId;
	Item item;

	while(1) {
		showItems(items);
//		printMsg("Enter Item-ID to place order for: ");
		cout << "Enter Item-ID to place order for: ";
		cin >> itemId;
		if (itemId < 0 || itemId >= itemCount)
			continue;
		item = items[itemId];
		printMsg("Item Name: " + string(items[itemId].name));
		printMsg("Item ID: " + to_string(items[itemId].itemId));
		printMsg("Item Location: " + to_string(items[itemId].coords.x) + " " + to_string(items[itemId].coords.y));
		printMsg("Order Successfully Placed");
		broadcastOrderMessageToRobots();
	}
}



/*
 * Function to broadcast order details to all the robots
 */
void broadcastOrderMessageToRobots() {

}






/*
Dummy Function for Items
*/
//struct Item* insertItems() {
//  struct Item items[5];
//  struct Item *tmp;
//  int i;
//  for(i=0;i<5;i++) {
//    tmp = createItem(i+1, 10, i+1,i+1);
//    items[i].itemId = tmp->itemId;
//    items[i].currentCount = tmp->currentCount;
//    items[i].coords = tmp->coords;
//    //placeItemOnGrid(grid, items[i].coords);
//  }
//}


int main() {

  srand(time(0));
//  myDetails.stationId = generateStationId();
//  cout << "My Station ID: " << myDetails.stationId << "\n";

//  insertItems();


  myDetails.orderQueue = 10;
  myDetails.exitQueue = 10;

  thread th1(createServer);
  th1.join();

//  struct Item items[5];

  return 0;

}

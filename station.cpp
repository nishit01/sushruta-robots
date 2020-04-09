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

// Global Variables
struct Station myDetails;


// Function Prototypes





/*
Function that will be running as separate thread
It is server listening port
*/
void createServer() {

  int portNo;

  portNo = generatePortNo();
  myDetails.networkInfo.portNo = portNo;


  cout << "My Port No. " << portNo << "\n";

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


  cout << "Station is ready to listen to any messages\n";

  printStationInfo(&myDetails);

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
  int i;
  for(i=0;i<5;i++) {
    items[i].itemId = i;
    items[i].currentNo = 10;
//    items[i].Coords = getCoords(i+1,i+1);
  }
}


int main() {

  srand(time(0));
  myDetails.stationId = generateStationId();
  cout << "My Station ID: " << myDetails.stationId << "\n";

  thread th1(createServer);
  th1.join();

  struct Item items[5];

}

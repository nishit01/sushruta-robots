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
int robot_count = 0;
int station_count = 0;




void createServer() {

  int server_fd, new_socket;
  int opt = 1;
  int recv_msg;
  int portNo = 9000;

  Robot robots[robot_count];
  Station stations[station_count];

  int robot_index = 0;
  int station_index = 0;


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

  // connect with robots and stations and send id to them
  while(1) {

    // accept new connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
      perror("initiator.cpp:createServer() -> accept failed\n");
      exit(EXIT_FAILURE);
    }

    int msg;
    recv_msg = recv(new_socket, &msg, sizeof(int));

    if (msg == ROBOT) {

      int robotId = generateRobotId();

      // send robot id information to the robot
      send(new_socket, &robotId, sizeof(int), 0);

      // receive server port details from the robot
      Robot robot;
      recv_msg = read(new_socket, &robot, sizeof(Robot));

      robots[robot_index] = robot;
      robot_index++;

      if (robot_index == robot_count) {
        sendBroadcastInfoToRobot(robots, robot_count);
        printMsg("initiator.cpp:createServer() -> send broadcast information to all robots");
      }

    }

    else {
      int stationId = generateStationId();

      // send station id information to the station
      send(new_socket, &stationId, sizeof(int), 0);

      // receive server port details from the robot
      Station station;
      recv_msg = read(new_socket, &station, sizeof(Station));

      stations[station_index] = station;
      station_index++;

      if (station_index == station_count) {
        sendBroadcastInfoToStation(stations, station_count);
        printMsg("initiator.cpp:createServer() -> send broadcast information to all stations");
      }
    }

    if (robot_index == robot_count && station_index == station_count) {
      printMsg("initator.cpp:createServer() -> System is online now ..... ");
      break;
    }
  }
  close(socket);
}


int main() {
  robot_count = 4;
  station_count = 2;
  return 0;
}

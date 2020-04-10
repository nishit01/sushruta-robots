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
Robot* robots;
Station* stations;


void sendBroadcastInfoToStation(int* station_socket_info) {
  int i;
  for(i=0;i<station_count;i++) {
    send(station_socket_info[i], robots, robot_count*sizeof(Robot), 0);
    send(station_socket_info[i], stations, station_count*sizeof(Station), 0);
    printMsg("initiator.cpp:sendBroadcastInfoToStation() -> send broadcast message to station " + to_string(i));
  }
}


void sendBroadcastInfoToRobot(int* robot_socket_info) {
  int i;
  for(i=0;i<robot_count;i++) {
    send(robot_socket_info[i], robots, robot_count*sizeof(Robot), 0);
    send(robot_socket_info[i], stations, station_count*sizeof(Station), 0);
    printMsg("initiator.cpp:sendBroadcastInfoToStation() -> send broadcast message to robot " + to_string(i));
  }
}


/*
Funtion to create server thread that will perform the initial communication in the system
*/
void createServer() {

  int server_fd, new_socket;
  int opt = 1;
  int recv_msg,msg;
  int portNo = 9000;

  int robot_socket_info[robot_count];
  int station_socket_info[station_count];

  robots = (Robot *)malloc(robot_count*sizeof(Robot));
  stations = (Station *)malloc(station_count*sizeof(Station));

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
  while(1) {

    // accept new connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
      perror("initiator.cpp:createServer() -> accept failed\n");
      exit(EXIT_FAILURE);
    }

    // receive first message from robot/station to discover its identity i.e. whether its robot/station
    // match-1
    recv_msg = read(new_socket, &msg, sizeof(int));

    if (msg == I_AM_ROBOT) {

      robotId = robot_index;

      // send robot id information to the robot
      // match-2
      send(new_socket, &robotId, sizeof(int), 0);

      // receive server port details from the robot
      // match-3
      recv_msg = read(new_socket, &robot, sizeof(Robot));


      // send robot, station count information
      // match-4
      send(new_socket, &robot_count, sizeof(int), 0);
      send(new_socket, &station_count, sizeof(int), 0);


      printMsg("===== New Robot Connected =====");
      printMsg("Robot ID: " + to_string(robotId));
      printMsg("Robot Server Port: " + to_string(robot.networkInfo.portNo));
      printMsg("==================================================================");

      robots[robot_index] = robot;

      // save robot socket info for sending broadcast message later on
      robot_socket_info[robot_index] = new_socket;

      robot_index++;

      if (robot_index == robot_count) {
        //sendBroadcastInfoToRobot(robots, robot_count);
        // socket info, msg
        sendBroadcastInfoToRobot(robot_socket_info);
        printMsg("initiator.cpp:createServer() -> send broadcast information to all robots");
      }

    }

    else {
      int stationId = station_index;

      // send station id information to the station
      // match-2
      send(new_socket, &stationId, sizeof(int), 0);


      // receive station details i.e. struct Station
      // match-3
      recv_msg = read(new_socket, &station, sizeof(Station));


      // send robot, station count information
      // match-4
      send(new_socket, &robot_count, sizeof(int), 0);
      send(new_socket, &station_count, sizeof(int), 0);


      printMsg("===== New Station Connected =====");
      printMsg("Station ID: " + to_string(stationId));
      printMsg("Station Server Port: " + to_string(station.networkInfo.portNo));
      printMsg("==================================================================");

      stations[station_index] = station;

      // save station socket information for later use
      station_socket_info[station_index] = new_socket;

      station_index++;

      if (station_index == station_count) {
        // match-5
        sendBroadcastInfoToStation(station_socket_info);
        printMsg("initiator.cpp:createServer() -> send broadcast information to all stations");
      }
    }

    if (robot_index == robot_count && station_index == station_count) {
      //sendBroadcastInfoToRobot(robot_socket_info);
      //sendBroadcastInfoToStation(station_socket_info);
      printMsg("initator.cpp:createServer() -> System is online now ..... ");
      break;
    }
  }
  // need to find proper way to close the socket connection
  // close(socket);
}


int main() {

  robot_count = 4;
  station_count = 2;

  thread th1(createServer);
  th1.join();

  return 0;
}

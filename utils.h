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

using namespace std;

#define ACTIVE 1
#define PASSIVE 2
#define ROBOT 3
#define STATION 4
#define ITEM 5


/*
Station ID will be in range from 100-199
Robot ID will be in range from 1000-1999
*/

struct Network {
  int portNo;
  struct sockaddr_in address;
};

struct Coords {
  int x,y;
};


struct Item {
  int itemId;
  int currentNo;
  struct Coords;
};

struct Order {
  int orderId;
  int stationId;
  int* itemList;
};

struct Robot {
  int robotId;
  int state;
  struct Network networkInfo;
  struct Coords currentCoords;
//  struct robot* robotsInfo;
//  struct station* stationsInfo;
};

struct Station {
  int stationId;
  int orderQueue;
  int exitQueue;
  struct Network networkInfo;
//  struct Station* stationsInfo;
};

struct RouteInfo {
  int itemId;
  int distance;
  struct Coords* route;
};

struct DeliverItem {
  int itemId;
  int stationId;
  struct Coords itemCoords;
};


/*
Function to generate port no for robot and station
All port no will be 4 digit number starting with 9
MSG: GET_PORT_NO
*/
int generatePortNo() {

  srand(time(0));
  int tmp;
  int portNo;
  int i;

  portNo = 9;
  for(i=1;i<=3;i++) {
    tmp = rand()%10;
    portNo = portNo*10 + tmp;
  }

  return portNo;
}


bool checkRobotId(int id) {
  return true;
}


bool checkStationId(int id) {
  return true;
}

/*
Function to get Coords Structure in return
*/
struct Coords getCoords(int x, int y) {
  struct Coords coords;
  coords.x = x;
  coords.y = y;
  return coords;
}


/*
Function to generate robot id
All robot id will be in range from 1000-1999
MSG: GET_ROBOT_ID
*/
int generateRobotId() {
  int tmp;
  int robotId;
  int i;

  robotId = 1;
  for(i=1;i<=3;i++) {
    tmp = rand()%10;
    robotId = robotId*10 + tmp;
  }

  return robotId;
}




/*
Function to generate station id
All station id will be in range from 100-199
MSG: GET_STATION_ID
*/
int generateStationId() {
  int tmp;
  int stationId;
  int i;

  stationId = 1;
  for(i=1;i<=2;i++) {
    tmp = rand()%10;
    stationId = stationId*10 + tmp;
  }

  return stationId;
}

/*
Function to generate Order Id
All Order Id will be in range from 10000-20000
*/
int generateOrderId() {
  int tmp;
  int orderId;
  int i;

  orderId = 1;
  for(i=1;i<=4;i++) {
    tmp = rand()%10;
    orderId = orderId*10 + tmp;
  }

  return orderId;

}



/*
Function to print Robot Information
*/
void printRobotInfo(struct Robot* robot) {
  cout << "===== Robot Information =====\n";
  cout << "Robot ID: " << robot->robotId << "\n";
  cout << "State: " << robot->state << "\n";
  cout << "Port No: " << robot->networkInfo.portNo << "\n";
  cout << "Current Location: " << robot->currentCoords.x << " " << robot->currentCoords.y << "\n";
}

/*
Function to print Station Information
*/
void printStationInfo(struct Station* station) {
  cout << "===== Station Information =====\n";
  cout << "Station ID: " << station->stationId << "\n";
  cout << "Port No: " << station->networkInfo.portNo << "\n";
}

/*
Function to place items on the grid
*/
void placeItemOnGrid(vector<vector<int>> grid, struct Coords coords) {
  int x = coords.x;
  int y = coords.y;
  grid[x][y] = ITEM;
}


/*
Function to print Entire Grid showing robot, items, stations
*/
void printGrid(vector<vector<int>> grid) {
  int i,j,n;
  n = grid.size();
  for(i=0;i<n;i++) {
    for(j=0;j<n;j++) {
      cout << grid[i][j] << " ";
    }
    cout << "\n";
  }
}

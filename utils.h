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
#define ORDER 6
#define ROBOT_INFO 8
#define STATION_INFO 9


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
  char name[100];
  int currentCount;
  struct Coords coords;
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
  struct Coords coords;
  struct Network networkInfo;
//  struct Station* stationsInfo;
};

struct RouteInfo {
  int itemId;
  int distance;
  struct Coords route;
};

struct DeliverItem {
  int itemId;
  int stationId;
  struct Coords itemCoords;
};

// global Variables
//int grid[20][20];
extern int** grid;
extern int grid_size;
extern int robot_count;
extern int station_count;
extern int item_count;
extern Robot* robots;
extern Station* stations;
extern Item* items;

/* Function Prototype */
int generatePortNo();


int generateRobotId();
int generateStationId();

void printRobotInfo(Robot*, int);
void printStationInfo(Station*, int);

void initializeGrid();
void printMsg(string str);

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
Function to place items on the grid
*/
void placeOnGrid(Coords coords, int identity) {
  int x = coords.x;
  int y = coords.y;
  grid[x][y] = identity;
}


/*
 * Function to get Item Details
 */



void initializeGrid() {
  int i,j;
  grid = (int **)malloc(grid_size*sizeof(int *));

  // allocating grid size
  for(i=0;i<grid_size;i++) {
    grid[i] = (int *)malloc(grid_size*sizeof(int));
  }

  // grid = new int[grid_size][grid_size];

  for(i=0;i<grid_size;i++) {
    for(j=0;j<grid_size;j++) {
      grid[i][j] = 0;
    }
  }

  // placing robots
  for(i=0;i<robot_count;i++) {
    placeOnGrid(robots[i].currentCoords, ROBOT);
  }

  // placing stations
  for(i=0;i<station_count;i++) {
    placeOnGrid(stations[i].coords, STATION);
  }

  // placing item
  for(i=0;i<item_count;i++) {
    placeOnGrid(items[i].coords, ITEM);
  }

  printMsg("initializeGrid() -> all robots, stations and items successfully placed");
}


void printItemInfo(Item* items) {
  int i;
  for(i=0;i<item_count;i++) {
    printMsg("Item " + to_string(items[i].itemId));
    printMsg("Item Name " + string(items[i].name));
    printMsg("Item Location " + to_string(items[i].coords.x) + " " + to_string(items[i].coords.y));
  }
}


/*
Function to get Coords Structure in return
*/
// struct Coords getCoords(int x, int y) {
//   struct Coords coords;
//   coords.x = x;
//   coords.y = y;
//   return coords;
// }


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
void printRobotInfo(struct Robot* robot, int n) {
  int i;
  cout << "===== Robot Information =====\n";
  for(i=0;i<n;i++) {
    cout << "Robot ID: " << robot[i].robotId << "\n";
    cout << "State: " << robot[i].state << "\n";
    cout << "Port No: " << robot[i].networkInfo.portNo << "\n";
    cout << "Current Location: " << robot[i].currentCoords.x << " " << robot[i].currentCoords.y << "\n";
  }
}

/*
Function to print Station Information
*/
void printStationInfo(struct Station* station, int n) {
  int i;
  cout << "===== Station Information =====\n";
  for(i=0;i<n;i++) {
    cout << "Station ID: " << station[i].stationId << "\n";
    cout << "Port No: " << station[i].networkInfo.portNo << "\n";
    cout << "Location: " << station[i].coords.x << " " << station[i].coords.y << "\n";
  }
}




/*
Function to print Entire Grid showing robot, items, stations
*/
void printGrid() {
  int i,j,n;
  n = grid_size;
  for(i=0;i<n;i++) {
    for(j=0;j<n;j++) {
      cout << grid[i][j] << " ";
    }
    cout << "\n";
  }
}


/*
Function to generate Item ID
Should be in range of 5000-6000
*/
int generateItemId() {
  int tmp;
  int itemId;
  int i;

  itemId = 5;
  for(i=1;i<=3;i++) {
    tmp = rand()%10;
    itemId = itemId*10 + tmp;
  }

  return itemId;
}

struct Coords setCoords(int x, int y) {
  // struct Coords* coords = (struct Coords *)malloc(sizeof(struct Coords));
  Coords coords;
  coords.x = x;
  coords.y = y;
  return coords;
}

void printMsg(string str) {
  cout << str << "\n";
}


/*
Function to create Item
*/
struct Item* createItem(int itemId, int count, int x, int y) {

  struct Item* item = NULL;
  item = (struct Item *)malloc(sizeof(struct Item));

  item->itemId = itemId;
  item->currentCount = count;
  item->coords = setCoords(x, y);

  return item;

}

/*
Function to set stock of a particular item
*/
void setItemCount(struct Item* item, int count) {
  item->currentCount = count;
}

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
#define BLOCK_CELL 7
#define ROBOT_INFO 8
#define STATION_INFO 9
#define REQUEST 10
#define REPLY 11
#define RELEASE 12
#define ORDER_REQUEST 13
#define ORDER_REPLY 14
#define ORDER_RELEASE 15
#define FREE_CELL 0

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
//  int* itemList;
  int itemId;
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
//  int itemId;
  int distance;
  int robotId;
  vector<pair<int, int>> path1; // robot to item
  vector<pair<int, int>> path2; // item to station
  // struct Coords route;
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
//extern bool isvalid(pair<int, int>);


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



/*
 * Function to send message to station
 */
void sendMsgToStation(int station_port_no, vector<int> msgs) {

	int i;
	int sock;
	int recv_msg;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("utils.h:sendMsgToStation() ... socket description failed\n");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(station_port_no);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("utils.h:sendMsgToStation() ... invalid address\n");
		exit(EXIT_FAILURE);
	}

	// connect to other station' server thread
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
	  perror("utils.h:sendMsgToStation() ... connection failed\n");
	  exit(EXIT_FAILURE);
	}


	// send messages
	int n = msgs.size();

	// send n to inform station about the number of messages that will be transferred
	// send(sock, &n, sizeof(int), 0);

	for(i=0;i<n;i++) {
		send(sock, &msgs[i], sizeof(int), 0);
	}

	// printMsg("utils.h:sendMsgToStation() ... completed sending message to station " + to_string(myDetails.networkInfo.portNo));

	close(sock);

}

/*
 * Function to send message to robot
 */
void sendMsgToRobot(int robot_port_no, vector<int> msgs) {

	int i;
	int sock;
	int recv_msg;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("utils.h:sendMsgToRobot() ... socket description failed\n");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(robot_port_no);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("utils.h:sendMsgToRobot() ... invalid address\n");
		exit(EXIT_FAILURE);
	}

	// connect to other station' server thread
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
	  perror("utils.h:sendMsgToRobot() ... connection failed\n");
	  exit(EXIT_FAILURE);
	}


	// send messages
	int n = msgs.size();

	// send n to inform station about the number of messages that will be transferred
	// send(sock, &n, sizeof(int), 0);

	for(i=0;i<n;i++) {
		send(sock, &msgs[i], sizeof(int), 0);
	}

	// printMsg("utils.h:sendMsgToStation() ... completed sending message to station " + to_string(myDetails.networkInfo.portNo));

	close(sock);

}


bool isvalid(pair<int, int> next_point) {
	if (next_point.first >= 0 && next_point.first < grid_size && next_point.second >= 0 && next_point.second < grid_size
	        && grid[next_point.first][next_point.second] == 0) {
		return true;
	}
	return false;
}

/*
 * Function to read time
 */
long readTime() {
  std::chrono::nanoseconds ms = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());
  long tmp = ms.count();
  return tmp;
}


/*
 * Function to broadcast messages to all robots
 */
void broadcastMsgToRobot(vector<int> msgs) {
	int i;
	for(i=0;i<robot_count;i++) {
		sendMsgToRobot(robots[i].networkInfo.portNo, msgs);
	}
}

/*
 * Function to broadcast messages to all stations
 */
void broadcastMsgToStation(vector<int> msgs) {
	int i;
	int n = msgs.size();
	for(i=0;i<station_count;i++) {
		sendMsgToStation(stations[i].networkInfo.portNo, msgs);
	}
}



/*
 * Function to check whether the coordinate is valid coordinate
 */
//bool isvalid(pair<int, int> next_point) {
//	if (next_point.first >= 0 && next_point.first < grid_size && next_point.second >= 0 && next_point.second < grid_size
//	        && grid[next_point.first][next_point.second] == 0) {
//		return true;
//	}
//
//
//	return false;
//}


/*
 * Function to find the route from robot to item
 */
vector<pair<int, int>> getPredecessor(pair<int, int> src, pair<int, int> dest, vector<pair<int, int>> pred) {
	vector<pair<int, int>> path;
	queue<pair<int, int>> q;
	q.push(src);
	set<pair<int, int>> seen_points;
	seen_points.insert(src);
	while (!q.empty()) {
		pair<int, int> current_point = q.front();
		q.pop();
		// cout << "current_point: " << current_point.first << " " << current_point.second << endl;
		if (current_point == dest) {
			return pred;
		}
		list<pair<int, int>> list_next = {
			make_pair(current_point.first + 1, current_point.second), make_pair(current_point.first - 1, current_point.second),
			make_pair(current_point.first, current_point.second + 1), make_pair(current_point.first, current_point.second - 1)
		};
		for (auto next_point : list_next) {
			// cout << "Point to be seen: " << next_point.first << " " << next_point.second << endl;
			if ((isvalid(next_point) || next_point == src || next_point == dest) &&
			        find(seen_points.begin(), seen_points.end(), next_point) == seen_points.end()) {
				// cout << "Pushing it in!" << endl;
				q.push(next_point);
				pred[(next_point.first * 10 + next_point.second)] = current_point;
				seen_points.insert(next_point);
			}
		}
	}
	return pred;
}



/*
 * Function to find the shortest path from robot to item
 */
vector<pair<int, int>> shortest(pair<int, int> src, pair<int, int> dest) {
	vector<pair<int, int>> pred(grid_size * grid_size);
	for (auto i = 0; i < grid_size * grid_size; i++)
		pred[i] = make_pair(-1, -1);
	pred = getPredecessor(src, dest, pred);
	vector<pair<int, int>> path;
	auto crawl = dest;
	path.push_back(crawl);
	while (pred[(crawl.first * grid_size + crawl.second)] != make_pair(-1, -1)) {
		path.push_back(pred[(crawl.first * grid_size + crawl.second)]);
		crawl = pred[(crawl.first * grid_size + crawl.second)];
	}
	reverse(path.begin(), path.end());
	return path;
}




void updateGrid(int msg, RouteInfo route) {

	vector<pair<int, int>> path1;
	vector<pair<int, int>> path2;

	path1 = route.path1;
	path2 = route.path2;
	pair<int, int> cell;
	int i;

	if (msg == BLOCK_CELL) {
		for(i=1;i<path1.size()-1;i++) {
			cell = path1[i];
			grid[cell.first][cell.second] = BLOCK_CELL;
		}
		for(i=1;i<path2.size();i++) {
			cell = path2[i];
			grid[cell.first][cell.second] = BLOCK_CELL;
		}
	} else {
		for(i=1;i<path1.size()-1;i++) {
			cell = path1[i];
			grid[cell.first][cell.second] = FREE_CELL;
		}
		for(i=1;i<path1.size();i++) {
			cell = path2[i];
			grid[cell.first][cell.second] = FREE_CELL;
		}
	}

	printGrid();
}

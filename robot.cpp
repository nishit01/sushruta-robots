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
int order_count;
Robot* robots;
Station* stations;
Item* items;
unordered_map<int, Order> orders;
unordered_map<int, RouteInfo> routes;
unordered_map<int, int> order_reply_count;
int grid_size;
int** grid;

// Function Prototype
void createServer();
void connectToInitiator();
void getPath(Order, vector<pair<int, int>>&, vector<pair<int, int>>&);
void computeRoute(Order);

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

    printMsg("robot.cpp:createServer() -> robot is up and running");

    while(1) {

      // accept new connection
      if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("robot.cpp:createServer() -> accept failed\n");
        exit(EXIT_FAILURE);
      }

      int msg;
      recv_msg = read(new_socket, &msg, sizeof(int));	// msg-1

      if (msg == ORDER_REQUEST) {

    	  Order order;

    	  recv_msg = read(new_socket, &order.orderId, sizeof(Order));
    	  recv_msg = read(new_socket, &order.stationId, sizeof(Order));
    	  recv_msg = read(new_socket, &order.itemId, sizeof(Order));

    	  orders[order.orderId] = order;
    	  order_count = order_count + 1;

    	  printMsg("robot.cpp:createServer() ... order request received from station " + to_string(order.stationId) + " for item " + to_string(order.itemId));
    	  computeRoute(order);

      }

      else if (msg == ORDER_REPLY) {

    	  int reply_robot_state;
    	  int reply_order_id;

    	  recv_msg = read(new_socket, &reply_order_id, sizeof(int));	// msg-2
    	  recv_msg = read(new_socket, &reply_robot_state, sizeof(int));	// msg-3
    	  order_reply_count[reply_order_id]++;

    	  if (reply_robot_state == PASSIVE) {

    		  // msg-4
    		  int reply_robot_id;
    		  recv_msg = read(new_socket, &reply_robot_id, sizeof(int));

    		  // msg-5
    		  int reply_distance1;
    		  read(new_socket, &reply_distance1, sizeof(int));

    		  // msg-6{...}
    		  vector<pair<int, int>> reply_path1;
    		  int i;
    		  int reply_path_x;
    		  int reply_path_y;
    		  for(i=0;i<=reply_distance1;i++) {
    			  read(new_socket, &reply_path_x, sizeof(int));
    			  read(new_socket, &reply_path_y, sizeof(int));
    			  reply_path1.push_back({reply_path_x, reply_path_y});
    		  }


    		  // msg-7
    		  int reply_distance2;
    		  read(new_socket, &reply_distance2, sizeof(int));

    		  vector<pair<int, int>> reply_path2;

    		  // msg-8{...}
    		  for(i=0;i<=reply_distance2;i++) {
    			  read(new_socket, &reply_path_x, sizeof(int));
    			  read(new_socket, &reply_path_y, sizeof(int));
    			  reply_path2.push_back({reply_path_x, reply_path_y});
    		  }

    		  int reply_distance = reply_distance1 + reply_distance2;

    		  if (routes[reply_order_id].distance > reply_distance ||
    				  (routes[reply_order_id].distance == reply_distance && routes[reply_order_id].robotId > reply_robot_id)) {
    			  routes[reply_order_id].distance = reply_distance;
    			  routes[reply_order_id].robotId = reply_robot_id;
    			  routes[reply_order_id].path1 = reply_path1;
    			  routes[reply_order_id].path2 = reply_path2;
    			  cout << "Found better robot " << reply_robot_id << " - distance - " << reply_distance << "\n";
    		  }

    	  }

    	  if (order_reply_count[reply_order_id] == robot_count) {
    		  cout << "Leader Elected " << routes[reply_order_id].robotId << "\n";
    		  cout << "Leader Distance Found " << routes[reply_order_id].distance << "\n";
    		  cout << "Path from Robot to Item " << routes[reply_order_id].path1.size() - 1<< "\n";
    		  cout << "Path from Item to Station " << routes[reply_order_id].path2.size() - 1<< "\n";
    		  updateGrid(BLOCK_CELL, routes[reply_order_id]);
    	  }

      }

      close(new_socket);
    }
}


void computeRoute(Order order) {

	if (myDetails.state == ACTIVE) {

		int msg;
		msg = ACTIVE; // this robot is currently active, can't participate

		routes[order.orderId].distance = -1; // implies active state and has not participated in this item delivery

		vector<int> msgs;

		msgs.push_back(ORDER_REPLY); // msg-1
		msgs.push_back(order.orderId); // msg-2
		msgs.push_back(ACTIVE); // msg-3

		broadcastMsgToRobot(msgs);

	}
	else {

		int msg;
		msg = PASSIVE;
//		RouteInfo route;

		vector<int> msgs;

		msgs.push_back(ORDER_REPLY);	// msg-1
		msgs.push_back(order.orderId); 	// msg-2
		msgs.push_back(PASSIVE);	// msg-3

		vector<pair<int, int>> path1;
		vector<pair<int, int>> path2;

		getPath(order, path1, path2);

		int distance1 = path1.size() - 1;
		int distance2 = path2.size() - 1;

		int distance = distance1 + distance2;

		routes[order.orderId].distance = distance;
		routes[order.orderId].robotId = myDetails.robotId;
		routes[order.orderId].path1 = path1;
		routes[order.orderId].path2 = path2;


		msgs.push_back(myDetails.robotId);	// msg-4

		msgs.push_back(distance1);	// msg-5 distance from robot to item

		for(auto d: path1) {	// msg-6{...} route from robot to item
			msgs.push_back(d.first);
			msgs.push_back(d.second);
		}

		msgs.push_back(distance2); // msg-7 distance from item to station

		for(auto d: path2) { // msg-8{...} route from item to station
			msgs.push_back(d.first);
			msgs.push_back(d.second);
		}

		broadcastMsgToRobot(msgs);

	}

}

//bool isvalid(pair<int, int> next_point) {
//	if (next_point.first >= 0 && next_point.first < grid_size && next_point.second >= 0 && next_point.second < grid_size
//	        && grid[next_point.first][next_point.second] == 0) {
//		return true;
//	}
//	return false;
//}

void getPath(Order order, vector<pair<int, int>>& path1, vector<pair<int, int>>& path2) {

	// from robot to item
	pair<int, int> src1 = {myDetails.currentCoords.x, myDetails.currentCoords.y};
	pair<int, int> dst1 = {items[order.itemId].coords.x, items[order.itemId].coords.y};

	cout << src1.first << ", " << src1.second << "\n";
	cout << dst1.first << ", " << dst1.second << "\n";

//	vector<pair<int, int>> path1 = shortest(src1, dst1);
	path1 = shortest(src1, dst1);
	int distance1 = path1.size() - 1;

	cout << "From Robot to Item - distance " << distance1 << "\n";
	for (auto tmp: path1)
		cout << tmp.first << ", " << tmp.second << "\n";

	// loop for 4 times

//	vector<pair<int, int>> path2; // final result direction
	vector<pair<int, int>> tmp;
	int min_length = INT_MAX;

	pair<int, int> src2 = dst1;
	pair<int, int> dst2 = { stations[order.stationId].coords.x, stations[order.stationId].coords.y };

	vector<pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, { -1, 0}};

	for (auto d : directions) {
		pair<int, int> new_dest = {dst2.first + d.first, dst2.second + d.second};
		if (isvalid(new_dest)) {
			vector<pair<int, int>> tmp = shortest(src2, new_dest);
			// cout << "Distance between src and dest is " << path.size() << endl;
			if (tmp.size() < min_length) {
				path2 = tmp;
				min_length = tmp.size();
			}
		}
	}

	int distance2 = path2.size() - 1;

	cout << "From Item to Station - distance " << distance2 << "\n";
	for(auto tmp: path2)
		cout << tmp.first << ", " << tmp.second << "\n";

	cout << "Total Distance " << distance1 + distance2 << "\n\n";
//	cout << "Path Shown\n";
}


/*
 * vector<pair<int, int>> shortest(pair<int, int> src, pair<int, int> dest) {
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

 * int main() {
	pair<int, int> src = {5, 0}, dest = {6, 7};
	int min_length = INT_MAX;
	vector<pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, { -1, 0}}, result_path;
	for (auto d : directions) {
		pair<int, int> new_dest = {dest.first + d.first, dest.second + d.second};
		if (isvalid(new_dest)) {
			vector<pair<int, int>> path = shortest(src, new_dest);
			cout << "Distance between src and dest is " << path.size() << endl;
			if (path.size() < min_length) {
				result_path = path;
				min_length = path.size();
			}
		}
	}
	cout << "Minimum path size: " << min_length << endl;
	for (auto x : result_path) {
		cout << x.first << "," << x.second << endl;
	}
	return 0;
}
 */

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

  robots = (Robot *)malloc(robot_count*sizeof(Robot));
  stations = (Station *)malloc(station_count*sizeof(Station));
  items = (Item *)malloc(item_count*sizeof(Item));

  // receive broadcast message from initiator about other robot, station information
  // match-5
  printMsg("At match-5 step begin");
  recv_msg = read(sock, robots, robot_count*sizeof(Robot));
  recv_msg = read(sock, stations, station_count*sizeof(Station));
  recv_msg = read(sock, items, item_count*sizeof(Item));
  printMsg("At match-5 step end");

  printMsg(to_string(myDetails.robotId) + ":robot.cpp:connectToInitiator() -> receive broadcast information from initiator");

  printRobotInfo(robots, robot_count);
  printStationInfo(stations, station_count);
//  printItemInfo(items);


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

	order_count = 0;

	thread th1(createServer);
	th1.join();



  return 0;
}

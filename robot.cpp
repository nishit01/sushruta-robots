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
unordered_map<int, vector<int>> order_reply_count;

vector<int> pending_order;
list<int> noroute_order;

int consensus_value_propose;
int consensus_value_agreed;
int consensus_value_disagreed_count;
vector<int> consensus_value_received;

std::atomic<bool> nextOrder (true);

int grid_size;
int** grid;
std::mutex mtx;
std::mutex mtx1;
std::mutex mtx2;
std::mutex mtx3;

// Function Prototype
void createServer();
void connectToInitiator();
void getPath(Order, vector<pair<int, int>>&, vector<pair<int, int>>&);
void computeRoute(Order);
void moveRobot(int);
void moveRobotToExitStand(int, int);
void proposeConsensusValue();
void processConsensusValue();
void processOrder();

/*
Function to create server thread
*/
void createServer() {

	cout << "-------------------------------begin createServer()----------------------------------------\n";

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
    	  routes[order.orderId].distance = INT_MAX;
    	  order_count = order_count + 1;

    	  printMsg("robot.cpp:createServer() ... order request received from station " + to_string(order.stationId) + " for item " + to_string(order.itemId));

    	  mtx.lock();
    	  {
    		  pending_order.push_back(order.orderId);
    		  push_heap(pending_order.begin(), pending_order.end(), greater<int>());
    	  }
    	  mtx.unlock();
//    	  proposeConsensusValue();
    	  // computeRoute(order);

//    	  thread th2(processOrder);
//    	  th2.detach();
//    	  cout << pending_order.front() << "\n";
//    	  mtx.lock();
//    	  {
//    		  cout << "total orders " << pending_order.size() << "\n";
//    	  }
//    	  mtx.unlock();
      }

      else if (msg == ORDER_REPLY) {

    	  int reply_robot_state;
    	  int reply_order_id;
    	  int isRouteExist;
    	  int reply_robot_id;
    	  int reply_distance1;
    	  int reply_distance2;
    	  int reply_distance;
    	  vector<pair<int, int>> reply_path1;
    	  vector<pair<int, int>> reply_path2;

    	  int reply_path_x;
    	  int reply_path_y;

    	  recv_msg = read(new_socket, &reply_order_id, sizeof(int));	// msg-2
    	  recv_msg = read(new_socket, &reply_robot_state, sizeof(int));	// msg-3
    	  recv_msg = read(new_socket, &isRouteExist, sizeof(int)); // msg-3.1
//    	  order_reply_count[reply_order_id]++;

    	  order_reply_count[reply_order_id].push_back(isRouteExist);

    	  if (reply_robot_state == PASSIVE && isRouteExist == ROUTE) {

    		  // msg-4
//    		  int reply_robot_id;
    		  recv_msg = read(new_socket, &reply_robot_id, sizeof(int));

    		  // msg-5
//    		  int reply_distance1;
    		  read(new_socket, &reply_distance1, sizeof(int));

    		  // msg-6{...}
//    		  vector<pair<int, int>> reply_path1;
    		  int i;
//    		  int reply_path_x;
//    		  int reply_path_y;

    		  for(i=0;i<=reply_distance1;i++) {	// as distance = path.size() - 1, hence loop runs till <= distance1
    			  read(new_socket, &reply_path_x, sizeof(int));
    			  read(new_socket, &reply_path_y, sizeof(int));
    			  reply_path1.push_back({reply_path_x, reply_path_y});
    		  }


    		  // msg-7
//    		  int reply_distance2;
    		  read(new_socket, &reply_distance2, sizeof(int));

//    		  vector<pair<int, int>> reply_path2;

    		  // msg-8{...}
    		  for(i=0;i<=reply_distance2;i++) {
    			  read(new_socket, &reply_path_x, sizeof(int));
    			  read(new_socket, &reply_path_y, sizeof(int));
    			  reply_path2.push_back({reply_path_x, reply_path_y});
    		  }

//    		  int reply_distance = reply_distance1 + reply_distance2;
    		  reply_distance = reply_distance1 + reply_distance2;


    		  cout << "existing route distance " << routes[reply_order_id].distance << "\n";
    		  cout << "checking replied route " << reply_distance << "\n";
    		  cout << "existing robot ID " << myDetails.robotId << "\n";
    		  cout << "replied robot ID " << reply_robot_id << "\n";


    		  if (isRouteExist == ROUTE &&
    				  (order_reply_count[reply_order_id].size() == 1  || routes[reply_order_id].distance > reply_distance ||
    				  (routes[reply_order_id].distance == reply_distance && routes[reply_order_id].robotId >= reply_robot_id))) {
    			  routes[reply_order_id].distance = reply_distance;
    			  routes[reply_order_id].robotId = reply_robot_id;
    			  routes[reply_order_id].path1 = reply_path1;
    			  routes[reply_order_id].path2 = reply_path2;
    			  cout << "Found better robot " << reply_robot_id << " - distance - " << reply_distance << "\n";
    		  }

    	  }

    	  if (order_reply_count[reply_order_id].size() == robot_count) {

    		  // check if route exist from any of the robot
    		  int i;

    		  for(i=0;i<order_reply_count[reply_order_id].size();i++) {
    			  if (order_reply_count[reply_order_id][i] == ROUTE)
    				  break;	// atleast one route exists
    		  }
    		  if (i == order_reply_count[reply_order_id].size()) {

    			  // no route exist
    			  // add route to the noroute_order list
    			  mtx1.lock();
    			  {
    				  cout << "no route exist for order " << reply_order_id << "\n";
    				  noroute_order.push_back(reply_order_id);
    				  cout << "added order to noroute_order list with size " << noroute_order.size() << "\n";
    			  }
    			  mtx1.unlock();

    			 // remove from pending order to process next order
    			  mtx.lock();
    			  {
    				  pop_heap(pending_order.begin(), pending_order.end(), greater<int>());
    				  pending_order.pop_back();
    			  }
    			  mtx.unlock();

    			  // allow consensus to select next available order
    			  nextOrder = true;
    			  cout << "next order put to true\n";
    			  order_reply_count[reply_order_id].clear();

    		  }
    		  else {


    			  // freeze the grid cell
    			  cout << "before updateGrid, reply_order_id " << reply_order_id << "\n";
    			  updateGrid(BLOCK_CELL, routes[reply_order_id], routes[reply_order_id].robotId == myDetails.robotId);
    			  cout << "grid is freezed ... can proceed to work for next order\n";

    			  cout << "Leader Elected " << routes[reply_order_id].robotId << "\n";
				  cout << "Leader Distance Found " << routes[reply_order_id].distance << "\n";
				  cout << "Path from Robot to Item " << routes[reply_order_id].path1.size() - 1<< "\n";
				  cout << "Path from Item to Station " << routes[reply_order_id].path2.size() - 1<< "\n";



				  if (routes[reply_order_id].robotId == myDetails.robotId) {
	    			  cout << "I am the Leader ... \n";
	    			  mtx2.lock();
	    			  {
	    				  myDetails.state = ACTIVE;
	    				  cout << "status changed to active\n";
	    			  }
	    			  mtx2.unlock();
//	    			  myDetails.state = ACTIVE;

	    			  mtx.lock();
					  {

	    				  cout << "popping consensus value from leader \n";

	    				  pop_heap(pending_order.begin(), pending_order.end(), greater<int>());
	    				  pending_order.pop_back();

	    				  cout << "total pending order size " << pending_order.size() << "\n";

					  }
					  mtx.unlock();

					  nextOrder = true;	// set nextOrder for proposing next consensus value
					  cout << "next order put to true\n";
					  cout << "set nextOrder to true to propose new consensus value\n";

					  thread th1(moveRobot, reply_order_id);
	    			  th1.detach();
				  } else {

					  mtx.lock();
					  {
						  cout << "popping consensus value from non-leaders \n";
						  pop_heap(pending_order.begin(), pending_order.end(), greater<int>());
						  pending_order.pop_back();
						  cout << "total pending order size " << pending_order.size() << "\n";
					  }
					  mtx.unlock();

					  nextOrder = true;
					  cout << "next order put to true\n";
				  }
    		  }
    	  }
      } else if (msg == ORDER_RELEASE) {
    	  // by this time, leader robot will have delivered item, changed state = passive, moved to one of exit stand

    	  int orderId;
    	  read(new_socket, &orderId, sizeof(int));

//    	  cout << "Order Delivered\n";

    	  updateGrid(FREE_CELL, routes[orderId], routes[orderId].robotId == myDetails.robotId);

    	  mtx1.lock();
    	  {
//    		  if (noroute_order.size() > 0) {
    			  mtx.lock();
    			  {
    				  while (noroute_order.size() > 0) {

						  pending_order.push_back(noroute_order.front());
						  push_heap(pending_order.begin(), pending_order.end(), greater<int>());

						  noroute_order.pop_front();
    				  }

    				  cout << "pending order size " << pending_order.size() << "\n";
    			  }
    			  mtx.unlock();
//    			  noroute_order.pop_front();

    	  }
    	  mtx1.unlock();

//    	  if (routes[orderId].robotId == myDetails.robotId) {
//
//    		  vector<pair<int, int>> path = routes[orderId].path2;
//    		  pair<int, int> cell = path[path.size() - 1];
//
//    		  moveRobotToExitStand(cell.first, cell.second);
//
////    		  myDetails.currentCoords.x = cell.first;
////    		  myDetails.currentCoords.y = cell.second;
//
//    		  myDetails.state = PASSIVE;
//
//    	  }

    	  printRobotInfo(&myDetails, 1);

//    	  bool is_order_pending = false;
//    	  mtx.lock();
//    	  {
//    		  if (pending_order.size() > 0) {
//    			  is_order_pending = true;
//    		  }
//    	  }
//    	  mtx.unlock();
//    	  if (is_order_pending)
//    		  proposeConsensusValue();

      } else if (msg == CONSENSUS_VALUE_PROPOSE) {
    	  int tmp;
    	  read(new_socket,&tmp, sizeof(int));
    	  bool process_consensus_possible = false;
    	  mtx3.lock();
    	  {
        	  consensus_value_received.push_back(tmp);
        	  if (consensus_value_received.size() == robot_count)
        		  process_consensus_possible = true;
    	  }
    	  mtx3.unlock();
//    	  consensus_value_received.push_back(tmp);

//    	  if (consensus_value_received.size() == robot_count) {
//    		  processConsensusValue();
//    	  }

    	  if (process_consensus_possible)
    		  processConsensusValue();

      } else if (msg == CONSENSUS_VALUE_DISAGREED) {
    	  cout << "Consensus Value Disagreed, proposing new value \n";
    	  proposeConsensusValue();
      }
    }

    cout << "closing socket\n";
    close(new_socket);
	cout << "-------------------------------end createServer()----------------------------------------\n";

}

/*
 * Function to process order
 * nextOrder variable - boolean variable to go for consensus
 * isOrderExist - boolean variable to check whether any order is pending
 */
void processOrder() {

//	cout << "in processOrder()\n";
	cout << "-------------------------------begin processOrder()----------------------------------------\n";

	bool isOrderExist;

	while(1) {



		while(1) {
			if (nextOrder) {	// boolean variable to look into next order i.e. go for next min. consensus value
				cout << "ready to propose next consensus value\n";
				break;
			}
			else {
				cout << "sleep for 2 seconds before to knock\n";
				sleep(2);

			}
		}

		isOrderExist = false;		// check whether pending_order has any orders

		mtx.lock();
		{
			if (pending_order.size() > 0) {
				isOrderExist = true;
				std::cout << "order exists in pending order queue\n";
			}
		}
		mtx.unlock();

		if (isOrderExist) {
			nextOrder = false;
			cout << "next order put to false\n";
			proposeConsensusValue();	// propose consensus value to agree on the which order to process
		} else {
			sleep(2);
		}
	}

	cout << "-------------------------------end processOrder()---------------------------------------\n";


}



void moveRobotToExitStand(int station_neighbor_x, int station_neighbor_y) {

//	cout << "in moveRobotToExitStand()\n";
	cout << "-------------------------------begin moveRobotToExitStand()----------------------------------------\n";

	vector<pair<int, int>> directions = { {-1, 0}, {1,0}, {0,-1}, {0,1} }; // up, down. left, right
	Coords exit_coords;

	for(auto d: directions) {
		exit_coords.x = d.first + station_neighbor_x;
		exit_coords.y = d.second + station_neighbor_y;
		if (isValidGridPosition(exit_coords.x, exit_coords.y) && grid[exit_coords.x][exit_coords.y] == 6) {
			myDetails.currentCoords.x = exit_coords.x;
			myDetails.currentCoords.y = exit_coords.y;
			cout << "Robot New Position " << myDetails.currentCoords.x << ", " << myDetails.currentCoords.y << "\n";
			break;
		}
	}

	cout << "-------------------------------end moveRobotToExitStand()----------------------------------------\n";


//	myDetails.state = PASSIVE;
}



void moveRobot(int orderId) {

//	cout << "in moveRobot()\n";
	cout << "-------------------------------begin moveRobot()----------------------------------------\n";


	int i;
	RouteInfo route = routes[orderId];

	vector<pair<int, int>> path1;
	vector<pair<int, int>> path2;

	path1 = route.path1;
	path2 = route.path2;

	cout << "Begin to move " << "\n";
	for(i=0;i<path1.size() - 1;i++) {
		sleep(1);
		cout << "I am at location: " << path1[i].first << ", " << path1[i].second << "\n";
	}

	cout << "I am picking item " << path1[i].first << ", " << path1[i].second << "\n";
	sleep(1);

	cout << "Picked Item " << path2[0].first << ", " << path2[0].second << "\n";
	sleep(1);

	cout << "Moving towards Station " << "\n";
	for(i=1;i<path2.size();i++) {
		sleep(1);
		cout << "I am at location: " << path2[i].first << ", " << path2[i].second << "\n";
	}

	cout << "Order Delivered\n";


	pair<int, int> cell = path2[path2.size() - 1];
	moveRobotToExitStand(cell.first, cell.second);
	mtx2.lock();
	{
		myDetails.state = PASSIVE;
		cout << "status changed to passive\n";
	}
	mtx2.unlock();

	int msg;
	msg = ORDER_RELEASE;

	vector<int> msgs;
	msgs.push_back(ORDER_RELEASE);	 // msg-1
	msgs.push_back(orderId); // msg-2
	broadcastMsgToRobot(msgs);

	// inform station about the order delivery
	vector<int> msg1;
	msg1.push_back(ORDER_DELIVERY);	// msg-1
	msg1.push_back(orderId);	// msg-2
	msg1.push_back(myDetails.robotId);	// msg-3
	sendMsgToStation(stations[orders[orderId].stationId].networkInfo.portNo, msg1);

//	broadcastMsgToRobot(msgs);
	cout << "-------------------------------end moveRobot()----------------------------------------\n";

}



void computeRoute(Order order) {

//	cout << "in computeRoute()\n";
	cout << "-------------------------------begin computeRoute()----------------------------------------\n";


	int state_status;
	cout << "step 1\n";
	mtx2.lock();
	{
		if (myDetails.state == ACTIVE) {
			state_status = ACTIVE;
			cout << "status is active\n";
		}
		else {
			state_status = PASSIVE;
			cout << "status is passive\n";
		}
	}
	mtx2.unlock();
	cout << "strp 2\n";

	if (state_status == ACTIVE) {

		int msg;
		msg = ACTIVE; // this robot is currently active, can't participate

//		routes[order.orderId].distance = INT_MAX; // implies active state and has not participated in this item delivery

		vector<int> msgs;

		msgs.push_back(ORDER_REPLY); // msg-1
		msgs.push_back(order.orderId); // msg-2
		msgs.push_back(ACTIVE); // msg-3
		msgs.push_back(NO_ROUTE); // msg-3.1

		cout << "step active-3\n";
		broadcastMsgToRobot(msgs);
		cout << "step active-4\n";

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

//		routes[order.orderId].distance = distance;
//		routes[order.orderId].robotId = myDetails.robotId;
//		routes[order.orderId].path1 = path1;
//		routes[order.orderId].path2 = path2;

		cout << "in compute route " << path1.size() << ", " << path2.size() << "\n";

		if (path1.size() == 0 || path1.size() == 0) {
			msgs.push_back(NO_ROUTE);	// msg-3.1
//			broadcastMsgToRobot(msgs);
			cout << "mentioning no route as path is zero\n";
		}

		else {

			msgs.push_back(ROUTE);	// msg-3.1

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
		}
		cout << "step 3\n";
		broadcastMsgToRobot(msgs);
		cout << "step 4\n";
	}

	cout << "-------------------------------end computeRoute()----------------------------------------\n";

}

//bool isvalid(pair<int, int> next_point) {
//	if (next_point.first >= 0 && next_point.first < grid_size && next_point.second >= 0 && next_point.second < grid_size
//	        && grid[next_point.first][next_point.second] == 0) {
//		return true;
//	}
//	return false;
//}

void getPath(Order order, vector<pair<int, int>>& path1, vector<pair<int, int>>& path2) {

//	cout << "in getPath()\n";
	cout << "-------------------------------begin getPath()----------------------------------------\n";


	// from robot to item
	pair<int, int> src1 = {myDetails.currentCoords.x, myDetails.currentCoords.y};
	pair<int, int> dst1 = {items[order.itemId].coords.x, items[order.itemId].coords.y};

	cout << "Robot Location: " << src1.first << ", " << src1.second << "\n";
	cout << "Item Location: " << dst1.first << ", " << dst1.second << "\n";

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


	pair<int, int> src2 = { items[order.itemId].coords.x, items[order.itemId].coords.y };
	pair<int, int> dst2 = { stations[order.stationId].coords.x, stations[order.stationId].coords.y };

	cout << "Item Location: " << src2.first << ", " << src2.second << "\n";
	cout << "Station Location: " << dst2.first << ", " << dst2.second << "\n";

	vector<pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, { -1, 0}};

	for (auto d : directions) {
		pair<int, int> new_dest = {dst2.first + d.first, dst2.second + d.second};
		if (isvalid(new_dest)) {
			cout << "Station Available Location: " << new_dest.first << ", " << new_dest.second << "\n";
			vector<pair<int, int>> tmp = shortest(src2, new_dest);
			// cout << "Distance between src and dest is " << path.size() << endl;
			if (tmp.size() > 0 && tmp.size() < min_length) {
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

	// set grid position as it is
	//	cout << "Path Shown\n";

	cout << "-------------------------------end getPath()----------------------------------------\n";

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

//	cout << "in connectToInitiator()\n";
	cout << "-------------------------------begin connectToInitiator()----------------------------------------\n";


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

//  if (myDetails.robotId == 0) {
//  		cout << "=============DKJASFLAFEIBLFIBEILBFLE===========================================\n";
//  		mtx.lock();
//  		{
//  			pending_order.push_back(-1);
//  			push_heap(pending_order.begin(), pending_order.end(), greater<int>());
//  		}
//  		mtx.unlock();
//  	}
//
//  if (myDetails.robotId == 1) {
//	  mtx.lock();
//	  {
//		  pending_order.push_back(4);
//		  push_heap(pending_order.begin(), pending_order.end(), greater<int>());
//
//		  pending_order.push_back(2);
//		  push_heap(pending_order.begin(), pending_order.end(), greater<int>());
//	  }
//	  mtx.unlock();
//  }


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

	cout << "-------------------------------end connectToInitiator()----------------------------------------\n";

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




void proposeConsensusValue() {


//	cout << "in proposeConsensusValue()\n";
	cout << "-------------------------------begin proposeConsensusValue()----------------------------------------\n";

	int min_value;

	vector<int> msgs;
//	bool isSomeOrderExist = false;

//	mtx.lock();
//	{
//		if (pending_order.size() > 0) {
//			min_value = pending_order.front();
//
////			pop_heap(pending_order.begin(), pending_order.end(), greater<int>());
////			pending_order.pop_back();
//
//			isSomeOrderExist = true;
//			cout << "Consensus Value Found " << min_value << "\n";
//		}
//	}
//	mtx.unlock();

//	if (isSomeOrderExist) {
//		consensus_value_propose = min_value;
//
//		msgs.push_back(CONSENSUS_VALUE_PROPOSE);
//		msgs.push_back(consensus_value_propose);
//
//		broadcastMsgToRobot(msgs);
//
//	} else {
//
//		sleep(2);
//		proposeConsensusValue();
//
//	}

	mtx.lock();
	{
//		min_value = pending_order.front();
		consensus_value_propose = pending_order.front();
	}
	mtx.unlock();

//	consensus_value_propose = min_value;
	cout << "consensus value proposed " << consensus_value_propose << "\n";

	msgs.push_back(CONSENSUS_VALUE_PROPOSE);
	msgs.push_back(consensus_value_propose);

	broadcastMsgToRobot(msgs);
	cout << "-------------------------------end proposeConsensusValue()----------------------------------------\n";

}


void processConsensusValue() {

//	cout << "in processConsensusValue()\n";

	cout << "-------------------------------begin processConsensusValue()----------------------------------------\n";

	int i;

	int n;
	mtx3.lock();
	{
		n = consensus_value_received.size();
	}
	mtx3.unlock();

	vector<int> tmp_values;
	mtx3.lock();
	{
		for(i=0;i<n;i++)
			tmp_values.push_back(consensus_value_received[i]);
	}
	mtx3.unlock();

	for(i=0;i<n;i++) {
		if (tmp_values[i] != consensus_value_propose) {
//			consensus_value_received.clear();
//			mtx.lock();
//			{
//				pending_order.push_back(consensus_value_propose);
//				push_heap(pending_order.begin(), pending_order.end(), greater<int>());
//			}
//			mtx.unlock();
//			sleep(2);
			break;
		}
	}

	mtx3.lock();
	{
		consensus_value_received.clear();
	}
	mtx3.unlock();
//	consensus_value_received.clear();
	tmp_values.clear();

	if (i == n) {
//		mtx.lock();
//		{
//			cout << "robot id " << myDetails.robotId << "\n";
//			cout << "routes robot id " << routes[consensus_value_propose].robotId << "\n";
//			if (myDetails.robotId != routes[consensus_value_propose].robotId) {
//				cout << "popping order " << consensus_value_propose << " \n";
//				pop_heap(pending_order.begin(), pending_order.end(), greater<int>());
//				pending_order.pop_back();
//			}
//		}
//		mtx.unlock();
		cout << "Robot will work on order Id " << consensus_value_propose << "\n";
		computeRoute(orders[consensus_value_propose]);
	}
	else {
		vector<int> msgs;
		msgs.push_back(CONSENSUS_VALUE_DISAGREED); // all robots have cleared their consensus_value_received vector, now propose again the new value
		broadcastMsgToRobot(msgs);
	}

	cout << "-------------------------------end processConsensusValue()----------------------------------------\n";

}



int main() {

  // get robot id provided from station

  // myDetails.state = PASSIVE;

//	cout << "in main()\n";
	cout << "-------------------------------begin main()----------------------------------------\n";

	order_count = 0;
	make_heap(pending_order.begin(), pending_order.end(), greater<int>());


	thread th2(processOrder);
	th2.detach();

//	sleep(2);

	thread th1(createServer);
	th1.join();

	cout << "-------------------------------end main()----------------------------------------\n";

	return 0;
}

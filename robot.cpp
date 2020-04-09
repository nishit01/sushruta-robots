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

struct Robot robot;



/*
Function to create server thread
*/
void createServer() {
  int
}


/*
Function to get details (pre-defined) about the picking station
*/
struct Station* getStationInfo() {

  int no_of_stations;

  cout << "Enter Number of Stations in the Grid: ";
  cin >> no_of_stations;

  struct Station* stations;
  stations = (struct Station *)malloc(no_of_stations*sizeof(struct Station));

  int i;
  for(i=0;i<no_of_stations;i++) {

    cout << "Enter Station ID: ";
    cin >> stations[i]->stationId;
    cout << "Enter Station PortNo: ";
    cin >> stations[i]->networkInfo.portNo;

  }

  return stations;
}


int main() {

  // get robot id provided from station





  return 0;
}

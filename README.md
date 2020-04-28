# sushruta-robots
DC Project

Steps to run the project
------------------------
1. We have to compile the 3 files: initiator.cpp, robot.cpp and station.cpp using the commands
    ```
    g++ initiator.cpp -lpthread -o init
    g++ robot.cpp -lpthread -o robot
    g++ station.cpp -lpthread -o station
    
    ```
2. We have written a dummy input file that has to be given to the init called **inp2.txt**.
3. We first open 1 terminal for initiator and 1 terminal each for the robot and the station.
4. The code is run by
  ```
  In terminal pertaining to initiator     :    ./init < inp2.txt
  In the terminals pertaining to robots   :    ./robot
  In the terminals pertaining to stations :    ./station
  ```
 5. Once the basic setup is taken care by the initiator, it is done.
 6. Now we have to place the order in the station pertaining to the order number given in the choice
 7. A robot would be chosen and would deliver the item needed to the station and then go to a pre-defined waiting area
 8. For now, we have the case where after we place the orders we need, we have to forcefully quit from the robot and station code as they always keep waiting for the order from station and user respectively.

Details about the inp2.txt
--------------------------
(We assume grid to be 0-indexed)<br/>
First line would contain 3 integers, **#robots, #stations and grid_size** <br/>
Next line we enter the **#items** <br/>
Then, each item has 3 details which are entered , namely **item_name, quantity and grid_location** (which would extend upto **3 \* #items** lines) <br/>
The  next **#robot lines** contain the grid-location of each robot <br/>
The next **#station lines** contain the grid-location of each station <br/>

GridPath.cpp

1. Contains the code for performing distance calculation based on the grid.
2. Assumes that the grid location is free to be considered if it has a value 0, it is 1; then I've assumed it to be occupied
3. Prints the entire path(each step; instead of the indexes where the robot needs to turn)
4. Here, I've considered each grid location as a pair<int,int> instead of creating a structure as those are the only two things needed.
5. Each robot can then calculate the distance by getting the size of the path vector(can subtract 2 for src and dst).
6. Using that distance we can find out which robot is closer to the pickup item(Assuming that the pickup station provides the grid index of the item to be picked up/ we somehow calculate the shortest of the 4 indexes that would be needed to pickup the object)
7. I've written the code considering each point as a pair of x ad y co-ordinates.
8. Need to optimize the code a little bit by using  array in place of the vector. 


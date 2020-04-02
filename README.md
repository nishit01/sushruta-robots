# sushruta-robots
DC Project

GridPath.cpp-- 
1. Contains the code for performing distance calculation based on the grid.
2. Assumes that the grid location is free to be considered if it has a value 0, it is 1; then I've assumed it to be occupied
3. Prints the entire path(each step; instead of the indexes where the robot needs to turn)
4. Here, I've considered each grid location as a pair<int,int> instead of creating a structure as those are the only two things needed.
5. Each robot can then calculate the distance by getting the size of the path vector(can subtract 2 for src and dst).
6. Using that distance we can find out which robot is closer to the pickup item(Assuming that the pickup station provides the grid index of the item to be picked up/ we somehow calculate the shortest of the 4 indexes that would be needed to pickup the object)
7. I've written the code considering each point as a pair of x ad y co-ordinates.
8. Need to optimize the code a little bit by using  array in place of the vector. 


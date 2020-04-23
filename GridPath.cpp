#include<bits/stdc++.h>
using namespace std;
/*
Assuming the grid is filled with 0's meaning, all paths are open
If some path is already taken from a source to destination,
then it is set is 1 which means that particular index is already taken
*/
#define grid_size 10
int grid[grid_size][grid_size];

//checking valid node
bool isvalid(pair<int, int> next_point) {
	if (next_point.first >= 0 && next_point.first < grid_size && next_point.second >= 0 && next_point.second < grid_size
	        && grid[next_point.first][next_point.second] == 0) {
		return true;
	}
	return false;
}

//finding shortest path
vector<pair<int, int>> getPredecessor(pair<int, int> src, pair<int, int> dest,
vector<pair<int, int>> pred) {
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
			if (isvalid(next_point) &&
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



int main() {
	pair<int, int> src = {5, 0}, dest = {6, 7};
	vector<pair<int, int>> path = shortest(src, dest);
	for (auto x : path) {
		cout << x.first << "," << x.second << endl;
	}
	return 0;
}

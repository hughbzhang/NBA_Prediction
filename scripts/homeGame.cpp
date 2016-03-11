#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

int main() {
	string team1, team2, location, newLoc;
	int score1, score2;
	while (cin >> team1){
		if (team1 == "playoffs"){
			 cin >> team1;
		}
		cin >> score1 >> team2 >> score2 >> location;
		if(team1.substr(0, location.size()-2).compare(location.substr(2)) == 0) {
			newLoc = "home";
		} else {
			if (team2.substr(0, location.size()-2).compare(location.substr(2)) != 0) {
				cout << "BAD " << team1 << " " << score1 << " " << team2 << " " << score2 << " " << location << endl;
				throw;
			}
			newLoc = "away";
		}
		cout << team1 << " " << score1 << " " << team2 << " " << score2 << " " << newLoc << endl; 
	}
}
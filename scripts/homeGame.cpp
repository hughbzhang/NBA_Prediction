/**
This script is responsible for telling whether each game is a home game or an away game.
Note, you need to run this before you replace the names, since location has to change too
*/

#include <cstdio>
#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

int main() {
	string team1, team2, location, newLoc;
	int score1, score2;
	while (cin >> team1){
		if (team1 == "playoffs"){
			 cout << "playoffs" << endl;
			 cin >> team1;
		}
		cin >> score1 >> team2 >> score2 >> location;
		if(team1.find(location.substr(2)) != -1) {
			newLoc = "home";
		} else {
			if (team2.find(location.substr(2)) == -1) {
				cout << "HOMEGAME ERROR " << team1 << " " << score1 << " " << team2 << " " << score2 << " " << location << endl;
				assert(false);
			}
			newLoc = "away";
		}
		cout << team1 << " " << score1 << " " << team2 << " " << score2 << " " << newLoc << endl; 
	}
}
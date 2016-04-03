/**

This file validates the data I have. It ensures that all teams are:

1) One of the thirty alive today
2) Winners first
3) In the format WINNER WSCORE LOSER LSCORE HOME/AWAY

*/

#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>

using namespace std;

const int numTeams = 30;
string teams[numTeams];
string input;
map<string,bool> exists;

int main() {


    int playoffCount = 0;

  	ifstream NBA ("../NBATeams.txt");
  	for (int x = 0; x<numTeams; x++) {
  		NBA >> teams[x];
  		exists[teams[x]] = true;
  	}
  	while(cin >> input) {
  		string team1, team2, location;
  		int score1, score2, date;
  		team1 = input;
  		if (input == "playoffs") {
        playoffCount++;
  			cin >> team1;
  		}
  		cin >> score1 >> team2 >> input >> location >> date;
  		if (score1 <= score2 || !exists[team1] || !exists[team2] || date < 0 || date > 731) { // Two years
  			cout << "VALIDATE ERROR " << team1 << " " << score1 << " " << team2 << " " << score2 << " " << location << " " << date << endl;
  			assert(false);
  		}
  	}
    assert(playoffCount == 1);
}
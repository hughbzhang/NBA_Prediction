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

string team = "miamiheat";

int main() {
  	while(cin >> input) {
  		string team1, team2, location;
  		int score1, score2, date;
  		team1 = input;
  		if (input == "playoffs") {
  			cin >> team1;
  		}
  		cin >> score1 >> team2 >> input >> location >> date;
  		if (team1==team) cout << 1 << endl;
  		else if (team2==team) cout << 0 << endl;
  	}
}
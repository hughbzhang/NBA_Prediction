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
  	ifstream NBA ("NBATeams.txt");
  	for (int x = 0; x<numTeams; x++) {
  		NBA >> teams[x];
  		exists[teams[x]] = true;
  	}
  	while(cin >> input) {
  		string team1, team2, location;
  		int score1, score2;
  		team1 = input;
  		if (input == "playoffs") {
  			cin >> team1;
  		}
  		cin >> score1 >> team2 >> input >> location;
  		if (score1 <= score2 || !exists[team1] || !exists[team2]) {
  			cout << team1 << " " << score1 << " " << team2 << " " << score2 << " " << location << endl;
  			// for (int x = 0;x<30;x++) cout << teams[x] << endl;
  			
  			assert(false);
  		}
  	}
}
// This file prints out the win history of any given team in any given year.
// Input the parsed data for that year (these files are stored in PARSED_DATA_WITH_DATE_AND_OT)

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
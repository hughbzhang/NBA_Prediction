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

string T1, T2;

int main() {
    ifstream gameData ("oneYear.txt");

    cin >> T1 >> T2;

    int W1 = 0, L1 = 0;


  	while(gameData >> input) {
  		string team1, team2, location;
  		int score1, score2, date;
  		team1 = input;
  		if (input == "playoffs") {
  			gameData >> team1;
        break;
  		}
  		gameData >> score1 >> team2 >> score2 >> location >> date;
      if (T1==team1&&T2==team2) {
        W1++;
      }
      if (T2==team1&&T1==team2) {
        L1++;
      }

  	}
    cout << W1 << " " << L1 << endl;
}
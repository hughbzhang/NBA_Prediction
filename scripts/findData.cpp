#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>

using namespace std;

map<string, int> lastPlayed;

int win[2][2]; // win with X days of separation // 1 is home 0 is away
int total[2][2]; // total games with X days of separation

int main() {
    string input;
  	while(cin >> input) {

  		string team1, team2, location;
  		int score1, score2, date;
  		team1 = input;
  		if (input.find("playoffs")!=-1) {
  			cin >> team1;
  		}
  		cin >> score1 >> team2 >> score2 >> location >> date;

      // cout << team1 << score1 << team2 << score2 << location << date << endl;



      int diff1 = date - lastPlayed[team1];
      int diff2 = date - lastPlayed[team2];

      // cout << diff1 << " " << diff2 << endl;

      lastPlayed[team1] = date;
      lastPlayed[team2] = date;

      if (diff1 < 50 && diff1 > 0) { // new year
          win[diff1!=1][location=="home"]++;
          total[diff1!=1][location=="home"]++;
      } 


      if (diff2 < 50 && diff2 > 0) { // new year for team 2 even if it is not for team 1
          total[diff2!=1][location!="home"]++;
      } 
      
  	}

    for (int x = 0;x<2;x++) cout << win[x][0] << " ";
    cout << endl;
    for (int x = 0;x<2;x++) cout << total[x][0] << " ";
    cout << endl;

    for (int x = 0;x<2;x++) cout << win[x][1] << " ";
    cout << endl;
    for (int x = 0;x<2;x++) cout << total[x][1] << " ";
    cout << endl;
}
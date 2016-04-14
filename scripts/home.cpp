#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>

using namespace std;

int main() {
    int home = 0, away = 0, homeP = 0, awayP = 0;
    bool playoffs = false;
    string input;
  	while(cin >> input) {
  		string team1, team2, location;
  		int score1, score2, date;
  		team1 = input;
  		if (input.find("playoffs")!=-1) {
        playoffs = true;
  			cin >> team1;
  		}
  		cin >> score1 >> team2 >> score2 >> location >> date;


      if (!playoffs) {
        if(location=="home") home++;
        else away++;
      } else {
        if(location=="home") homeP++;
        else awayP++;
      }
  	}
    cout << home << " " << home+away << endl;
    cout << homeP << " " << homeP+awayP << endl;
}
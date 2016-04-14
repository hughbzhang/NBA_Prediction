/**

This is my rating system. It needs a lot of cleaning up. Tweak the rank constants and see what magic happens

*/

#include <cstdio>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <iostream>
#include <assert.h>
#include <fstream>

using namespace std;
typedef long double ld;

// Global data
map<string,ld> rating; // Map from name of team to its rating
map<string, vector<ld> > ratingHistory; // list of previous ratings
vector<string> names; // List of team names

// Rank constants
const int K = 16;
const int startRank = 1500;
const ld carryOverRatio = 1;

int compareTwoTeams(string one, string two){
    return rating[one] > rating[two]; // Sort so the best team is on top
}

ld regress (ld rating) {
    return carryOverRatio*rating + (1-carryOverRatio)*startRank;
}

void printTeamRanking(int numToPrint = names.size()) {
    ofstream output ("ranks.txt");
    sort(names.begin(),names.end(), compareTwoTeams);
    for(int x = 0;x<numToPrint;x++) output << names[x] << " " << regress(rating[names[x]]) << endl; 
}

bool update(){

    string firstTeam, secondTeam, tmp, location;
    int score1, score2;

    if (!(cin >> firstTeam)) {
        return false;
    }

    if (firstTeam == "playoffs") {
        cin >> firstTeam;
    }

    cin >> score1 >> secondTeam >> score2 >> location;

    ld QA = pow(10,rating[firstTeam]/400); 
    ld QB = pow(10,rating[secondTeam]/400); 
    ld EA = QA/(QA+QB);
    rating[firstTeam] += K*(1-EA);
    rating[secondTeam] += K*(-EA);
    ratingHistory[firstTeam].push_back(rating[firstTeam]);
    ratingHistory[secondTeam].push_back(rating[secondTeam]);
    return true;
}

void getInput() {
    ifstream newfile ("ranks.txt");
    string tmp;
    for (int x = 0; x < 30; x++) {  
        newfile >> tmp;
        names.push_back(tmp);
        newfile >> rating[tmp];
    }
}

int main(){
    getInput();
    while(update());
    printTeamRanking();
    cout << "YEAR" << endl;

    // save elo scores
}

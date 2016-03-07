/* This is my elo system for predicting basketball. Essentially I use Naive Bayesian updating to assign a rating for each candidate.
 * I then use this data to predict who is going to win.
 * Parameters to be adjusted include the K value (how much the ranks change) as well as the logistic regression model for win % based on different ratings
 * Initial tests show around 65% accuracy
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
map<string,bool> vis; // Whether a team is in the list of names
int playoffCount = 0;

// Elo constants
const int K = 16;
const int startElo = 3000;

// Constants for each season
const int finals = 1307;
const int totalGames = 1314;
const int regularSeason = 1229;
const int firstRound = 1274;
const int confFinals = 1296;

ofstream newfile ("scorefile.txt", ios_base::app);

int compareTwoTeams(string one, string two){
    return rating[one] > rating[two]; // Sort so the best team is on top
}

void printTeamRanking(int numToPrint = names.size()) {
    sort(names.begin(),names.end(), compareTwoTeams);
    for(int x = 0;x<numToPrint;x++) cout << names[x] << " " << rating[names[x]] << endl; 
}

void ensureTeamExists(string team) {
    if(!vis[team]){
        vis[team] = true;
        rating[team] = startElo;
        names.push_back(team);
    }    
}

void update(){

    string firstTeam, secondTeam, tmp, location;
    int score1, score2;

    cin >> tmp;
    if (tmp == "playoffs") {
        cout << "THIS IS PLAYOFF ROUND " << playoffCount << endl;
        playoffCount++;
        // printTeamRanking(20);
        update();
        return;
    } else {
        firstTeam = tmp;
    }

    cin >> score1 >> secondTeam >> score2 >> location;
    newfile << score1-score2 << endl;

    // ensureTeamExists(firstTeam);
    // ensureTeamExists(secondTeam);

    // assert(score1 > score2);

    ld QA = pow(10,rating[firstTeam]/400); 
    ld QB = pow(10,rating[secondTeam]/400); 
    ld EA = QA/(QA+QB);
    rating[firstTeam] += K*(1-EA);
    rating[secondTeam] += K*(-EA);
    ratingHistory[firstTeam].push_back(rating[firstTeam]);
    ratingHistory[secondTeam].push_back(rating[secondTeam]);
}

void printTeamHistory(string team) {
    for (int x = 0;x<ratingHistory[team].size();x++) {
        cout << ratingHistory[team][x] << endl;
    }
}

void printAllTeamsHistory() {
    sort(names.begin(),names.end(), compareTwoTeams);
    for (int a = 0; a<ratingHistory.size(); a++) {
        string teamName = names[a];
        cout << teamName << ",";
        for (int x = 0;x<ratingHistory[teamName].size();x++) {
            cout << ratingHistory[teamName][x] << ",";
        } 
        cout << endl;
    }
}

int main(){
    //freopen("DATA.txt","r",stdin);
//     ofstream newfile ("scorefile.txt", ios_base::app);
    for(int x = 0;x<1311;x++){
        update();
    }
    for(int x = 0;x<names.size();x++) cout << names[x] << endl; 
    newfile << "GAME" << endl;
    // printTeamRanking();
}

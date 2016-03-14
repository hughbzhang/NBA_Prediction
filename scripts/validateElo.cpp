/**

This is my rating system. It needs a lot of cleaning up. Tweak the rank constants and see what magic happens
Problem. The model sucks. The tweaks don't help. WTF is going on.
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

struct Game {
    string winTeam;
    string loseTeam;
    int winScore;
    int loseScore;
    bool homeGame;
};

Game* allGames;
int counter = 0;
int trainGames = 0;

const ld INITIAL_ELO = 1500;
const ld DENOM = 400;
const ld epsilon = 1e-6;

void resetRanks() { //xcxc rewrite
    for(int x = 0;x<names.size();x++) {
        rating[names[x]] = INITIAL_ELO;
    }
}

int compareTwoTeams(string one, string two){
    return rating[one] > rating[two]; // Sort so the best team is on top
}

void printTeamRanking() {
    sort(names.begin(),names.end(), compareTwoTeams);
    for(int x = 0;x<names.size();x++) {
        cout << names[x] << " " << rating[names[x]] << endl; 
    }
}

void regressRanks(ld carryOver) { //xcxc rewrite
    for(int x = 0;x<names.size();x++) {
        rating[names[x]]*=carryOver;
        rating[names[x]]+=(INITIAL_ELO*(1-carryOver));
    }
}

ld winPercentage (ld eloDiff) {
    return 1/(ld)(1+pow(10, -eloDiff/DENOM));
}

ld costFunction(ld probability) {
    return -log(probability); // logistic cost function given you win
}

bool zero(ld in) {
    if(abs(in)<epsilon) {
        return true;
    } else {
        return false;
    }
}

pair<ld,int> elo(ld K, ld carryOver) {
    resetRanks();
    ld error = 0;
    int accuracy = 0;
    for (int x = 0; x<counter; x++) {

        string winTeam = allGames[x].winTeam;
        string loseTeam = allGames[x].loseTeam;

        int eloDiff = allGames[x].winScore - allGames[x].loseScore;

        if (winTeam == "playoffs"){

            // Nothing special for now
        } else if (winTeam == "yearEnd") {
            regressRanks(carryOver);
        } else {

            //cout << rating[winTeam] - rating[loseTeam] << "," << eloDiff << endl;
            ld ratingDiff = rating[winTeam]-rating[loseTeam];

            ld QA = pow(10,rating[winTeam]/DENOM); 
            ld QB = pow(10,rating[loseTeam]/DENOM); 
            ld EA = QA/(QA+QB); // winner's probability of winning

            assert(zero(winPercentage(ratingDiff)-EA));

            if (x > trainGames) {
                error += costFunction(EA);
                accuracy += (rating[winTeam] > rating[loseTeam]);
            }

            rating[winTeam] += K*(1-EA); //1 for the actual win minus what he was expected is his bonus
            rating[loseTeam] += K*(0-(1-EA)); // EB = 1-EA and zero for his loss


            //ratingHistory[firstTeam].push_back(rating[firstTeam]);
            //ratingHistory[secondTeam].push_back(rating[secondTeam]);
        }
    }
    return make_pair(error, accuracy);
}

void initialize() {
    allGames = new Game[100000];

    string tmp;

    freopen("../NBATeams.txt", "r", stdin);
    while (cin >> tmp) {
        names.push_back(tmp);
        rating[tmp] = INITIAL_ELO;
    }
    ifstream gameData ("../gameData.txt");

    string winTeam, loseTeam, location;
    int winScore, loseScore;

    while (gameData >> winTeam) {
        if (winTeam == "playoffs") { 
            allGames[counter].winTeam = winTeam;
        } else if (winTeam == "yearEnd") { // separation of years
            allGames[counter].winTeam = winTeam;
        } else {
            gameData >> winScore >> loseTeam >> loseScore >> location;

            allGames[counter].winTeam   = winTeam;
            allGames[counter].loseTeam  = loseTeam;
            allGames[counter].loseScore = loseScore;
            allGames[counter].winScore  = winScore;
            allGames[counter].homeGame  = location == "home" ? true : false;
        }
        counter++;
    }

    trainGames = floor(counter*.9);

    cout << trainGames << " " << counter << " " << counter-trainGames << endl;

}

void print() {
    for (int x = 0; x < counter; x++ ) {
        if (allGames[x].winTeam == "playoffs" || allGames[x].winTeam == "yearEnd") {
            cout << allGames[x].winTeam << endl;
        } else {
            cout << allGames[x].winTeam << " " << allGames[x].winScore; 
            cout << " " << allGames[x].loseTeam << " " << allGames[x].loseScore << endl;
        }
    }
}

int main(){

    initialize();

    for (ld K = 2; K < 100; K*=2) {
        for (ld carry = 0.1; carry < 1; carry+=.1){
            pair<ld,int> ans = elo(K, carry);
            cout << K << " " << carry << " ";
            cout << ans.first << " " << ans.second << endl;
        }
    }






    // save elo scores
}

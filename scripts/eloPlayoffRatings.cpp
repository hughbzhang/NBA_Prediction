// My genetic algorithm is too slow for playoff prediction (probably because I suck)

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

// Elo constants
int K = 16;
int HOME_ADVANTAGE_CONSTANT = 200;
ld SCORE_WEIGHT_CONSTANT = 1;

// Global data
map<string,ld> rating; // Map from name of team to its rating
map<string,ld> homeAdvantage;
const ld averagePointsPerGame = 195.4; //205.34  2015-2016 NBA Season 195.4 is total average though. IDK.

struct Game {
    string winTeam;
    string loseTeam;
    int winScore;
    int loseScore;
    bool homeGame;
    int date;
};

Game* allGames;
int playoffGames = 0;


map<string,string> opponent;
map<string,int> winCounter;
map<string, bool> predictWin;

map<string, vector<ld> > ratingHistory; // list of previous ratings
vector<string> names; // List of team names
map<string,bool> vis; // Whether a team is in the list of names
int playoffCount = 0;

// Elo constants
const int TOP_N_TEAMS = 20;
const int numTeams = 30;
const int maxSimulations = 1000; // maybe vary based on confidence interval
const ld DENOM = 400;

ifstream ratingData ("ratings.txt");
ifstream gameData ("oneYear.txt");
ifstream NBA ("../NBATeams.txt");




int compareTwoTeams(string one, string two){
    return rating[one] > rating[two]; // Sort so the best team is on top
}
ld randomDouble() { return (ld)rand() / RAND_MAX; }
ld winPercentage (ld eloDiff) { return 1/(ld)(1+pow(10, -eloDiff/DENOM)); }

void printTeamRanking(int numToPrint = names.size()) {
    sort(names.begin(),names.end(), compareTwoTeams);
    for(int x = 0;x<numToPrint;x++) cout << names[x] << " " << rating[names[x]] << endl; 
}

ld simulateTwoPlayers(ld homeWinRate, ld awayWinRate) {

    // NBA format is 2-2-1-1-1
    int seriesWinner = 0;

    for (int x = 0; x < maxSimulations; x++) {
        int win = 0;
        for (int a = 0; a < 4; a++) if (randomDouble() < homeWinRate) win++;
        for (int a = 0; a < 3; a++) if (randomDouble() < awayWinRate) win++;

        if (win >= 4) seriesWinner++;
    }

    return seriesWinner/(ld)maxSimulations;
}

int executeCycle(){

    int correctPredictions = 0;

    for (int x = 0;x<playoffGames;x++) {

        string winTeam = allGames[x].winTeam;
        string loseTeam = allGames[x].loseTeam;

        if (opponent[winTeam]==loseTeam) {
            winCounter[winTeam]++;
            winCounter[loseTeam]--;
        } else {

            // Get data from the old round
            if (opponent[winTeam]=="") assert(opponent[loseTeam]=="");
            if (predictWin[winTeam]) correctPredictions++;
            if (predictWin[loseTeam]) correctPredictions++;

            // Reset for the next round
            opponent[winTeam] = loseTeam;
            opponent[loseTeam] = winTeam;
            winCounter[winTeam] = 1;
            winCounter[loseTeam] = -1;
            predictWin[winTeam] = false;
            predictWin[loseTeam] = false;

            // Predict next round
            ld homeWin = winPercentage(rating[winTeam] - rating[loseTeam] + HOME_ADVANTAGE_CONSTANT);
            ld awayWin = winPercentage(rating[winTeam] - rating[loseTeam] - HOME_ADVANTAGE_CONSTANT);
            ld winRate = simulateTwoPlayers(homeWin, awayWin);
            if (winRate > .5) predictWin[winTeam] = true;
            else predictWin[loseTeam] = true;
        }

        // Update ratings
        int scoreDiff = allGames[x].winScore - allGames[x].loseScore;
        int totalScore = allGames[x].winScore + allGames[x].loseScore;
        int actualScore = SCORE_WEIGHT_CONSTANT*log(scoreDiff/(ld)totalScore*averagePointsPerGame);

        ld QA = pow(10,rating[winTeam]/DENOM); 
        ld QB = pow(10,rating[loseTeam]/DENOM); 
        ld EA = QA/(QA+QB);
        rating[winTeam] += K*(actualScore-EA);
        rating[loseTeam] += K*(EA-actualScore);
    }

    if (predictWin[allGames[playoffGames-1].winTeam]) correctPredictions++; // NBA Champions
    return correctPredictions;
}

void initialize() {
    allGames = new Game[1000];

    string winTeam, loseTeam, location, tmp;
    int winScore, loseScore, date;

    bool start = false;

    while (gameData >> winTeam) {
        
        if (start) {
            gameData >> winScore >> loseTeam >> loseScore >> location >> date;

            allGames[playoffGames].winTeam   = winTeam;
            allGames[playoffGames].loseTeam  = loseTeam;
            allGames[playoffGames].loseScore = loseScore;
            allGames[playoffGames].winScore  = winScore;
            allGames[playoffGames].homeGame  = location == "home" ? true : false;
            allGames[playoffGames].date = date;
            playoffGames++;

        }

        if (winTeam.find("playoffs")!=-1) {
            start = true;
        }
    }

    for (int x = 0; x<numTeams; x++) {
        NBA >> tmp;
        names.push_back(tmp);
    }
}

void readInitialRatings() {

    string heading1, heading2, teamName;
    ld input_rating, homeadv, fitness;
    ratingData >> heading1 >> heading2;

    assert(heading1=="WINNING"); assert(heading2=="CREATURE");
    for (int x = 0;x<TOP_N_TEAMS;x++) {
        ratingData >> teamName >> input_rating >> homeadv;
        rating[teamName] = input_rating;
        homeAdvantage[teamName] = homeadv;
    }

    ratingData >> fitness;

    ld baseRating = input_rating-50;
    for (int x = 0; x<numTeams; x++) {
        if (rating[names[x]]==0) rating[names[x]] = baseRating;
    }

}

void resetAll() {
    rating.clear();
    homeAdvantage.clear();
    opponent.clear();
    winCounter.clear();
    predictWin.clear();
}

ld cycle() {

    ratingData.seekg(0, ios::beg);
    int correct = 0;

    for (int x = 0;x<10;x++) {

        resetAll();
        readInitialRatings();

        correct += executeCycle();
    }
    return correct/(ld)10;
}

int main(){

    srand(7);
    initialize();

    for (K = 8;K<64;K*=2) {
        for (HOME_ADVANTAGE_CONSTANT = 300; HOME_ADVANTAGE_CONSTANT < 600; HOME_ADVANTAGE_CONSTANT+=50) {
            for (SCORE_WEIGHT_CONSTANT = 0.1; SCORE_WEIGHT_CONSTANT <= 3.2; SCORE_WEIGHT_CONSTANT*=2) {
                cout << cycle() << ",";
            }
        }
    }
    cout << endl;
}

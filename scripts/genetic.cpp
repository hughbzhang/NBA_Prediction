/**

Genetic algorithm to minimize a given function


DENOM is 400 so I can line up approximately with 538. Mean is held at 1500 for the same reason

*/

#include <cstdio>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <cstdlib>
#include <math.h>

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

struct Creature {
	ld* ratings;
};

const ld epsilon = 1e-6;
const int numTeams = 32;
const int populationSize = 100;
const ld DENOM = 400;
const int averageRating = 1500;
const ld RANDOM_MAX_SAFE = 1e9;

Game* allGames;
Creature* population;
int counter = 0;
int trainGames = 0;

ld sampleNormal() {
    ld u = ((ld) rand() / (RANDOM_MAX_SAFE)) * 2 - 1;
    ld v = ((ld) rand() / (RANDOM_MAX_SAFE)) * 2 - 1;
    ld r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    ld c = sqrt(-2 * log(r) / r);
    return u * c + 5; // Prevent hitting zero average. It is normalized anyway.
}

ld winPercentage (ld eloDiff) {
    return 1/(ld)(1+pow(10, -eloDiff/DENOM));
}

ld logCostFunction(ld probability) {
    return -log(probability); // logistic cost function given you win
}

bool zero(ld in) {
    if(abs(in)<epsilon) {
        return true;
    } else {
        return false;
    }
}

void normalizeRating(int index) {

	ld sum = 0;

	Creature creature = population[index];

	for (int x = 0; x < numTeams; x++) {
		sum += creature.ratings[x];
	}
	for (int x = 0; x < numTeams; x++) {
		creature.ratings[x] *= (averageRating*32/sum);
	}
}

void generateRandomPopulation() {
	for (int a = 0; a < populationSize; a++) {
		for (int x = 0; x < numTeams; x++) {

			population[a].ratings[x] = sampleNormal();
		}
		normalizeRating(a);
	}
}

void initialize() {
    allGames = new Game[100000];
    population = new Creature[populationSize];
    for (int x = 0;x<populationSize; x++) {
    	population[x].ratings = new ld[numTeams];
    }

    string tmp;

    freopen("../NBATeams.txt", "r", stdin);
    while (cin >> tmp) {
        names.push_back(tmp);
        rating[tmp] = 2;
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
}

void evolve() {
	generateRandomPopulation();
	
	// Kill off a K per round
	// Make sure to keep best B < K
	// Mutation rate M
	// Crossover by averaging ratings

	// Also index teams to ratings


	return;
}

int main(){
	srand(7);
    initialize();
    evolve();
}

/**

Genetic algorithm to minimize a given function

DENOM is 400 so I can line up approximately with 538. Mean is held at 1500 for the same reason

Sanity checked with ELO output from validateElo.cpp to make sure it made sense.
Elo output with 538 constants is a cost of 778.981.
Random ratings seem to have costs double this.

*/

#include <cstdio>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <random>

using namespace std;
typedef long double ld;

// Global data
vector<string> names; // List of team names
set<int> testSet;

struct Game {
    string winTeam;
    string loseTeam;
    int winScore;
    int loseScore;
    bool homeGame;
    int date;
};

struct Score {
    int wrongGuess;
    ld brier;
    ld logLoss;
};

struct Creature {
	ld* ratings;
    ld homeAdvantage; // Make it per team later?
    ld fitness; // fitness is calculated on initialization
    Score score;
} ELO;

// Base constants
const ld epsilon = 1e-6;
const int numTeams = 30;
const ld DENOM = 400;
const int averageRating = 1500;
const ld RANDOM_MAX_SAFE = 1e9;

// Genetic constants
const int populationSize = 10;
const int killSize = 5; // Kill all from 0 to killSize
const int mutationRate = 50; // Percentage is 1/mutationRate. We use this for modding.
const int maxTimeSteps = 4000;
const int convergeTimeSteps = maxTimeSteps/10;

ld ELO_RATINGS[] = {1524.06, 1360.66, 1634.58, 1550.66, 1627.52, 1352.95, 1431.03, 1461.23, 1505.31, 1317.65, 1502.1, 1495.59, 1470.91, 1533.77, 1497.83, 1573.84, 1601.16, 1333.95, 1543.81, 1339.3, 1427.83, 1441.91, 1505.42, 1478.93, 1718.75, 1412.11, 1536.67, 1551.66, 1639.06, 1629.73};

Game* allGames;
Creature* population;
Creature* winners; // For round 2 of the genetic algorithm with the winners of the first round

int numGamesInSeason = 0;
int regularSeason = -1; // Number of games in the regular season (probably data loss)

map<string, int> teamToIndex;
string indexToTeam[numTeams];
map<string, ld> finalRating;
map<string, int> playoffWrong;


default_random_engine generator;
normal_distribution<ld> distribution(averageRating, 500);

// Helper functions
int compareCreatures(Creature one, Creature two) { return one.fitness > two.fitness; } // Sort form high to low 
int compareTwoTeams(string one, string two) { return finalRating[one] > finalRating[two]; } // Sort so the best team is on top
double randomDouble() { return (double)rand() / RANDOM_MAX_SAFE; }
ld winPercentage (ld eloDiff) { return 1/(ld)(1+pow(10, -eloDiff/DENOM)); }
ld logCost(ld probability) { return -log(probability); }
bool zero(ld in) { if(abs(in)<epsilon) { return true; } else { return false; } } 
Score newScore() { Score score; score.logLoss = 0; score.brier = 0; score.wrongGuess = 0; return score; }

// Hacky implementation where person of rank X is multiplicatively more likely to get selected as rank X+1
const double RANDOM_BASE = 1.3;
int magicLogFunction(int total) { return (int)floor(log(rand()%total+1)/log(RANDOM_BASE)); }

pair<int, int> selectTwoRandom(int total) {
    pair<int, int> ans;
    ans.first = rand()%total;

    int next = rand()%total;
    while (next == ans.first) {
        next = rand()%total;
    }

    ans.second = next;
    return ans;
}

ld builtInNormal() {
    return distribution(generator);
}

void initialize() {
    allGames = new Game[100000];
    population = new Creature[populationSize];
    winners = new Creature[populationSize];
    for (int x = 0;x<populationSize; x++) {
        population[x].ratings = new ld[numTeams];
        winners[x].ratings = new ld[numTeams];
    }

    string tmp;
    freopen("../NBATeams.txt", "r", stdin);
    while (cin >> tmp) names.push_back(tmp);

    ifstream gameData ("oneYear.txt");
    string winTeam, loseTeam, location;
    int winScore, loseScore, date;

    while (gameData >> winTeam) {
        if (winTeam == "playoffs") { 
            allGames[numGamesInSeason].winTeam = winTeam;
            regularSeason = numGamesInSeason;
        } else if (winTeam == "yearEnd") { // separation of years
            allGames[numGamesInSeason].winTeam = winTeam;
        } else {
            gameData >> winScore >> loseTeam >> loseScore >> location >> date;

            allGames[numGamesInSeason].winTeam   = winTeam;
            allGames[numGamesInSeason].loseTeam  = loseTeam;
            allGames[numGamesInSeason].loseScore = loseScore;
            allGames[numGamesInSeason].winScore  = winScore;
            allGames[numGamesInSeason].homeGame  = location == "home" ? true : false;
            allGames[numGamesInSeason].date = date;
        }
        numGamesInSeason++;
    }

    ifstream NBA ("../NBATeams.txt");
    for (int x = 0; x<numTeams; x++) {
        NBA >> tmp;
        indexToTeam[x] = tmp;
        teamToIndex[tmp] = x;
    }

    int cnt = 0;
    while(cnt < regularSeason/10) {
        int pos = rand() % regularSeason;
        if (!testSet.count(pos)) {
            cnt++;
            testSet.insert(pos);
        }
    }
}

void copyCreature(int cycle, Creature creature) {
    for (int x = 0; x < numTeams; x++) {
        winners[cycle].ratings[x] = creature.ratings[x];
    }
    winners[cycle].fitness = creature.fitness;
}

int bestCreature() {
    int best = 0;
    for (int x = 0; x < populationSize; x++) {
        if (population[x].fitness < population[best].fitness) best = x;
    }
    return best;
}

ld bestRating() {
    ld best = RANDOM_MAX_SAFE;
    for (int x = 0; x < populationSize; x++) { best = min(best, population[x].fitness); }
    return best;
}

ld sampleNormal() { // TODO Check if this is actually right
    ld u = ((ld) rand() / (RANDOM_MAX_SAFE)) * 2 - 1;
    ld v = ((ld) rand() / (RANDOM_MAX_SAFE)) * 2 - 1;
    ld r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    ld c = sqrt(-2 * log(r) / r);
    return u * c;
}

ld brierCost(ld probability, int scoreDiff) {
    ld unSquaredCost = (1 - probability + log(scoreDiff));
    return unSquaredCost * unSquaredCost;
}

Score costForGame(Creature creature, int game, Score score, bool isPlayoffs) {
    int winIndex = teamToIndex[allGames[game].winTeam];
    int loseIndex = teamToIndex[allGames[game].loseTeam];
    int scoreDiff = allGames[game].winScore - allGames[game].loseScore;
    int homeBonus = allGames[game].homeGame ? creature.homeAdvantage : 0;

    ld win = winPercentage(homeBonus + creature.ratings[winIndex] - creature.ratings[loseIndex]);

    score.logLoss += logCost(win);
    score.wrongGuess += (win < 0.5);
    score.brier += brierCost(win, scoreDiff);

    if (isPlayoffs) {
        cout << ((win > .5) ? "WIN" : "LOSS") << " " << win << " " << allGames[game].winTeam << " " << allGames[game].loseTeam << endl;
    }

    return score;
}

Score playoffScoreFunction(Creature creature) {
    Score score = newScore();

    for (int x = regularSeason + 1; x < numGamesInSeason; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }
        score = costForGame(creature, x, score, true);
    }

    return score;
}

Score testSetScore(Creature creature) {
    Score score = newScore();

    for (int x = 0; x < regularSeason; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }
        if (testSet.count(x)) {
            score = costForGame(creature, x, score, false);
        }
    }
    return score;
}

Score allScoreFunctions(Creature creature) {
    Score score = newScore();

    for (int x = 0; x < regularSeason; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }
        if (testSet.count(x)) continue; // skip the test set
        score = costForGame(creature, x, score, false);
    }
    return score;
}

// Have we screwed up yet? Returns the average rating of the population
ld validateRatings() {
    ld average = 0;
    for (int x = 0; x < populationSize; x++) {
        // assert (zero(population[x].fitness-costFunction(population[x])));
        average += population[x].fitness;
    }
    return average/populationSize;
}

void normalizeRating(int index) { // Since linear differences are all that matter, normalize them linearly

	ld sum = 0;
	Creature creature = population[index];

	for (int x = 0; x < numTeams; x++) sum += creature.ratings[x];
	for (int x = 0; x < numTeams; x++) creature.ratings[x] += (averageRating*numTeams-sum)/numTeams;
}

void generateRandomAtIndex(int index) {
    for (int x = 0; x < numTeams; x++) {
        population[index].ratings[x] = builtInNormal();
    }
    normalizeRating(index);
    population[index].score = allScoreFunctions(population[index]);
    population[index].fitness = population[index].score.brier;
    population[index].homeAdvantage = builtInNormal()/3; // Mean at 500
}

void generateRandomPopulation() {
	for (int a = 0; a < populationSize; a++) generateRandomAtIndex(a);
}

// Important functions below

void printCreature(Creature creature) { // xcxc WIP

    vector<string> namesToPrint;
    Score score;
    for (int x = 0; x < numTeams; x++) {
        namesToPrint.push_back(names[x]);
        finalRating[names[x]] = creature.ratings[x];
    }

    sort(namesToPrint.begin(),namesToPrint.end(), compareTwoTeams);
    for(int x = 0;x<namesToPrint.size();x++) {
        cout << namesToPrint[x] << " " << finalRating[namesToPrint[x]] << endl; 
    }
    cout << "HOME ADVANTAGE " << creature.homeAdvantage << endl;

    cout << "FITNESS: " << creature.fitness << endl;

    /*score = playoffScoreFunction(creature);

    cout << "PLAYOFF LOG: " << score.logLoss << endl;
    cout << "PLAYOFF BRIER: " << score.brier << endl;
    cout << "PLAYOFF WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "PLAYOFF PERCENTAGE: " << score.wrongGuess/(ld)(numGamesInSeason - regularSeason) << endl;*/

    score = allScoreFunctions(creature);

    cout << "REGULAR LOG: " << score.logLoss << endl;
    cout << "REGULAR BRIER: " << score.brier << endl;
    cout << "REGULAR WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "REGULAR PERCENTAGE: " << score.wrongGuess/(ld)regularSeason << endl;

    score = testSetScore(creature);

    cout << "TEST LOG: " << score.logLoss << endl;
    cout << "TEST BRIER: " << score.brier << endl;
    cout << "TEST WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "TEST PERCENTAGE: " << score.wrongGuess*10/(ld)(regularSeason) << endl;
}

pair<int, int> selectTwoRandomLogWeighting(int total) { // Wrong, maybe not worth trying to debug
    pair<int, int> ans;
    int mod = (int)pow(RANDOM_BASE, total);

    ans.first = magicLogFunction(total);

    int next = magicLogFunction(total);
    while (next == ans.first) {
        next = magicLogFunction(total);
    }

    ans.second = next;
    return ans;
}

void reproduce(int index, int father, int mother) {
    for (int x = 0; x < numTeams; x++) {
        population[index].ratings[x] = (population[father].ratings[x] + population[mother].ratings[x])/2;
    }
    population[index].score = allScoreFunctions(population[index]);
    population[index].fitness = population[index].score.brier;
    population[index].homeAdvantage = (rand() % 2) ? population[father].homeAdvantage : population[mother].homeAdvantage;
}

void cycleGeneration(bool mutation) {
    
    sort(population, population + populationSize, compareCreatures);
    // Note: we never touch the best creature in each round. Don't want to make him suck.

    // Mutation phase. For all survivors we see if we want to mutate (randomize) any genes
    if (mutation) {
        int numMutations = (int)ceil((populationSize-killSize-1)*numTeams/mutationRate);

        for (int x = 0; x < numMutations; x++) {

            int mutatedCreature = rand() % (populationSize-killSize-1) + killSize;
            int mutatedGene = rand() % numTeams;

            population[mutatedCreature].ratings[mutatedGene] = builtInNormal();
            population[mutatedCreature].score = allScoreFunctions(population[mutatedCreature]);
            population[mutatedCreature].fitness = population[mutatedCreature].score.brier;
        }
    }

    // Propogation phase. We just kill all creatures < killSize. This is pretty like gradient descent.
    for (int x = 0; x < killSize; x++) {
        pair<int, int> parents = selectTwoRandom(populationSize-killSize);
        reproduce(x, parents.first + killSize, parents.second + killSize);
    }
}

void evolve(int cycle) { // Not optimized at all
    generateRandomPopulation();

    cout << "Random weight score starts with average rating of " << validateRatings() << endl;

    for (int x = 0; x < maxTimeSteps; x++) {
        cycleGeneration(true);
        if ((x%1000)==0) cout << "Time step of " << x << " has best rating of " << bestRating() << endl;
    }

    for (int x = 0; x < convergeTimeSteps; x++) {
        cycleGeneration(false);
        if ((x%100)==0) cout << "No mutation time step of " << x << " has best rating of " << bestRating() << endl;
    }

    sort(population, population + populationSize, compareCreatures);
    normalizeRating(populationSize-1);
    if (cycle != populationSize) {
        // Save the winners for a meta round.
        copyCreature(cycle, population[populationSize-1]);
    }
}

int main(){
	srand(26);
    initialize();

    cout << regularSeason << " " << numGamesInSeason << endl;

    /*for (int x = 0; x < populationSize; x++) {
        evolve(x);
    }
    population = winners;

    evolve(populationSize);*/
    evolve(0);
    printCreature(population[populationSize-1]);
}

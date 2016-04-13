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
#include <fstream>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <random>
#include <utility>
#define A first
#define B second

using namespace std;
typedef long double ld;
typedef pair<ld, int> pid;


int maxTimeSteps = 400;
const int maxSimulations = 1000; // maybe vary based on confidence interval

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
    ld* homeAdvantage;
    ld fitness; // fitness is calculated on initialization
    ld regular;
    Score score;
    ld restAdvantage;
};

// Base constants
const ld epsilon = 1e-6;
const int numTeams = 30;
const ld DENOM = 400;
const int averageRating = 1500;
int averageHomeAdvantage = 200;

// Genetic constants

const int populationSize = 10;
const int killSize = 5; // Kill all from 0 to killSize
const int mutationRate = 50; // Percentage is 1/mutationRate. We use this for modding.
const int convergeTimeSteps = maxTimeSteps/10;
const ld regularizationConstant = 1;

Game* allGames;
Creature* population;
Creature* winners; // For round 2 of the genetic algorithm with the winners of the first round

map<string, int> lastPlayed;
map<string, string> opponent;
map<string, bool> playoffWinners;
vector<string> homers;

vector<pid> allScores;
map<int, bool> badEggs; // The outliers of the training set to ignore
int outliersToIgnore = 50;


int numGamesInSeason = 0;
int regularSeason = -1; // Number of games in the regular season (probably data loss)

map<string, int> teamToIndex;
string indexToTeam[numTeams];
map<string, ld> finalRating;
map<string, ld> finalHomeAdvantage;
map<string, int> playoffWrong;
vector<string> namesToPrint;
ofstream dump ("dump.txt", fstream::app);




default_random_engine generator;

int testSetWrong = 0;

// Helper functions
int compareCreatures(Creature one, Creature two) { return one.fitness + one.regular > two.fitness + two.regular; } // Sort form high to low 
int compareTwoTeams(string one, string two) { return finalRating[one] > finalRating[two]; } // Sort so the best team is on top
ld randomDouble() { return (ld)rand() / RAND_MAX; }
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

ld builtInNormal(int average, int std) {
    normal_distribution<ld> distribution(average, std);
    return distribution(generator);
}

void initialize() {
    allGames = new Game[100000];
    population = new Creature[populationSize];
    winners = new Creature[populationSize];
    for (int x = 0;x<populationSize; x++) {
        population[x].ratings = new ld[numTeams];
        winners[x].ratings = new ld[numTeams];

        population[x].homeAdvantage = new ld[numTeams];
        winners[x].homeAdvantage = new ld[numTeams];
    }

    string tmp;
    freopen("../NBATeams.txt", "r", stdin);
    while (cin >> tmp) names.push_back(tmp);

    ifstream gameData ("oneYear.txt");
    string winTeam, loseTeam, location;
    int winScore, loseScore, date;

    while (gameData >> winTeam) {
        if (winTeam.find("playoffs")!=-1) { 
            allGames[numGamesInSeason].winTeam = winTeam;
            if (regularSeason == -1) regularSeason = numGamesInSeason;
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
}

Creature duplicate(Creature creature) {
    Creature newCreature;
    newCreature.ratings = new ld[numTeams];
    newCreature.homeAdvantage = new ld[numTeams];

    for (int x = 0; x < numTeams; x++) {
        newCreature.ratings[x] = creature.ratings[x];
        newCreature.homeAdvantage[x] = creature.homeAdvantage[x];
    }
    newCreature.fitness = creature.fitness;
    newCreature.score = creature.score;
    newCreature.regular = creature.regular;
    return newCreature;
}

int bestCreature() {
    int best = 0;
    for (int x = 0; x < populationSize; x++) {
        if (population[x].fitness < population[best].fitness) best = x;
    }
    return best;
}

ld bestRating() {
    ld best = RAND_MAX;
    for (int x = 0; x < populationSize; x++) { best = min(best, population[x].fitness); }
    return best;
}

ld sampleNormal() { // TODO Check if this is actually right
    ld u = ((ld) rand() / (RAND_MAX)) * 2 - 1;
    ld v = ((ld) rand() / (RAND_MAX)) * 2 - 1;
    ld r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    ld c = sqrt(-2 * log(r) / r);
    return u * c;
}

ld regularization(Creature creature) {
    ld cost = 0;
    for (int x = 0;x<numTeams;x++) {
        cost += (creature.ratings[x] - averageRating)/averageRating;
        cost += (creature.homeAdvantage[x] - averageHomeAdvantage)/averageHomeAdvantage;
    }
    cost += creature.restAdvantage;
    return cost*regularizationConstant;
}

ld brierCost(ld probability, int scoreDiff) {
    ld unSquaredCost = (1 - probability + log(scoreDiff));
    return unSquaredCost * unSquaredCost;
}

Score playoffsCostForGame(Creature creature, int game, Score score) {
    int winIndex = teamToIndex[allGames[game].winTeam];
    int loseIndex = teamToIndex[allGames[game].loseTeam];
    int scoreDiff = allGames[game].winScore - allGames[game].loseScore;
    int homeBonus = allGames[game].homeGame ? creature.homeAdvantage[winIndex] : -creature.homeAdvantage[loseIndex];

    ld win = winPercentage(homeBonus + creature.ratings[winIndex] - creature.ratings[loseIndex]);

    score.logLoss += logCost(win);
    score.wrongGuess += (win < 0.5);
    score.brier += brierCost(win, scoreDiff);

    //cout << ((win > .5) ? "RIGHT" : "WRONG") << " " << win << " ";
    //cout << allGames[game].winTeam << " " << allGames[game].loseTeam << " " << allGames[game].homeGame << endl;

    return score;
}

ld brierOnlyCostForGame(Creature creature, int game) {
    int winIndex = teamToIndex[allGames[game].winTeam];
    int loseIndex = teamToIndex[allGames[game].loseTeam];
    int scoreDiff = allGames[game].winScore - allGames[game].loseScore;
    int homeBonus = allGames[game].homeGame ? creature.homeAdvantage[winIndex] : -creature.homeAdvantage[loseIndex];

    ld win = winPercentage(homeBonus + creature.ratings[winIndex] - creature.ratings[loseIndex]);
    return brierCost(win, scoreDiff);
}

Score costForGame(Creature creature, int game, Score score, bool print, ld fatigueBonus) {
    int winIndex = teamToIndex[allGames[game].winTeam];
    int loseIndex = teamToIndex[allGames[game].loseTeam];
    int scoreDiff = allGames[game].winScore - allGames[game].loseScore;
    int homeBonus = allGames[game].homeGame ? creature.homeAdvantage[winIndex] : -creature.homeAdvantage[loseIndex];

    ld win = winPercentage(fatigueBonus + homeBonus + creature.ratings[winIndex] - creature.ratings[loseIndex]);

    if (print) {
        cout << win << " " << allGames[game].winTeam << " " << allGames[game].loseTeam << endl;

    }

    score.logLoss += logCost(win);
    score.wrongGuess += (win < 0.5);
    score.brier += brierCost(win, scoreDiff);

    return score;
}

ld simulateTwoPlayers(ld homeWinRate, ld awayWinRate) { // home team first

    // xcxc go until you are confident

    // NBA format is 2-2-1-1-1
    int seriesWinner = 0;

    for (int x = 0; x < maxSimulations; x++) {
        // cout << randomDouble() << endl;
        int win = 0;
        for (int a = 0; a < 4; a++) if (randomDouble() < homeWinRate) win++;
        for (int a = 0; a < 3; a++) if (randomDouble() < awayWinRate) win++;

        if (win >= 4) seriesWinner++;
    }
    // cout << seriesWinner << endl;

    return seriesWinner/(ld)maxSimulations;
}

void checkOdds(Creature creature, bool useHome) {
    int correct = 0;
    for (int x = 0;x<homers.size();x++) {
        int firstTeamIndex = teamToIndex[homers[x]];
        int secondTeamIndex = teamToIndex[opponent[homers[x]]];

        ld homeWin = winPercentage((useHome ? creature.homeAdvantage[firstTeamIndex] : 0) +
            creature.ratings[firstTeamIndex] - creature.ratings[secondTeamIndex]);

        ld awayWin = winPercentage((useHome ? -creature.homeAdvantage[secondTeamIndex] : 0) +
            creature.ratings[firstTeamIndex] - creature.ratings[secondTeamIndex]);

        // cout << homers[x] << " " << opponent[homers[x]] << " " << homeWin << " ";
        // cout << awayWin << " " << simulateTwoPlayers(homeWin, awayWin) << endl;
        correct += playoffWinners[homers[x]]==(simulateTwoPlayers(homeWin, awayWin)>.5);
    }
    dump << correct << " ";

}

void playoffCorrect(Creature creature) {
    opponent.clear();
    homers.clear();
    playoffWinners.clear();

    for (int x = regularSeason + 1; x < numGamesInSeason; x++) {
        string winTeam = allGames[x].winTeam;
        string loseTeam = allGames[x].loseTeam;

        if (winTeam == "playoffs") { assert(false); }
        // if (winTeam == "playoffs2") break;
        if(opponent[winTeam] == "") {

            opponent[winTeam] = loseTeam;
            opponent[loseTeam] = winTeam;

            if (allGames[x].homeGame) homers.push_back(winTeam);
            else homers.push_back(loseTeam);

        } else if (opponent[winTeam] != loseTeam) {
            break;
        } else {
            // assert (opponent[winTeam] == loseTeam);
            assert (opponent[loseTeam] == winTeam);
            playoffWinners[winTeam] = true;
            playoffWinners[loseTeam] = false; // The person who wins the last in the series wins the series
        }
    }
    assert(homers.size() == 8);

    checkOdds(creature, true);
    checkOdds(creature, false);
}

Score playoffScoreFunction(Creature creature) {
    Score score = newScore();

    for (int x = regularSeason + 1; x < numGamesInSeason; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }
        // if (allGames[x].winTeam == "playoffs2") break;
        score = playoffsCostForGame(creature, x, score);
    }

    return score;
}

Score testSetScore(Creature creature) {
    Score score = newScore();

    for (int x = 0; x < regularSeason; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }
        if (testSet.count(x)) {
            int winBonus = (allGames[x].date-lastPlayed[allGames[x].winTeam])>1;
            int loseBonus = (allGames[x].date-lastPlayed[allGames[x].loseTeam])>1;

            score = costForGame(creature, x, score, false, (winBonus-loseBonus)*creature.restAdvantage);
        }
        lastPlayed[allGames[x].winTeam] = allGames[x].date;
        lastPlayed[allGames[x].loseTeam] = allGames[x].date;
    }
    testSetWrong += score.wrongGuess;
    return score;
}

Score allScoreFunctions(Creature creature) {
    Score score = newScore();

    int start = 0;
    int end = regularSeason;

    lastPlayed.clear();

    for (int x = start; x < end; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }
        if (testSet.count(x)==0 && !badEggs[x]) {

            int winBonus = (allGames[x].date-lastPlayed[allGames[x].winTeam])>1;
            int loseBonus = (allGames[x].date-lastPlayed[allGames[x].loseTeam])>1;

            score = costForGame(creature, x, score, false, (winBonus-loseBonus)*creature.restAdvantage);
        }

        lastPlayed[allGames[x].winTeam] = allGames[x].date;
        lastPlayed[allGames[x].loseTeam] = allGames[x].date;


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
        population[index].ratings[x] = builtInNormal(averageRating, 500);
        population[index].homeAdvantage[x] = builtInNormal(averageHomeAdvantage, 100);
    }
    population[index].restAdvantage = builtInNormal(averageHomeAdvantage/6, 10); // Rest advantage is 3% Home advantage is 20%

    normalizeRating(index);
    population[index].score = allScoreFunctions(population[index]);
    population[index].fitness = population[index].score.brier;
    population[index].regular = regularization(population[index]);
}

void generateRandomPopulation() {
	for (int a = 0; a < populationSize; a++) generateRandomAtIndex(a);
}

// Important functions below

void printScore(Creature creature) {
    cout << "FITNESS: " << creature.fitness << endl;
    
    Score score = allScoreFunctions(creature);

    cout << "REGULAR LOG: " << score.logLoss << endl;
    cout << "REGULAR BRIER: " << score.brier << endl;
    cout << "REGULAR WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "REGULAR PERCENTAGE: " << score.wrongGuess/(ld)regularSeason << endl;
}

void printCreature(Creature creature, bool isPlayoffs) {

    namesToPrint.clear();
    Score score;
    for (int x = 0; x < numTeams; x++) {
        namesToPrint.push_back(names[x]);
        finalRating[names[x]] = creature.ratings[x];
        finalHomeAdvantage[names[x]] = creature.homeAdvantage[x];
    }

    sort(namesToPrint.begin(),namesToPrint.end(), compareTwoTeams);
    for(int x = 0;x<namesToPrint.size();x++) {
        // cout << namesToPrint[x] << " " << finalRating[namesToPrint[x]] << " " << finalHomeAdvantage[namesToPrint[x]] << endl; 
    }

    cout << "FITNESS: " << creature.fitness << endl;
    
    score = allScoreFunctions(creature);

    cout << "REGULAR LOG: " << score.logLoss << endl;
    cout << "REGULAR BRIER: " << score.brier << endl;
    cout << "REGULAR WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "REGULAR PERCENTAGE: " << score.wrongGuess/(ld)regularSeason << endl;

    if (isPlayoffs) {
        score = playoffScoreFunction(creature);

        cout << "PLAYOFF LOG: " << score.logLoss << endl;
        cout << "PLAYOFF BRIER: " << score.brier << endl;
        cout << "PLAYOFF WRONG GUESSES: " << score.wrongGuess << endl;
        cout << "PLAYOFF PERCENTAGE: " << score.wrongGuess/(ld)(numGamesInSeason - regularSeason) << endl;

        // playoffCorrect(creature);
    } else {
        score = testSetScore(creature);

        cout << "TEST LOG: " << score.logLoss << endl;
        cout << "TEST BRIER: " << score.brier << endl;
        cout << "TEST WRONG GUESSES: " << score.wrongGuess << endl;
        cout << "TEST PERCENTAGE: " << score.wrongGuess*10/(ld)(regularSeason) << endl;
    }

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
        population[index].homeAdvantage[x] = (population[father].homeAdvantage[x] + population[mother].homeAdvantage[x])/2;
    }
    population[index].restAdvantage = (population[father].restAdvantage + population[mother].restAdvantage)/2;

    population[index].score = allScoreFunctions(population[index]);
    population[index].fitness = population[index].score.brier;
    population[index].regular = regularization(population[index]);
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

            population[mutatedCreature].ratings[mutatedGene] = builtInNormal(averageRating, 500);
            population[mutatedCreature].fitness = 0;

            // Different mutation for the homeGameAdvantage
            mutatedCreature = rand() % (populationSize-killSize-1) + killSize;
            mutatedGene = rand() % numTeams;

            population[mutatedCreature].homeAdvantage[mutatedGene] = builtInNormal(averageHomeAdvantage, 100);
            population[mutatedCreature].fitness = 0;

        }
        for (int x = 0; x < populationSize; x++) {
            if (population[x].fitness == 0) {
                population[x].score = allScoreFunctions(population[x]);
                population[x].fitness = population[x].score.brier;
                population[x].regular = regularization(population[x]);
            }
        }
    }

    // Propogation phase. We just kill all creatures < killSize. This is pretty like gradient descent.
    for (int x = 0; x < killSize; x++) {
        pair<int, int> parents = selectTwoRandom(populationSize-killSize);
        reproduce(x, parents.first + killSize, parents.second + killSize);
    }
}

void evolve(int cycle, bool mutate = true) { // Not optimized at all
    bool finalRound = (cycle==populationSize);
    if (!finalRound) generateRandomPopulation(); // For the last one we already the population

    // cout << "Random weight score starts with average rating of " << validateRatings() << endl;

    if(finalRound) maxTimeSteps *= 10;

    for (int x = 0; x < maxTimeSteps; x++) {
        cycleGeneration(mutate); // no mutation on the last one
        //if ((x%1000)==0) cout << "Time step of " << x << " has best rating of " << bestRating() << endl;
    }

    for (int x = 0; x < convergeTimeSteps; x++) {
        cycleGeneration(false);
        // cout << "No mutation time step of " << x << " has best rating of " << bestRating() << endl;
    }

    sort(population, population + populationSize, compareCreatures);
    normalizeRating(populationSize-1);
    // printScore(population[populationSize-1]);
    if (cycle != populationSize) {
        // Save the winners for a meta round.
        winners[cycle] = duplicate(population[populationSize-1]);
    }
}

void liveGeneration(bool isPlayoffs) {
    for (int x = 0; x < populationSize; x++) {
        evolve(x);
    }
    for (int x = 0;x<populationSize;x++) {
        population[x] = winners[x];
    }

    evolve(populationSize, false);
    // printCreature(population[populationSize-1], isPlayoffs);
}

void generateTestSet() {
    testSet.clear();
    int cnt = 0;
    while(cnt < regularSeason/10) {
        int pos = rand() % regularSeason;
        if (!testSet.count(pos)) {
            cnt++;
            testSet.insert(pos);
            // cout << pos << " ";
        }
    }
    cout << endl;
}

void findBadEggs() {
    Creature creature = population[populationSize-1];
    allScores.clear();
    badEggs.clear();

    for (int x = 0; x < regularSeason; x++) {
        if (allGames[x].winTeam == "playoffs") { assert(false); }

        if (testSet.count(x)) {
            allScores.push_back(pid(0,x)); // skip the test set
        } else {
            allScores.push_back(pid(brierOnlyCostForGame(creature, x), x));
        }
    }
    sort(allScores.begin(), allScores.end());
    reverse(allScores.begin(), allScores.end());

    for (int x = 0; x < outliersToIgnore; x++) {
        badEggs[allScores[x].second] = true;
    }
}

int main() {
    
    int seed = 14;

    cout << "GIVE SEED\n"; cin >> seed;
	srand(seed); //1241232 is a good number


    initialize();

    // cout << regularSeason << " " << numGamesInSeason << endl;

    // generateTestSet();
    
    liveGeneration(true);
    Creature best = population[populationSize-1];

    ld noThrowTrain = allScoreFunctions(best).wrongGuess/(ld)regularSeason;
    ld noThrowPlayoffs = playoffScoreFunction(best).wrongGuess/(ld)(numGamesInSeason - regularSeason);
    playoffCorrect(best);

    findBadEggs();
    liveGeneration(true);
    best = population[populationSize-1];
    ld throwTrain = allScoreFunctions(best).wrongGuess/(ld)regularSeason;
    ld throwPlayoffs = playoffScoreFunction(best).wrongGuess/(ld)(numGamesInSeason - regularSeason);
    playoffCorrect(best);
    
    dump << noThrowTrain << " " << noThrowPlayoffs << " " << throwTrain << " " << throwPlayoffs << "\n";

}

/**

Genetic algorithm to minimize a given function

DENOM is 400 so I can line up approximately with 538. Mean is held at 1500 for the same reason

Sanity checked with ELO output from validateElo.cpp to make sure it made sense.
Elo output with 538 constants is a cost of 778.981.
Random ratings seem to have costs double this.


Optimizations:

1) Kill organisms in a more realistic way




2) Weight parent selection in favor of better creatures DONE
3) Actually optimize calculation in parts of code DONE
4) Try mixing parents instead of averaging? PROBABLY DOESNT MATTER
5) Initialize non randomly? DOESNT MATTER
6) Linear Transformations DONE
7) Use brier scores or accuracy DONE
8) Weighted random children DONE


We are getting close. Got the spurs vs clippers AND the rockets vs clippers without any extra data
srand 7 got cavs > hawks, but srand 8 didn't. srand 8 also had the lower loss function :/.

BUT THIS IS AMAZING CONSIDERING THE NO INPUT AND NO TWEAKING. There is a lot to do.

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
#include <time.h>
#include <algorithm>

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
const ld expBase = 10;

// Genetic constants
const int populationSize = 10;
const int killSize = 5; // Kill all from 0 to killSize
const int mutationRate = 50; // Percentage is 1/mutationRate. We use this for modding.
const int maxTimeSteps = 4000; // 5k total seems good
const int convergeTimeSteps = 500;
int regularSeason = -1;

ld ELO_RATINGS[] = {1524.06, 1360.66, 1634.58, 1550.66, 1627.52, 1352.95, 1431.03, 1461.23, 1505.31, 1317.65, 1502.1, 1495.59, 1470.91, 1533.77, 1497.83, 1573.84, 1601.16, 1333.95, 1543.81, 1339.3, 1427.83, 1441.91, 1505.42, 1478.93, 1718.75, 1412.11, 1536.67, 1551.66, 1639.06, 1629.73};

Game* allGames;
Creature* population;
Creature* winners;
int numGamesInSeason = 0;
int trainGames = 0;
map<string, int> teamToIndex;
string indexToTeam[numTeams];
map<string, ld> finalRating;
map<string, int> playoffWrong;

int compareCreatures(Creature one, Creature two) {

    return one.fitness > two.fitness; // Sort form high to low
}

double randomDouble() {
    return (double)rand() / RAND_MAX;
}

ld sampleNormal() {
    ld u = ((ld) rand() / (RANDOM_MAX_SAFE)) * 2 - 1;
    ld v = ((ld) rand() / (RANDOM_MAX_SAFE)) * 2 - 1;
    ld r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    ld c = sqrt(-2 * log(r) / r);
    return u * c;
}

ld winPercentage (ld eloDiff) {
    return 1/(ld)(1+pow(expBase, -eloDiff/DENOM));
}

ld logCost(ld probability) {
    return -log(probability); // logistic cost function given you win
}

ld brierCost(ld probability, int scoreDiff) {

    ld unSquaredCost = (1 - probability + log(scoreDiff)); // Remove scoreDiff when testing without it.

    return unSquaredCost * unSquaredCost;
}

Score playoffScoreFunction(Creature creature) {
    Score score;
    score.logLoss = 0;
    score.wrongGuess = 0;
    score.brier = 0;

    for (int x = regularSeason + 1; x < numGamesInSeason; x++) {
        if (allGames[x].winTeam == "playoffs") {
            assert(false);
        }
        int winIndex = teamToIndex[allGames[x].winTeam];
        int loseIndex = teamToIndex[allGames[x].loseTeam];
        int scoreDiff = allGames[x].winScore - allGames[x].loseScore;

        int homeBonus = allGames[x].homeGame ? creature.homeAdvantage : 0;

        ld win = winPercentage(homeBonus + creature.ratings[winIndex] - creature.ratings[loseIndex]);
        score.logLoss += logCost(win);
        score.wrongGuess += (win < 0.5);
        score.brier += brierCost(win, scoreDiff);

        /*printer.clear();
        if (win < .5) {
            playoffWrong[]
        }
        for (int x = 0; x < printer.size(); x++) {
            cout << printer[x] << endl;
        }*/
    }
    return score;
}

Score allScoreFunctions(Creature creature) { // TODO MERGE THIS WITH PLAYOFF CODE SCORE
    Score score;
    score.logLoss = 0;
    score.wrongGuess = 0;
    score.brier = 0;

    for (int x = 0; x < numGamesInSeason; x++) {
        if (allGames[x].winTeam == "playoffs") {
            break;
        }
        int winIndex = teamToIndex[allGames[x].winTeam];
        int loseIndex = teamToIndex[allGames[x].loseTeam];
        int scoreDiff = allGames[x].winScore - allGames[x].loseScore;

        int homeBonus = allGames[x].homeGame ? creature.homeAdvantage : 0;

        ld win = winPercentage(homeBonus + creature.ratings[winIndex] - creature.ratings[loseIndex]);
        score.logLoss += logCost(win);
        score.wrongGuess += (win < 0.5);
        score.brier += brierCost(win, scoreDiff);
    }
    return score;
}

bool zero(ld in) {
    if(abs(in)<epsilon) {
        return true;
    } else {
        return false;
    }
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

int bestCreature() {
    int best = 0;
    for (int x = 0; x < populationSize; x++) {
        if (population[x].fitness < population[best].fitness) {
            best = x;
        }
    }
    return best;
}

ld bestRating() {
    ld best = 1000000;
    for (int x = 0; x < populationSize; x++) {
        best = min(best, population[x].fitness);
    }
    return best;
}

void normalizeRating(int index) { // Since linear differences are all that matter, normalize them this way

	ld sum = 0;

	Creature creature = population[index];

	for (int x = 0; x < numTeams; x++) {
		sum += creature.ratings[x];
	}
	for (int x = 0; x < numTeams; x++) {
		creature.ratings[x] += (averageRating*32-sum)/32;
	}
}

void generateRandomAtIndex(int index) {
    for (int x = 0; x < numTeams; x++) {
        population[index].ratings[x] = sampleNormal();
    }
    normalizeRating(index);
    population[index].score = allScoreFunctions(population[index]);
    population[index].fitness = population[index].score.brier;
    population[index].homeAdvantage = 5 * sampleNormal() + 100;
}

void generateRandomPopulation() {
	for (int a = 0; a < populationSize; a++) {
		generateRandomAtIndex(a);
	}
}

int compareTwoTeams(string one, string two){
    return finalRating[one] > finalRating[two]; // Sort so the best team is on top
}

void printCreature(Creature creature) {

    vector<string> namesToPrint;
    for (int x = 0; x < numTeams; x++) {
        namesToPrint.push_back(names[x]);
        finalRating[names[x]] = creature.ratings[x];
    }

    sort(namesToPrint.begin(),namesToPrint.end(), compareTwoTeams);
    for(int x = 0;x<namesToPrint.size();x++) {
        cout << namesToPrint[x] << " " << finalRating[namesToPrint[x]] << endl; 
    }

    cout << "FITNESS: " << creature.fitness << endl;

    Score score = playoffScoreFunction(creature);

    cout << "LOG: " << score.logLoss << endl;
    cout << "BRIER: " << score.brier << endl;
    cout << "WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "PLAYOFF GAMES: " << numGamesInSeason - regularSeason << endl;

    score = allScoreFunctions(creature);

    cout << "LOG: " << score.logLoss << endl;
    cout << "BRIER: " << score.brier << endl;
    cout << "WRONG GUESSES: " << score.wrongGuess << endl;
    cout << "PLAYOFF GAMES: " << numGamesInSeason - regularSeason << endl;
}

// For now we just select two compelely randomly. No weighting.
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

// Hacky implementation where person of rank X is multiplicatively more likely to get selected as rank X+1
const double RANDOM_BASE = 1.3;

int magicLogFunction(int total) {
    return (int)floor(log(rand()%total)/log(RANDOM_BASE));
}

pair<int, int> selectTwoRandomLogWeighting(int total) {
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
    
    // clock_t t1,t2,t3,t4;
    // t1 = clock();

    sort(population, population + populationSize, compareCreatures);
    // Note: we never touch the best creature in each round. Don't want to make him suck.

    // t2 = clock();

    // Mutation phase. For all survivors we see if we want to mutate (randomize) any genes
    if (mutation) {

        // Can definitely use 1337 skillz to optimize this


        int numMutations = (int)ceil((populationSize-killSize-1)*numTeams/mutationRate);

        for (int x = 0; x < numMutations; x++) {

            int mutatedCreature = rand() % (populationSize-killSize-1) + killSize;
            int mutatedGene = rand() % numTeams;

            population[mutatedCreature].ratings[mutatedGene] = sampleNormal()*averageRating;
            population[mutatedCreature].score = allScoreFunctions(population[mutatedCreature]);
            population[mutatedCreature].fitness = population[mutatedCreature].score.brier;
        }
    }

    // t3 = clock();

    // Propogation phase. We just kill all creatures < killSize. This is pretty like gradient descent.

    for (int x = 0; x < killSize; x++) {
        pair<int, int> parents = selectTwoRandom(populationSize-killSize);
        reproduce(x, parents.first + killSize, parents.second + killSize);
    }

    // t4 = clock();
    // cout << (t2-t1)/(float) CLOCKS_PER_SEC << endl;
    // cout << (t3-t2)/(float) CLOCKS_PER_SEC << endl;
    // cout << (t4-t3)/(float) CLOCKS_PER_SEC << endl;
}

// This is really bad but whatever
void copyCreature(int cycle, Creature creature) {
    for (int x = 0; x < numTeams; x++) {
        winners[cycle].ratings[x] = creature.ratings[x];
    }
    winners[cycle].fitness = creature.fitness;
}

void evolve(int cycle) { // Not optimized at all
    generateRandomPopulation();

    for (int x = 0; x < maxTimeSteps; x++) {
        cycleGeneration(true);
        int print = bestCreature();
        if (x==1000) {
            printCreature(population[print]);
        }
        if ((x%1000)==0) cout << population[print].fitness << " " << population[print].score.wrongGuess << endl;
    }

    for (int x = 0; x < convergeTimeSteps; x++) {
        cycleGeneration(false);
        if ((x%100)==0) cout << "VAL" << bestRating() << endl;
    }

    sort(population, population + populationSize, compareCreatures);
    normalizeRating(populationSize-1);
    printCreature(population[populationSize-1]);
    if (cycle != populationSize) {
        copyCreature(cycle, population[populationSize-1]);
    }
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
    while (cin >> tmp) {
        names.push_back(tmp);
        rating[tmp] = 2;
    }
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

    trainGames = floor(numGamesInSeason*.9);

    ifstream NBA ("../NBATeams.txt");
    for (int x = 0; x<numTeams; x++) {
        NBA >> tmp;
        indexToTeam[x] = tmp;
        teamToIndex[tmp] = x;
    }
}

int main(){

	srand(8);
    initialize();

    cout << regularSeason << " " << regularSeason * .25 << endl;

    evolve(0);
    return 0;

    for (int x = 0; x < populationSize; x++) {
        evolve(x);
    }
    population = winners;

    evolve(populationSize);

}

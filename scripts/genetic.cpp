/**

Genetic algorithm to minimize a given function

DENOM is 400 so I can line up approximately with 538. Mean is held at 1500 for the same reason

Sanity checked with ELO output from validateElo.cpp to make sure it made sense.
Elo output with 538 constants is a cost of 778.981.
Random ratings seem to have costs double this.


Optimizations:

1) Kill organisms in a more realistic way
2) Weight parent selection in favor of better creatures
3) Actually optimize calculation in parts of code
4) Try mixing parents instead of averaging?
5) Initialize non randomly?
6) Linear Transformations DONE
7) Use brier scores or accuracy
8) Weighted random children


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
    ld fitness; // fitness is calculated on initialization
} ELO;

struct Score {
    int wrongGuess;
    ld brier;
    ld logLoss;
};

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
const int maxTimeSteps = 9000; // 10k total seems good
const int convergeTimeSteps = 1000;


ld ELO_RATINGS[] = {1524.06, 1360.66, 1634.58, 1550.66, 1627.52, 1352.95, 1431.03, 1461.23, 1505.31, 1317.65, 1502.1, 1495.59, 1470.91, 1533.77, 1497.83, 1573.84, 1601.16, 1333.95, 1543.81, 1339.3, 1427.83, 1441.91, 1505.42, 1478.93, 1718.75, 1412.11, 1536.67, 1551.66, 1639.06, 1629.73};

Game* allGames;
Creature* population;
int numGamesInSeason = 0;
int trainGames = 0;
map<string, int> teamToIndex;
string indexToTeam[numTeams];
map<string, ld> finalRating;

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

ld brierCost(ld probability) {
    return (1-probability)*(1-probability);
}

Score allScoreFunctions(Creature creature) {
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

        ld win = winPercentage(creature.ratings[winIndex] - creature.ratings[loseIndex]);
        score.logLoss += logCost(win);
        score.wrongGuess += (win < 0.5);
        score.brier += brierCost(win);
    }
    return score;
}


ld costFunction(Creature creature) { // Can also swap it out for a linear function
    return allScoreFunctions(creature).brier;
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
    population[index].fitness = costFunction(population[index]);
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

    Score score = allScoreFunctions(creature);

    cout << "LOG: " << score.logLoss << endl;
    cout << "BRIER: " << score.brier << endl;
    cout << "WRONG GUESSES: " << score.wrongGuess << endl;
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
    population[index].fitness = costFunction(population[index]);
}

void cycleGeneration(bool mutation) {
    sort(population, population + populationSize, compareCreatures);
    // Note: we never touch the best creature in each round. Don't want to make him suck.

    // Mutation phase. For all survivors we see if we want to mutate (randomize) any genes
    if (mutation) {

        // Can definitely use 1337 skillz to optimize this

        int numMutations = (int)ceil((populationSize-killSize-1)*numTeams/mutationRate);

        for (int x = 0; x < numMutations; x++) {

            int mutatedCreature = rand() % (populationSize-killSize-1) + killSize;
            int mutatedGene = rand() % numTeams;

            population[mutatedCreature].ratings[mutatedGene] = sampleNormal()*averageRating;
            population[mutatedCreature].fitness = costFunction(population[mutatedCreature]);
        }
    }

    // Propogation phase. We just kill all creatures < killSize. This is pretty like gradient descent.

    for (int x = 0; x < killSize; x++) {
        pair<int, int> parents = selectTwoRandom(populationSize-killSize);
        reproduce(x, parents.first + killSize, parents.second + killSize);
    }
}

void evolve() { // Not optimized at all
    generateRandomPopulation();

    for (int x = 0; x < maxTimeSteps; x++) {
        cycleGeneration(true);
        if ((x%100)==0) cout << bestRating() << endl;
    }

    for (int x = 0; x < convergeTimeSteps; x++) {
        cycleGeneration(false);
        if ((x%100)==0) cout << "VAL" << validateRatings() << endl;
    }

    sort(population, population + populationSize, compareCreatures);
    normalizeRating(populationSize-1);
    printCreature(population[populationSize-1]);
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
    ifstream gameData ("oneYear.txt");

    string winTeam, loseTeam, location;
    int winScore, loseScore;

    while (gameData >> winTeam) {
        if (winTeam == "playoffs") { 
            allGames[numGamesInSeason].winTeam = winTeam;
        } else if (winTeam == "yearEnd") { // separation of years
            allGames[numGamesInSeason].winTeam = winTeam;
        } else {
            gameData >> winScore >> loseTeam >> loseScore >> location;

            allGames[numGamesInSeason].winTeam   = winTeam;
            allGames[numGamesInSeason].loseTeam  = loseTeam;
            allGames[numGamesInSeason].loseScore = loseScore;
            allGames[numGamesInSeason].winScore  = winScore;
            allGames[numGamesInSeason].homeGame  = location == "home" ? true : false;
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

    for (int x = 0; x < 10; x++) {
        evolve();
    }
}

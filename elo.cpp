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

using namespace std;
typedef long double ld;
map<string,ld> rating;
map<string,bool> vis;

bool win;
int totalGames = 1314;
int regularSeason = 1229;
int firstRound = 1274;
int confFinals = 1296;
int finals = 1307;
string one,two;
int P1,P2;
ld EA,EB,QA,QB;
int K = 16;
int startElo = 1500;
int correct = 0,original = 0;
vector<string> names;

int comp(string one, string two){
    return rating[one]<rating[two];
}
bool update(){
    bool answer = false;
    cin >> one >> P1 >> two >> P2;
    if(!vis[one]){
        vis[one] = true;
        rating[one] = startElo;
        names.push_back(one);
    }
    if(!vis[two]){
        vis[two] = true;
        rating[two] = startElo;
        names.push_back(two);
    }
    if(rating[one]>rating[two]==P1>P2) answer = true;
    QA = pow(10,rating[one]/400); 
    QB = pow(10,rating[two]/400); 
    EA = QA/(QA+QB);
    EB = QB/(QA+QB);
    win = P1>P2;  
    rating[one] += K*(win-EA);
    rating[two] += K*(!win-EA);
    
    return answer;
}



int main(){
    freopen("DATA.txt","r",stdin);
    for(int x = 0;x<finals;x++){
        if(update()) original++; 
    }
    sort(names.begin(),names.end(),comp);
    for(int x = 0;x<names.size();x++) cout << names[x] << " " << rating[names[x]] << endl;
    cout << endl;
}

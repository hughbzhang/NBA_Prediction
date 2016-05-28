 #include <iostream>

using namespace std;

 int main() {


 	int K, HOME_ADVANTAGE_CONSTANT;
 	long double SCORE_WEIGHT_CONSTANT;

    for (K = 8;K<64;K*=2) {
        for (HOME_ADVANTAGE_CONSTANT = 300; HOME_ADVANTAGE_CONSTANT < 600; HOME_ADVANTAGE_CONSTANT+=50) {
            for (SCORE_WEIGHT_CONSTANT = 0.1; SCORE_WEIGHT_CONSTANT <= 3.2; SCORE_WEIGHT_CONSTANT*=2) {
                cout << K << " " << HOME_ADVANTAGE_CONSTANT << " " << SCORE_WEIGHT_CONSTANT << ",";
            }
        }
    }
    cout << endl;
}
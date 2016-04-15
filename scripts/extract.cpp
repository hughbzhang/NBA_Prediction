#include <cstdio>
#include <iostream>

using namespace std;

int marker;
string stuff;
int main() {
    while (cin >> marker) {
        for (int x = 0;x<20;x++) {
            cin >> stuff;
            if (marker == 8) {
                cout << stuff << ",";
                if(x%4==3) cout << endl;
            }
        }
    }
}

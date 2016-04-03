/**
Append Date and OT to 
*/

#include <cstdio>
#include <iostream>
#include <string>
#include <assert.h>
#include <sstream>
#include <map>

using namespace std;

// Input data. I don't care about leap year. Just assume 366 days every year
string months[12] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
int days[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

// Calculated data based ONLY on input data [easier to debug this way]
int cumDays[13];
map<string, int> isMonth;

int main() {

	for (int x = 0;x<12;x++) {
		cumDays[x+1] = cumDays[x] + days[x];
		isMonth[months[x]] = x+1; // 0 is default map value
	}

	string line, word;

	int day, year;

	int date = -1;

	while(getline (cin, line)) {

	    istringstream iss(line);
	    while(iss >> word) {
	    	if (isMonth[word]) {
				iss >> day >> year;
				int month = isMonth[word]-1;
				date = cumDays[month]+day;
				if (date < 250) date += 365; // So that we can calculate Dec 31 -> Jan 1 properly
				break;
			}
	    }
	    if (!isMonth[word]) {
	    	
	    	cout << line << " " << date << "\n";
	    }
	}
}
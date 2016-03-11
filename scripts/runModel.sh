# This script runs my elo model on all the data I have

g++ -o model model.cpp

for file in ../PARSED_DATA/*
do
	cp ../initialRanks.txt ranks.txt
    ./model <$file
    cat ranks.txt
done


rm model
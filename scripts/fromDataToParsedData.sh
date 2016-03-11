# As you might have guessed, if you populate the DATA folder with the right data
# this script will populate the PARSED_DATA folder with the right data

g++ -o homeGameScript homeGame.cpp 
g++ -o validate validate.cpp

for file in ../DATA/*;
	do
	./parser.sh $file; # Parser gets the prelimary parsing data into tmp1.txt
	./homeGameScript <tmp1.txt >tmp2.txt; # Puts home and away data into tmp.txt
	./replaceNameChanges.sh tmp2.txt; # Replaces team names and puts that into tmp.txt

	./validate <tmp2.txt; # Validate the data
	mv tmp2.txt "../PARSED_DATA/"${file:8:4}".txt"; #Puts everything together
done

# Clean up
rm validate
rm homeGameScript
rm tmp1.txt

# As you might have guessed, if you populate the DATA folder with the right data
# this script will populate the PARSED_DATA_WITH_DATE_AND_OT folder with the right data

g++ -o homeGameScript homeGame.cpp 
g++ -o validate validate.cpp
g++ -o date dateParser.cpp

for file in ../DATA/*;
	do
	./parser.sh $file; # Parser gets the prelimary parsing data into tmp1.txt
	./date <tmp1.txt >tmp.txt
	sed -i 's/^.\{,20\}$//g' tmp.txt # Remove all lines less than 20 chars long (dates)
	sed -i 's/\([a-z]\)\s\+\([a-z]\)/\1\2/g' tmp.txt # Make team names one word
	sed -i 's/fulllist.*/playoffs/g' tmp.txt # Edge case marking the beginning of playoffs

	./homeGameScript <tmp.txt >tmp2.txt; # Puts home and away data into tmp.txt
	./replaceNameChanges.sh tmp2.txt; # Replaces team names and puts that into tmp.txt

	./validate <tmp2.txt; # Validate the data
	mv tmp2.txt "../PARSED_DATA_WITH_DATE_AND_OT/"${file:8:4}".txt"; #Puts everything together
done

# Clean up
rm validate
rm homeGameScript
rm date
rm tmp1.txt
rm tmp.txt

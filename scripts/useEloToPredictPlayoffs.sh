g++ -o eloPlayoffs eloPlayoffRatings.cpp;

rm eloPlayoffConstantData.txt

for file in ../PARSED_DATA_WITH_DATE_AND_OT/*; do
	cp "Top20RatingsPerYear/"${file:32:4}"Rating.txt" ratings.txt
	cp $file oneYear.txt

	./eloPlayoffs >>eloPlayoffConstantData.txt

done

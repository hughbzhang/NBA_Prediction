g++ -o genetic genetic.cpp;

rm thisYearResults.txt;
cp thisYear.txt oneYear.txt

for i in {1001..1050}; do
	echo $i > in.txt
	./genetic < in.txt >>thisYearResults.txt
done

g++ -o genetic genetic.cpp;

for file in ../PARSED_DATA_WITH_DATE_AND_OT/*; do
	echo > ${file:32:4}"Rating.txt"

	cp $file oneYear.txt
	for i in {401..410}; do
		echo $i > in.txt
		./genetic <in.txt >>${file:32:4}"Rating.txt"
	done
done

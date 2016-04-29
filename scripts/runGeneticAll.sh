g++ -O3 -o genetic genetic.cpp;

echo > dump.txt;

for file in ../PARSED_DATA_WITH_DATE_AND_OT/*; do
	cp $file oneYear.txt
	step="0.25"
	echo "NEWYEAR" >> dump.txt
	for (( a=4; $a<=$(bc<<<"2/$step"); a++ )); do
		echo $a >> dump.txt
		for i in {401..405}; do
			echo $i $(bc<<<"$step * $a") > in.txt
			./genetic < in.txt
		done
	done
done	
g++ genetic.cpp;

echo > dump.txt;

for file in ../PARSED_DATA_WITH_DATE_AND_OT/*
	do
		cp $file oneYear.txt
		for i in {201..210}
			do
				echo $i 1 > in.txt
				./a.out <in.txt
			done
	done

mv dump.txt assert.txt
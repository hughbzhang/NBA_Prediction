g++ genetic.cpp;

echo > dump.txt;

for file in ../PARSED_DATA_WITH_DATE_AND_OT/*
	do
		cp $file oneYear.txt
		for i in {201..210}
			do
				echo $i > in.txt
				time ./a.out < in.txt
			done
	done
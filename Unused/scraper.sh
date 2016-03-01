#This is my bash script from downloading all the basketball win loss data between teams from landofbasketball.com

cd AllData
for i in {2001..2012};
do
    wget 'http://www.landofbasketball.com/results/'$i'_'`expr $i + 1`'_scores_full.htm'
done

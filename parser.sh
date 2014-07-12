#This is my parser for the basketball data gathered from landofbasketball.com
cp $1'314.txt' cur.txt

gsed -i 's/,//g' cur.txt 
gsed -i 's/[0-9]*OT//g' cur.txt
gsed -i 's/\([A-Z]\)/\L\1/g' cur.txt
gsed -i 's/\([a-z]\)\s76/\1SS/g' cur.txt
gsed -i 's/\([a-z]\)\s\+\([a-z]\)/\1\2/g' cur.txt
gsed -i 's/\s\+/ /g' cur.txt
cut -f 4-7 -d ' ' cur.txt >$1'DATA.txt'

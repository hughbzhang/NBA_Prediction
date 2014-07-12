#This is my script for parsing the moneyline data from the baseball odds at oddshark.com

gsed -i '/DATE/d' line.txt
gsed -i 's/.*REG\s//g' line.txt
gsed -i 's/\s\+/ /g' line.txt
cut -f 1,3 -d ' ' <line.txt >temp.txt
mv temp.txt line.txt
gsed -i 's/W/1/g' line.txt
gsed -i 's/L/0/g' line.txt

head line.txt


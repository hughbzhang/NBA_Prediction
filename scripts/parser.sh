# Before you do this, you have to manually copy and paste the table from landofbasketball.com. I do not have a way to get an easily parsible version from the html
# This is my parser for the basketball data gathered from landofbasketball.com
cp $1 cur.txt

# echo ${1:5:4}
# Tricky here because 1 is the variable
# 5 is the start location (get rid of data)
# and 4 is the number of chars to take

sed -i 's/,//g' cur.txt # Remove all the commas
sed -i 's/[0-9]*OT//g' cur.txt # Remove all the overtime statements
sed -ie '/.\{20\}/!d' cur.txt # Remove all lines less than 20 chars long (dates)
sed -i 's/\([A-Z]\)/\L\1/g' cur.txt # Make everything lowercase
sed -i 's/\([a-z]\)\s76ers/\1SSers/g' cur.txt # Edge case with the 76ers
sed -i 's/\([a-z]\)\s\+\([a-z]\)/\1\2/g' cur.txt # Make team names one word
sed -i 's/\s\+/ /g' cur.txt # Remove extra spaces
sed -i 's/originally.*//g' cur.txt # Edge case for rescheduled games
sed -i 's/fulllist.*/playoffs/g' cur.txt # Edge case marking the beginning of playoffs

sed -i 's/atneworleans.*$/atneworleans/g' cur.txt # funkiness with new orleans

mv cur.txt "PARSED_DATA/"${1:5:4}".txt"

# At the end, you should get a list of WINNER WSCORE LOSER LSCORE LOCATION


#TODO maybe keep date and OT

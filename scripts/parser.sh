# Before you do this, you have to manually copy and paste the table from landofbasketball.com. I do not have a way to get an easily parsible version from the html
# This is my parser for the basketball data gathered from landofbasketball.com
cp $1 tmp.txt

# echo ${1:5:4}
# Tricky here because 1 is the variable
# 5 is the start location (get rid of data)
# and 4 is the number of chars to take

sed -i 's/,//g' tmp.txt # Remove all the commas
sed -i 's/[0-9]*OT//g' tmp.txt # Remove all the overtime statements
sed -i 's/\([A-Z]\)/\L\1/g' tmp.txt # Make everything lowercase
sed -i 's/\([a-z]\)\s76ers/\1SSers/g' tmp.txt # Edge case with the 76ers
sed -i 's/\s\+/ /g' tmp.txt # Remove extra spaces
sed -i 's/originally.*//g' tmp.txt # Edge case for rescheduled games

sed -i 's/played\sat.*$//g' tmp.txt # funkiness with non home locations

mv tmp.txt tmp1.txt

# At the end, you should get a list of WINNER WSCORE LOSER LSCORE LOCATION [DATE] [X]OT


#TODO maybe keep date and OT

#!/bin/bash
SAVEFILE=~/.config/ckb/ckb.conf

###################
# 
# Change the format of macro definitons from 4- to 5-parts.
# 
#

# Find a line, beginning with '$macro:',
# followed by exact 3 colon-separated strings.
# The may not be a 4th string (it would be the new format).
# If found, add a ':x' at the end.
# Otherwise just print it.
SEDLINE='/$macro:[^:]*:[^:]*:[^:]*$/s/$/:x/'

cp $SAVEFILE $SAVEFILE.orig

# some statistics
echo -n "how many \$macro:-Statements to deal with: "
grep  "$macro:" $SAVEFILE | wc -l

echo -n "That number entries do we find: "
sed -e $SEDLINE $SAVEFILE | grep  "$macro:" | wc -l

echo -n "how many lines without \$macro:-statements: "
grep  -v "$macro:" $SAVEFILE | wc -l

echo -n "That many entries are ignored: "
sed -e $SEDLINE $SAVEFILE | grep -v "$macro:" | wc -l

read -p "Is the information correct? [y|n] " RL
RL=${RL:-n}

if [[ ${RL} != "y" ]]; then
	echo you typed $RL.
	echo "Aborted."
	exit -1;
fi

echo you typed $RL.
sed -e $SEDLINE $SAVEFILE > $SAVEFILE.new
rm -f $SAVEFILE
mv $SAVEFILE.new $SAVEFILE
wc $SAVEFILE
grep :x$ $SAVEFILE

exit 0

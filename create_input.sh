#!/bin/bash
if [ $# -ne 2 ]
    then echo "Give file with available political parties and number of lines"
    exit 1
fi

exec 6<&1         # Save stdout to a fd, so it doesn't get lost when replaced.

exec >inputFile    # stdin replaced by file in var $1 (original stdin saved in fd 6)

parties=$1
numLines=$2

text=($(cat $1))   # array with contents of file
numParties=${#text[@]}      # total lines of file
chars='abcdefghijklmnopqrstuvwxyz'


while [ 1 ]
    do
    name=${chars:($RANDOM%9)+3:$((($RANDOM%9)+3))}
    sur=${chars:($RANDOM%9)+3:$((($RANDOM%9)+3))}
    num=$(($RANDOM % $numParties))

    echo -n $name; echo -n " $sur "; echo -n ${text[$num]}
    numLines=`expr $numLines - 1`

    if [ $numLines -eq 0 ]  # avoid newline at end of file
        then break
    fi
    echo;
done

exec 1<&6 6<&-  # restore stdout
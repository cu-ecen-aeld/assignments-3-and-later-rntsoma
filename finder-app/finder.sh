#!/bin/bash

FILES_DIR=$1
SEARCH_STR=$2

if [ -z "$FILES_DIR" ] || [ -z "$SEARCH_STR" ]
then
    echo "Missing FILES_DIR or SEARCH_STR"
    exit 1
fi

if [ ! -d "$FILES_DIR" ]
then
    echo "$FILES_DIR does not exist"
    exit 1
fi

# FILE_MATCH_COUNT=`ls $FILES_DIR -R | wc -w`
FILE_MATCH_COUNT=`ls $FILES_DIR | wc -w`
PATTERN_MATCH_COUNT=`grep -r $SEARCH_STR $FILES_DIR | wc -w`

echo "The number of files are $FILE_MATCH_COUNT and the number of matching lines are $PATTERN_MATCH_COUNT"

#!/bin/bash

FILE=$1
WRITE_STR=$2
FILE_PATH=$(dirname "$FILE")

if [ -z "$FILE" ] || [ -z "$WRITE_STR" ]
then
    echo "Missing FILE or WRITE_STR"
    exit 1
fi

if [ ! -d "$FILE_PATH" ]
then
    if ! mkdir -p $FILE_PATH
    then
        echo "Failed to create dirs"
        exit 1
    fi
fi

if [ -f "$FILE" ]
then
    if ! rm $FILE
    then
        echo "Failed to remove file"
        exit 1
    fi
fi

if ! touch $FILE
then
    echo "Failed to create file"
    exit 1
fi
echo $WRITE_STR >> $FILE

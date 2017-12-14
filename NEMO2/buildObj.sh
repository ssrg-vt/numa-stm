#!/bin/sh

FOLDERS="genome intruder kmeans labyrinth ssca2 vacation yada redblacktree bayes"
rm lib/*.o || true

for F in $FOLDERS
do
    cd $F
    rm *.o || true
    rm $F
    make -f MakefileObj
    rc=$?
    if [[ $rc != 0 ]] ; then
	echo ""
        echo "=================================== ERROR BUILDING $F ===================================="
	echo ""
        exit 1
    fi
    cd ..
done

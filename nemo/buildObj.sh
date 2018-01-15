#!/bin/sh

FOLDERS="genome intruder kmeans labyrinth ssca2 vacation yada redblacktree bayes"
rm lib/*.o || true

Obj="Obj"

for F in $FOLDERS
do
    cd $F
    rm *.o || true
    rm $F$Obj
    make -f MakefileObj
    rc=$?
    cd ..
done

#!/bin/bash

for ((i=1; i<=$1; i++))
do
    IDADE=$(((RANDOM % 100)+1))
    echo "./person -i Pessoa${i} $IDADE"

    ./person -i Pessoa${i} $IDADE

done

#$1 é um argumento passado ao script
#./pscript 10 -> 10 é o nº de pessoas
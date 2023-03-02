#!/bin/bash

for ((i=1; i<=$1; i++))
do
    IDADE=$(((RANDOM % 100)+1))
    echo "./pessoas -i Pessoa${i} $IDADE"

    ./pessoas -i Pessoas${i} $IDADE

done

#$1 é um argumento passado ao script
#./pscript 10 -> 10 é o nº depessoas
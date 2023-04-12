#!/bin/bash

# SCRIPT MEASURES THE TIME TO ADD X NUMBER OF NEW RANDOM PEOPLE TO FILE
# $1 -> argumento passado ao script (./pscript 10 -> vai inserir 10 pessoas)

time (
    for ((i=1; i<=$1; i++))
    do
        IDADE=$(((RANDOM % 100)+1))
        echo "./person -i Pessoa${i} $IDADE"

        ./person -i Pessoa${i} $IDADE

    done
)


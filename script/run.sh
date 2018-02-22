#!/bin/bash


./paraterbi_baseline < sents > /dev/null 2> ./tmp.err
./paraterbi_simd < sents > /dev/null 2>> tmp.err
./paraterbi < sents > /dev/null 2>> tmp.err
for i in {1..9}
do
	./paraterbi_baseline < sents > /dev/null 2>> tmp.err
	./paraterbi_simd < sents > /dev/null 2>> tmp.err
	./paraterbi < sents > /dev/null 2>> tmp.err
done

python3 ./sum.py

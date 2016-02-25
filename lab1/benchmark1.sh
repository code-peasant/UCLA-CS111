#!/usr/local/cs/bin/bash
echo>output1.txt
echo>stderr.txt
./simpsh --profile --rdonly foo --wronly output1.txt --wronly stderr.txt --command 0 1 2 uniq foo --wait

time cat < foo|uniq > output1.txt


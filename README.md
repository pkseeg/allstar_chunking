# ALLSTAR NMT Chunker

This repo contains code for accessing, chunking and pairing 
binary/source code function pairs within the 
[ALLSTAR JHU/APL dataset](https://allstar.jhuapl.edu/).

## Prereqs

Environment: angr, ctags, [allstar repo](https://github.com/JHUAPL/ALLSTAR).

python: requests, urllib, sys, subprocess, os, random, editdistance

## How to use

To grab, chunk, and split 20 random function pairs, simply run

```
python3 main.py
```

This can easily be edited, as well, to include more than 20 
random packages or specific packages under specific architectures.

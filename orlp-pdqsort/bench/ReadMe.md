# Sort Benchmark

## Depend library

`bench-graph.py` depend numpy and matplotlib.

### 1. Install numpy

```bash
python3 -m pip install -U numpy
```

### 2. Install matplotlib

Update pip:

```bash
python3 -m pip install -U pip
```

Install matplotlib:

```bash
python3 -m pip install -U pip setuptools
python3 -m pip install -U matplotlib
```

## Benchmark

This is the benchmark I used to test pdqsort and co. bench.cpp just spits out
the raw cycle counts, I use Python for post-processing, such as making a bar
graph of the median cycle count.

Example:

```bash
g++ -std=c++11 -O2 -m64 -march=native bench.cpp -o ./bench

mkdir ./profiles
./bench > profiles/pdqsort.txt

mkdir ./plots
python3 bench-graph.py "i5-4670k @ 3.4GHz"
```


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

#include "jstd/SortAlgorithms.h"

#include "SortBench/CPUWarmUp.h"
#include "SortBench/StopWatch.h"

extern void print_marcos();

void sort_benchmark()
{
    //
}

int main(int argc, char * argv[])
{
    printf("Sort Algorithms Benchmark.\n\n");
    print_marcos();

#if 1
    sort_benchmark();
#endif

    return 0;
}

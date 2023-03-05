
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "jstd/basic/vld.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>       // For time(time_t *);
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>        // For std::time()
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include "jstd/SortAlgorithms.h"

#include "SortBench/CPUWarmUp.h"
#include "SortBench/StopWatch.h"

extern void print_marcos();

#ifdef NDEBUG
static const size_t kTotalArrayCount = 1024 * 1024 * 4;
#else
static const size_t kTotalArrayCount = 1024 * 128;
#endif

struct Algorithm {
    enum {
        SelectSort,
        BubbleSort,
        InsertSort,
        BinaryInsertSort,
        QuickSort,
        TimSort,
        StdHeapSort,
        StdStableSort,
        StdSort,
        ORLP_PdQSort,
        Last
    };
};

struct ArrayKind {
    enum {
        ShuffledNoRepeat,
        AscendingNoRepeat,
        DescendingNoRepeat,
        Shuffled,
        Ascending,
        Descending,
        ShuffledHeavyRepeat,
        AscendingHeavyRepeat,
        DescendingHeavyRepeat,
        AllEqual,
        Last
    };
};

inline uint32_t rand16()
{
    return (uint32_t)rand();
}

inline uint32_t rand30()
{
#if (RAND_MAX == 0x7FFF)
    return (((uint32_t)rand() << 15) |
            ((uint32_t)rand() & 0x00007FFFu));
#else
    return  ((uint32_t)rand() << 16) |
            ((uint32_t)rand() & 0x0000FFFFu);
#endif
}

inline uint32_t rand32()
{
#if (RAND_MAX == 0x7FFF)
    return ((((uint32_t)rand()              ) << 30) |
            (((uint32_t)rand() & 0x00007FFFu) << 15) |
             ((uint32_t)rand() & 0x00007FFFu));
#else
    return  ((uint32_t)rand() << 16) |
            ((uint32_t)rand() & 0x0000FFFFu);
#endif
}

inline uint64_t rand64()
{
#if (RAND_MAX == 0x7FFF)
    return ((((uint64_t)rand()                ) << 45) |
            (((uint64_t)rand() & 0x00007FFFull) << 30) |
            (((uint64_t)rand() & 0x00007FFFull) << 15) |
             ((uint64_t)rand() & 0x00007FFFull));
#else
    return ((((uint64_t)rand()                ) << 48) |
            (((uint64_t)rand() & 0x0000FFFFull) << 32) |
            (((uint64_t)rand() & 0x0000FFFFull) << 16) |
             ((uint64_t)rand() & 0x0000FFFFull));
#endif
}

template <typename Iterator, typename Comparer>
void heap_sort(Iterator first, Iterator last, Comparer compare)
{
    std::make_heap(first, last, compare);
    std::sort_heap(first, last, compare);
}

template <typename Iterator>
void heap_sort(Iterator first, Iterator last)
{
    typedef typename std::iterator_traits<Iterator>::value_type T;
    std::make_heap(first, last, std::less<T>());
    std::sort_heap(first, last, std::less<T>());
}

template <size_t ArrayCount, size_t N>
inline size_t getArrayCount()
{
    size_t array_count;
    if (N <= 8)
        array_count = ((ArrayCount / 32) + (N - 1)) / N;
    else if (N <= 16)
        array_count = ((ArrayCount / 16) + (N - 1)) / N;
    else if (N <= 64)
        array_count = ((ArrayCount / 8) + (N - 1)) / N;
    else if (N <= 512)
        array_count = ((ArrayCount / 4) + (N - 1)) / N;
    else if (N <= 4096)
        array_count = ((ArrayCount / 2) + (N - 1)) / N;
    else if (N <= 32768)
        array_count = (ArrayCount + (N - 1)) / N;
    else
        array_count = (ArrayCount + (N - 1)) / N / 2;
    return (array_count == 0) ? 1 : array_count;
}

template <size_t AlgorithmId>
const char * getSortAlgorithmName()
{
    if (0)
        return "Unreachable";
    else if (AlgorithmId == Algorithm::SelectSort)
        return "SelectSort";
    else if (AlgorithmId == Algorithm::BubbleSort)
        return "BubbleSort";
    else if (AlgorithmId == Algorithm::InsertSort)
        return "InsertSort";
    else if (AlgorithmId == Algorithm::BinaryInsertSort)
        return "BinaryInsertSort";
    else if (AlgorithmId == Algorithm::QuickSort)
        return "QuickSort";
    else if (AlgorithmId == Algorithm::StdHeapSort)
        return "std::heap_sort";
    else if (AlgorithmId == Algorithm::StdStableSort)
        return "std::stable_sort";
    else if (AlgorithmId == Algorithm::StdSort)
        return "std::sort";
    else if (AlgorithmId == Algorithm::ORLP_PdQSort)
        return "orlp::pdqsort";
    else
        return "Unknown";
}

template <typename T>
bool verify_sort_answers(const std::unique_ptr<std::vector<T>[]> & test_array_list,
                         const std::unique_ptr<std::vector<T>[]> & standard_answers,
                         size_t array_count)
{
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & test_array = test_array_list[i];
        std::vector<T> & answer_array = standard_answers[i];
        if (test_array.size() != answer_array.size())
            return false;
        for (size_t n = 0; n < test_array.size(); n++) {
            if (test_array[n] != answer_array[n])
                return false;
        }
    }

    return true;
}

template <size_t AlgorithmId, typename T>
void run_sort_benchmark(const std::unique_ptr<std::vector<T>[]> & src_array_list,
                        const std::unique_ptr<std::vector<T>[]> & standard_answers,
                        size_t array_count, size_t total_items)
{
    test::StopWatch sw;
    std::unique_ptr<std::vector<T>[]> test_array_list(new std::vector<T>[array_count]());

    printf("Algorithm: %-23s ", getSortAlgorithmName<AlgorithmId>());

    // Copy test array from src_array_list
    sw.start();
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & src_test_array = src_array_list[i];
        std::vector<T> & test_array = test_array_list[i];
        test_array.insert(test_array.cbegin(), src_test_array.begin(), src_test_array.end());
    }
    sw.stop();

    //printf("Copy time: %6.3f ms, ", sw.getElapsedMillisec());

    // Sort all test array
    sw.start();
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & test_array = test_array_list[i];
        if (0) {
            // Do nothing!!
        } else if (AlgorithmId == Algorithm::BubbleSort) {
            jstd::BubbleSort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::SelectSort) {
            jstd::SelectSort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::InsertSort) {
            jstd::InsertSort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::BinaryInsertSort) {
            jstd::BinaryInsertSort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::StdHeapSort) {
            heap_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::StdStableSort) {
            std::stable_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::StdSort) {
            std::sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::ORLP_PdQSort) {
            orlp::pdqsort(test_array.begin(), test_array.end());
        } else {
            // Unknown algorithm
        }
    }
    sw.stop();

    printf("Sort time: %8.3f ms, ", sw.getElapsedMillisec());
    if (total_items != 0)
        printf("Per item time: %8.3f ns", sw.getElapsedNanosec() / total_items);
    else
        printf("Per item time: N/A ns");

    if (0) {
        bool verifyRsult = verify_sort_answers(test_array_list, standard_answers, array_count);
        printf("verify = %s\n", verifyRsult ? "Passed" : "Failed");
    } else {
        printf("\n");
    }
}

template <typename T>
void generate_standard_answers(std::unique_ptr<std::vector<T>[]> & standard_answers,
                               const std::unique_ptr<std::vector<T>[]> & src_array_list,
                               size_t array_count)
{
    // Copy test array from src_array_list
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & src_test_array = src_array_list[i];
        std::vector<T> & test_array = standard_answers[i];
        test_array.insert(test_array.cbegin(), src_test_array.begin(), src_test_array.end());
    }

    // Generate all standard sort answers
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & test_array = standard_answers[i];
        std::sort(test_array.begin(), test_array.end());
    }
}

template <typename T, size_t ArrayType, size_t MinLen, size_t MaxLen>
void sort_benchmark_impl()
{
    static const size_t minLen = (MinLen < MaxLen) ? MinLen : MaxLen;
    static const size_t maxLen = (MinLen > MaxLen) ? MinLen : MaxLen;
    static const size_t rndRange = (maxLen + 1 - minLen);

    size_t array_count = getArrayCount<kTotalArrayCount, (MinLen > MaxLen) ? MinLen : MaxLen>();
    std::unique_ptr<std::vector<T>[]> test_array_list(new std::vector<T>[array_count]());

    printf("sort_benchmark<%u, %u, %u>, rnd_range = %u, array_count = %u\n\n",
           (uint32_t)ArrayType, (uint32_t)minLen, (uint32_t)maxLen,
           (uint32_t)rndRange, (uint32_t)array_count);
    
    size_t total_items = 0;
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & test_array = test_array_list[i];
        size_t length = minLen + static_cast<size_t>(rand30()) % rndRange;
        total_items += length;
        for (size_t n = 0; n < length; n++) {
            T rndNum = static_cast<T>(rand30());
            test_array.push_back(rndNum);
        }
    }

    std::unique_ptr<std::vector<T>[]> standard_answers(new std::vector<T>[array_count]());
    generate_standard_answers<T>(standard_answers, test_array_list, array_count);

#ifdef NDEBUG
    if (maxLen <= 512) {
        run_sort_benchmark<Algorithm::SelectSort, T>(test_array_list, standard_answers, array_count, total_items);
        run_sort_benchmark<Algorithm::BubbleSort, T>(test_array_list, standard_answers, array_count, total_items);
    }
    if (maxLen <= 5120) {
        run_sort_benchmark<Algorithm::InsertSort, T>(test_array_list, standard_answers, array_count, total_items);
        run_sort_benchmark<Algorithm::BinaryInsertSort, T>(test_array_list, standard_answers, array_count, total_items);
    }
#else
    if (maxLen <= 256) {
        run_sort_benchmark<Algorithm::SelectSort, T>(test_array_list, standard_answers, array_count, total_items);
        run_sort_benchmark<Algorithm::BubbleSort, T>(test_array_list, standard_answers, array_count, total_items);
    }
    if (maxLen <= 1024) {
        run_sort_benchmark<Algorithm::InsertSort, T>(test_array_list, standard_answers, array_count, total_items);
        run_sort_benchmark<Algorithm::BinaryInsertSort, T>(test_array_list, standard_answers, array_count, total_items);
    }
#endif // NDEBUG

    run_sort_benchmark<Algorithm::StdHeapSort, T>(test_array_list, standard_answers, array_count, total_items);
    run_sort_benchmark<Algorithm::StdStableSort, T>(test_array_list, standard_answers, array_count, total_items);
    run_sort_benchmark<Algorithm::StdSort, T>(test_array_list, standard_answers, array_count, total_items);
    run_sort_benchmark<Algorithm::ORLP_PdQSort, T>(test_array_list, standard_answers, array_count, total_items);

    printf("\n");
}

template <typename T, size_t ArrayType, size_t N>
void sort_benchmark_impl()
{
    sort_benchmark_impl<T, ArrayType, N, N>();
}

template <typename T, size_t ArrayType>
void sort_benchmark()
{
    // Randomize short array threshold test
    sort_benchmark_impl<T, ArrayType, 1, 8>();
    sort_benchmark_impl<T, ArrayType, 1, 16>();
    sort_benchmark_impl<T, ArrayType, 1, 32>();
    sort_benchmark_impl<T, ArrayType, 1, 64>();
    sort_benchmark_impl<T, ArrayType, 1, 96>();
    sort_benchmark_impl<T, ArrayType, 1, 128>();

    // Randomize short array range test
    sort_benchmark_impl<T, ArrayType, 4, 8>();
    sort_benchmark_impl<T, ArrayType, 9, 16>();
    sort_benchmark_impl<T, ArrayType, 24, 32>();
    sort_benchmark_impl<T, ArrayType, 48, 64>();
    sort_benchmark_impl<T, ArrayType, 65, 96>();
    sort_benchmark_impl<T, ArrayType, 97, 128>();
    sort_benchmark_impl<T, ArrayType, 192, 256>();

    // Randomize short array test
    sort_benchmark_impl<T, ArrayType, 40, 50>();
    sort_benchmark_impl<T, ArrayType, 90, 100>();
    sort_benchmark_impl<T, ArrayType, 280, 300>();
    sort_benchmark_impl<T, ArrayType, 450, 500>();

    // Randomize long array test
    sort_benchmark_impl<T, ArrayType, 900, 1000>();
    sort_benchmark_impl<T, ArrayType, 4500, 5000>();
    sort_benchmark_impl<T, ArrayType, 9500, 10000>();
    sort_benchmark_impl<T, ArrayType, 49000, 50000>();
    sort_benchmark_impl<T, ArrayType, 99000, 100000>();
    sort_benchmark_impl<T, ArrayType, 499000, 500000>();
    sort_benchmark_impl<T, ArrayType, 999000, 1000000>();
}

int main(int argc, char * argv[])
{
    print_marcos();

    printf("Sort Algorithms Benchmark.\n\n");

    //std::srand((unsigned int)std::time(0));
    std::srand((unsigned int)20230304L);

    test::CPU::WarmUp warmUper(1000);

    if (1)
    {
        //sort_benchmark<uint32_t, ArrayKind::ShuffledNoRepeat>();
        sort_benchmark<uint32_t, ArrayKind::Shuffled>();
        //sort_benchmark<uint32_t, ArrayKind::ShuffledHeavyRepeat>();

        //sort_benchmark<uint32_t, ArrayKind::AllEqual>();
    }

    printf("\n");
    return 0;
}

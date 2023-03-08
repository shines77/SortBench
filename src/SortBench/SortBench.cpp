
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
#include <iostream>
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

struct Algorithm {
    enum {
        jstdBubbleSort,
        jstdSelectSort,
        jstdInsertSort,
        jstdBinaryInsertSort,
        jstdBinaryInsertSort_v1,
        jstdBinaryInsertSort_v2,
        jstdBucketSort,
        jstdQuickSort,
        TimSort,
        stdHeapSort,
        stdStableSort,
        stdSort,
        orlp_pdqsort,
        ska_sort,
        Last
    };
};

template <size_t AlgorithmId>
const char * getSortAlgorithmName()
{
    if (0)
        return "Unreachable";
    else if (AlgorithmId == Algorithm::jstdBubbleSort)
        return "jstd::bubble_sort";
    else if (AlgorithmId == Algorithm::jstdSelectSort)
        return "jstd::select_sort";
    else if (AlgorithmId == Algorithm::jstdInsertSort)
        return "jstd::insert_sort";
    else if (AlgorithmId == Algorithm::jstdBinaryInsertSort)
        return "jstd::binary_insert_sort";
    else if (AlgorithmId == Algorithm::jstdBinaryInsertSort_v1)
        return "jstd::binary_insert_sort_v1";
    else if (AlgorithmId == Algorithm::jstdBinaryInsertSort_v2)
        return "jstd::binary_insert_sort_v2";
    else if (AlgorithmId == Algorithm::jstdBucketSort)
        return "jstd::bucket_sort";
    else if (AlgorithmId == Algorithm::jstdQuickSort)
        return "jstd::quick_sort";
    else if (AlgorithmId == Algorithm::stdHeapSort)
        return "std::heap_sort";
    else if (AlgorithmId == Algorithm::stdStableSort)
        return "std::stable_sort";
    else if (AlgorithmId == Algorithm::stdSort)
        return "std::sort";
    else if (AlgorithmId == Algorithm::orlp_pdqsort)
        return "orlp::pdqsort";
    else if (AlgorithmId == Algorithm::ska_sort)
        return "ska_sort";
    else
        return "Unknown Algorithm";
}

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

template <typename T>
void print_array(const std::string & name, const std::vector<T> & _array)
{
    static const size_t kItemsOneLine = 16;
    std::cout << name.c_str()
              << "[" << (uint32_t)_array.size() << "] = {" << std::endl;
    size_t count = 0;
    for (auto & val : _array) {
        if ((count % kItemsOneLine) == 0)
            std::cout << "  ";
        std::cout << val;
        if (count == (_array.size() - 1)) {
            std::cout << std::endl;
        } else {
            if ((count % kItemsOneLine) != (kItemsOneLine - 1))
                std::cout << ", ";
            else
                std::cout << "," << std::endl;
        }
        count++;
    }
    std::cout << "};" << std::endl;
    std::cout << std::endl;
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

template <typename T>
void generate_standard_answer(const std::vector<T> & src_array, std::vector<T> & answers)
{
    // Copy to answer from src_array
    answers.insert(answers.cbegin(), src_array.begin(), src_array.end());

    // Sorting the answer
    std::sort(answers.begin(), answers.end());
}

template <typename T>
void generate_standard_answers(std::unique_ptr<std::vector<T>[]> & standard_answers,
                               const std::unique_ptr<std::vector<T>[]> & src_array_list,
                               size_t array_count)
{
    // Copy to standard_answers from src_array_list, and then sorting.
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & src_array    = src_array_list[i];
        std::vector<T> & answer_array = standard_answers[i];
        generate_standard_answer(src_array, answer_array);
    }
}

template <typename T>
bool verify_sort_answer(const std::vector<T> & src_array, const std::vector<T> & answer)
{
    if (src_array.size() != answer.size())
        return false;
    for (size_t n = 0; n < src_array.size(); n++) {
        if (src_array[n] != answer[n])
            return false;
    }
    return true;
}

template <typename T>
bool verify_sort_answers(const std::unique_ptr<std::vector<T>[]> & test_array_list,
                         const std::unique_ptr<std::vector<T>[]> & standard_answers,
                         size_t array_count)
{
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & test_array = test_array_list[i];
        std::vector<T> & answer_array = standard_answers[i];
        if (!verify_sort_answer(test_array, answer_array))
            return false;
    }

    return true;
}

template <size_t AlgorithmId, typename T>
void sort_algo_bench(const std::unique_ptr<std::vector<T>[]> & src_array_list,
                     const std::unique_ptr<std::vector<T>[]> & standard_answers,
                     size_t array_count, size_t total_items)
{
    test::StopWatch sw;
    std::unique_ptr<std::vector<T>[]> test_array_list(new std::vector<T>[array_count]());

    printf(" %-28s ", getSortAlgorithmName<AlgorithmId>());

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
        } else if (AlgorithmId == Algorithm::jstdBubbleSort) {
            jstd::bubble_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::jstdSelectSort) {
            jstd::select_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::jstdInsertSort) {
            jstd::insert_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::jstdBinaryInsertSort) {
            jstd::binary_insert_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::jstdBinaryInsertSort_v1) {
            jstd::binary_insert_sort_v1(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::jstdBinaryInsertSort_v2) {
            jstd::binary_insert_sort_v2(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::jstdBucketSort) {
            jstd::bucket_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::stdHeapSort) {
            heap_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::stdStableSort) {
            std::stable_sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::stdSort) {
            std::sort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::orlp_pdqsort) {
            orlp::pdqsort(test_array.begin(), test_array.end());
        } else if (AlgorithmId == Algorithm::ska_sort) {
            ska_sort(test_array.begin(), test_array.end());
        } else {
            // Unknown algorithm
        }
    }
    sw.stop();

    printf("Sort time: %8.3f ms", sw.getElapsedMillisec());
    if (total_items != 0)
        printf(", Per item time: %8.3f ns", sw.getElapsedNanosec() / total_items);
    else
        printf(", Per item time: N/A ns");

    if (1) {
        bool correctness = verify_sort_answers(test_array_list, standard_answers, array_count);
        printf(", verify = %s", correctness ? "Pass" : "Failed");
    }
    printf("\n");
}

template <typename T, size_t ArrayType, size_t MinLen, size_t MaxLen>
void sort_benchmark_impl()
{
    static const size_t minLen = (MinLen < MaxLen) ? MinLen : MaxLen;
    static const size_t maxLen = (MinLen > MaxLen) ? MinLen : MaxLen;
    static const size_t lenRange = (maxLen + 1 - minLen);

    size_t array_count = getArrayCount<kTotalArrayCount, (MinLen > MaxLen) ? MinLen : MaxLen>();
    std::unique_ptr<std::vector<T>[]> test_array_list(new std::vector<T>[array_count]());

    printf(" sort_benchmark<%u, %u, %u>, len_range = %u, array_count = %u\n\n",
           (uint32_t)ArrayType, (uint32_t)minLen, (uint32_t)maxLen,
           (uint32_t)lenRange, (uint32_t)array_count);
    
    size_t total_items = 0;
    for (size_t i = 0; i < array_count; i++) {
        std::vector<T> & test_array = test_array_list[i];
        size_t length;
        if (lenRange != 0)
            length = minLen + static_cast<size_t>(rand30()) % lenRange;
        else
            length = minLen + static_cast<size_t>(rand30());
        total_items += length;
        if (length <= (8 * 65536)) {
            for (size_t n = 0; n < length; n++) {
                T rndNum = static_cast<T>(rand16());
                test_array.push_back(rndNum);
            }
        } else {
            for (size_t n = 0; n < length; n++) {
                T rndNum = static_cast<T>(rand30());
                test_array.push_back(rndNum);
            }
        }
    }

    std::unique_ptr<std::vector<T>[]> standard_answers(new std::vector<T>[array_count]());
    generate_standard_answers<T>(standard_answers, test_array_list, array_count);

#define TEST_PARAMS(test_array_list) \
    test_array_list, standard_answers, array_count, total_items

#ifdef NDEBUG
    if (maxLen <= 512) {
        sort_algo_bench<Algorithm::jstdBubbleSort, T>(TEST_PARAMS(test_array_list));
        sort_algo_bench<Algorithm::jstdSelectSort, T>(TEST_PARAMS(test_array_list));
    }
    if (maxLen <= 5120) {
        sort_algo_bench<Algorithm::jstdInsertSort, T>(TEST_PARAMS(test_array_list));
    }
    if (maxLen <= 10240) {
        sort_algo_bench<Algorithm::jstdBinaryInsertSort_v1, T>(TEST_PARAMS(test_array_list));
        sort_algo_bench<Algorithm::jstdBinaryInsertSort_v2, T>(TEST_PARAMS(test_array_list));
    }
#else
    if (maxLen <= 128) {
        sort_algo_bench<Algorithm::jstdBubbleSort, T>(TEST_PARAMS(test_array_list));
        sort_algo_bench<Algorithm::jstdSelectSort, T>(TEST_PARAMS(test_array_list));
    }
    if (maxLen <= 256) {
        sort_algo_bench<Algorithm::jstdInsertSort, T>(TEST_PARAMS(test_array_list));
    }
    if (maxLen <= 512) {
        sort_algo_bench<Algorithm::jstdBinaryInsertSort_v1, T>(TEST_PARAMS(test_array_list));
        sort_algo_bench<Algorithm::jstdBinaryInsertSort_v2, T>(TEST_PARAMS(test_array_list));
    }
#endif // NDEBUG

    sort_algo_bench<Algorithm::stdHeapSort,    T>(TEST_PARAMS(test_array_list));
    sort_algo_bench<Algorithm::stdStableSort,  T>(TEST_PARAMS(test_array_list));
    sort_algo_bench<Algorithm::stdSort,        T>(TEST_PARAMS(test_array_list));
    sort_algo_bench<Algorithm::orlp_pdqsort,   T>(TEST_PARAMS(test_array_list));
    sort_algo_bench<Algorithm::ska_sort,       T>(TEST_PARAMS(test_array_list));
    sort_algo_bench<Algorithm::jstdBucketSort, T>(TEST_PARAMS(test_array_list));

    printf("\n");
#undef TEST_PARAMS
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
#ifdef NDEBUG
    sort_benchmark_impl<T, ArrayType, 499000, 500000>();
    sort_benchmark_impl<T, ArrayType, 999000, 1000000>();
#endif
}

template <typename T, size_t MinLen, size_t MaxLen>
bool bucket_sort_test_impl(T minVal, T maxVal)
{
    static const size_t kMinLen = MinLen;
    static const size_t kMaxLen = MaxLen;
    static const size_t kLenRange = kMaxLen - kMinLen + 1;
    std::vector<T> test_array;

    size_t valRange = maxVal - minVal + 1;
    size_t length = kMinLen + static_cast<size_t>(rand30()) % kLenRange;
    if (valRange != 0) {
        for (size_t n = 0; n < length; n++) {
            T rndNum = minVal + static_cast<T>(rand32()) % valRange;
            test_array.push_back(rndNum);
        }
    } else {
        for (size_t n = 0; n < length; n++) {
            T rndNum = minVal + static_cast<T>(rand32());
            test_array.push_back(rndNum);
        }
    }

    if (kMaxLen < 1024) {
        print_array("test_array", test_array);
    }

    std::vector<T> answer;
    generate_standard_answer<T>(test_array, answer);

    jstd::bucket_sort(test_array.begin(), test_array.end());

    if (kMaxLen < 1024) {
        print_array("test_array", test_array);
        print_array("answer_array", answer);
    }

    bool correctness = verify_sort_answer(test_array, answer);
    return correctness;
}

void bucket_sort_debug_test()
{
    std::srand((unsigned int)std::time(0));

    bool correctness = true;
    if (0) {
        printf("bucket_sort_test_impl<uint32_t, 20000, 65536>(0, 65535);\n");
        correctness = bucket_sort_test_impl<uint32_t, 20000, 65536>(0, 65535);
        printf("correctness = %s\n\n", (correctness ? "Pass" : "Failed"));
    }

    if (1) {
        printf("bucket_sort_test_impl<uint32_t, 20000, 65536>(0, 65535 * 4 - 1);\n");
        correctness = bucket_sort_test_impl<uint32_t, 20000, 65536>(0, 65535 * 4 - 1);
        printf("correctness = %s\n\n", (correctness ? "Pass" : "Failed"));
    }
}

void bucket_sort_test()
{
    bool correctness = true;
    if (1) {
        printf("bucket_sort_test_impl<uint32_t, 256, 512>(0, 65535);\n");
        correctness = bucket_sort_test_impl<uint32_t, 256, 320>(0, 65535);
        printf("correctness = %s\n\n", (correctness ? "Pass" : "Failed"));
    }

    if (0) {
        printf("bucket_sort_test_impl<uint32_t, 256, 512>(0, 65535);\n");
        correctness = bucket_sort_test_impl<uint32_t, 256, 512>(0, 65535);
        printf("correctness = %s\n\n", (correctness ? "Pass" : "Failed"));
    }
}

int main(int argc, char * argv[])
{
    print_marcos();

    printf("Sort Algorithms Benchmark.\n\n");

    //std::srand((unsigned int)std::time(0));
    std::srand((unsigned int)20230304L);

    test::CPU::WarmUp warm_up(1000);

#ifdef _DEBUG
    bucket_sort_debug_test();
#endif

#ifndef _DEBUG
    if (1)
    {
        bucket_sort_test();
    }

    if (1)
    {
        //sort_benchmark<uint32_t, ArrayKind::ShuffledNoRepeat>();
        sort_benchmark<uint32_t, ArrayKind::Shuffled>();
        //sort_benchmark<uint32_t, ArrayKind::ShuffledHeavyRepeat>();

        //sort_benchmark<uint32_t, ArrayKind::AllEqual>();
    }
#endif

    printf("\n");
    return 0;
}


#ifndef JSTD_BUCKET_SORT_H
#define JSTD_BUCKET_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"

#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>       // For std::unique_ptr<T>
#include <cstring>      // For std::memset()
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace detail {

template <typename T, typename CountType>
struct SortBucket {
    typedef T         value_type;
    typedef CountType count_type;

    count_type              count;
    std::vector<value_type> items;

    SortBucket() noexcept : count(0) {}

    explicit SortBucket(count_type capacity) {
        items.reserve(capacity);
    }

    ~SortBucket() {}
};

template <typename T>
inline size_t ilog2(T n)
{
    size_t exponent = 0;
    while (n > 1) {
        exponent++;
        n >>= 1;
    }
    return exponent;
}

template <typename T, typename DiffType, typename Iterator, typename Comparer>
inline void counting_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                 const T & minVal, const DiffType & distance) {
    typedef Iterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    assert(length > 0);

    static size_t print_cnt = 0;

    if (print_cnt < 16) {
        printf("counting_bucket_sort(), length = %5u, minVal = %5u, distance = %5u\n",
               (uint32_t)length, (uint32_t)minVal, (uint32_t)distance);
        print_cnt++;
    }

    assert(distance > 0);
    if (distance < diff_type(65536)) {
        uint16_t counts[65536];
        std::memset(&counts[0], 0, sizeof(uint16_t) * (distance + 1));

        iterator iter;
        for (iter = first; iter < last; ++iter) {
            value_type idx = *iter - minVal;
            counts[idx] = counts[idx] + 1;
        }

        if (print_cnt < 16) {
            printf("counts[] = {\n");
            printf("    ");
            for (diff_type i = 0; i <= 16; ++i) {
                printf("%u, ", counts[i]);
            }
            printf("\n}\n");
        }

        iter = first;
        for (diff_type i = 0; i <= distance; ++i) {
            uint16_t count = counts[i];
            if (count != 0) {
                value_type val = minVal + static_cast<value_type>(i);
                for (uint16_t n = 0; n < count; ++n) {
                    assert(iter != last);
                    *iter = val;
                    ++iter;
                }
            }
        }
        assert(iter == last);
    } else {
        std::unique_ptr<uint32_t[]> counts(new uint32_t[distance + 1]());
        std::memset(&counts[0], 0, sizeof(uint32_t) * (distance + 1));

        iterator iter;
        for (iter = first; iter < last; ++iter) {
            value_type idx = *iter - minVal;
            counts[idx] = counts[idx] + 1;
        }

        iter = first;
        for (diff_type i = 0; i <= distance; ++i) {
            uint32_t count = counts[i];
            if (count != 0) {
                value_type val = minVal + static_cast<value_type>(i);
                for (uint32_t n = 0; n < count; ++n) {
                    assert(iter != last);
                    *iter = val;
                    ++iter;
                }
            }
        }
        assert(iter == last);
    }
}

template <typename T, typename DiffType, typename Iterator, typename Comparer>
inline void histogram_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                  const T & minVal, const DiffType & distance,
                                  size_t bucketSize) {
    typedef Iterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    //typedef typename std::iterator_traits<iterator>::difference_type diff_type;
    typedef SortBucket<value_type, size_t> bucket_type;

    size_t bucketCount = (distance + (bucketSize - 1)) / bucketSize;

    std::unique_ptr<bucket_type[]> buckets(new bucket_type[bucketCount]);
    for (size_t i = 0; i < bucketCount; i++) {
        bucket_type & bucket = buckets[i];
        bucket.count = 0;
    }
}

template <typename RandomAccessIterator, typename Comparer>
inline void bucket_sort_impl(RandomAccessIterator first, RandomAccessIterator last,
                             Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (likely(length <= 256)) {
        std::sort(first, last, compare);
    } else {
        assert(first != last);
        value_type minVal = *first;
        value_type maxVal = *first;
        for (iterator iter = std::next(first); iter < last; ++iter) {
            if (*iter < minVal) minVal = *iter;
            if (*iter > maxVal) maxVal = *iter;
        }

        diff_type distance = static_cast<diff_type>(maxVal - minVal);
        if (likely(distance != 0)) {
            if (distance < diff_type(65536 * 4)) {
                //printf("counting_bucket_sort(), minVal = %u, distance = %u\n",
                //       (uint32_t)minVal, (uint32_t)distance);
                counting_bucket_sort<value_type>(first, last, compare, minVal, distance);
            } else if (length < diff_type(65536 * 4)) {
                //
                printf("bucket_sort_impl() unknown branch1\n");
            } else {
                static const size_t kMaxBucketSize = 65536;
                static const size_t kMaxBucketShift = 16;
                static const size_t kMaxBucketCount = 65536;
                static const size_t kBucketSizeThreshold = 8;

                size_t minBucketCount = (distance + (kMaxBucketSize - 1)) / kMaxBucketSize;
                size_t exp = ilog2(minBucketCount);
                size_t bucketShift = kMaxBucketShift - exp;
                size_t bucketSize = kMaxBucketSize >> bucketShift;
                assert(bucketSize > 0);
                if (bucketSize <= kBucketSizeThreshold ||
                    (size_t)length <= (kBucketSizeThreshold * kMaxBucketSize)) {
                    //
                    printf("bucket_sort_impl() unknown branch2\n");
                } else {
                    printf("histogram_bucket_sort()\n");
                    histogram_bucket_sort<value_type>(first, last, compare, minVal, distance, bucketSize);
                }
            }
        }
    }
}

template <typename BiDirectionalIterator, typename Comparer>
inline void bucket_sort_impl(BiDirectionalIterator first, BiDirectionalIterator last,
                             Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIterator iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_base_of<iterator_category, std::bidirectional_iterator_tag>::value,
                  "detail::bucket_sort() is not supported std::bidirectional_iterator.");
    if (likely(first != last)) {
        //
    }
}

template <typename ForwardIterator, typename Comparer>
inline void bucket_sort_impl(ForwardIterator first, ForwardIterator last,
                             Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIterator iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_base_of<iterator_category, std::forward_iterator_tag>::value,
                  "detail::bucket_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iterator, typename Comparer>
inline void bucket_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::bucket_sort_impl(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void bucket_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    bucket_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUCKET_SORT_H

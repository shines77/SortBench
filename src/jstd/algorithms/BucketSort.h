
#ifndef JSTD_BUCKET_SORT_H
#define JSTD_BUCKET_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/support/BitUtils.h"

#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>       // For std::unique_ptr<T>
#include <cstring>      // For std::memset()
#include <bitset>       // For std::bitset<N>
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

template <typename T, typename ValueType, typename DiffType, typename Iterator, typename Comparer>
inline void dense_counting_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                       const ValueType & minVal, const DiffType & distance) {
    typedef T count_type;
    typedef Iterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    assert(length > 0);

    assert(distance > 0);
    if (likely(distance < diff_type(65536))) {
        count_type counts[65536];
        std::memset(&counts[0], 0, sizeof(count_type) * (distance + 1));

        iterator iter;
        for (iter = first; iter < last; ++iter) {
            value_type idx = *iter - minVal;
            counts[idx] += 1;
        }

#if 0
        static size_t debug_print_cnt = 0;

        if (debug_print_cnt < 16) {
            printf("counts[] = {\n");
            printf("    ");
            for (diff_type i = 0; i <= 32; ++i) {
                printf("%u, ", counts[i]);
            }
            printf("\n}\n");
            debug_print_cnt++;
        }
#endif

        iter = first;
        for (diff_type i = 0; i <= distance; ++i) {
            count_type count = counts[i];
            if (count != 0) {
                value_type val = minVal + static_cast<value_type>(i);
                for (count_type n = 0; n < count; ++n) {
                    assert(iter != last);
                    *iter = val;
                    ++iter;
                }
            }
        }
        assert(iter == last);
    } else {
        std::unique_ptr<count_type[]> counts(new count_type[distance + 1]());
        //std::memset(&counts[0], 0, sizeof(count_type) * (distance + 1));

        iterator iter;
        for (iter = first; iter < last; ++iter) {
            value_type idx = *iter - minVal;
            counts[idx] += 1;
        }

        iter = first;
        for (diff_type i = 0; i <= distance; ++i) {
            count_type count = counts[i];
            if (count != 0) {
                value_type val = minVal + static_cast<value_type>(i);
                for (count_type n = 0; n < count; ++n) {
                    assert(iter != last);
                    *iter = val;
                    ++iter;
                }
            }
        }
        assert(iter == last);
    }
}

template <typename T, typename ValueType, typename DiffType, typename Iterator, typename Comparer>
inline void sparse_counting_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                        const ValueType & minVal, const DiffType & distance) {
    typedef T count_type;
    typedef Iterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    static const size_t kFixedDistance = 65536;
    static const size_t kBitsPerWord = sizeof(size_t) * 8;
    static const size_t kShiftPerWord = 6;

    diff_type length = last - first;
    assert(length > 0);

    assert(distance > 0);
    if (likely(distance < diff_type(kFixedDistance))) {
        static const size_t kFixedBitsWordLen = kFixedDistance / sizeof(size_t);
        size_t count_bits[kFixedBitsWordLen];
        count_type counts[kFixedDistance];
        size_t bitsAlignedBytes = ((distance + 1) + sizeof(size_t) - 1) / sizeof(size_t);
        size_t maxBitsWordLen  = (bitsAlignedBytes + sizeof(size_t) - 1) / sizeof(size_t);
        size_t maxCountWordLen = ((distance + 1) * sizeof(count_type) + sizeof(size_t) - 1) / sizeof(size_t);
        assert(maxBitsWordLen <= kFixedBitsWordLen);
        std::memset(&count_bits[0], 0, sizeof(size_t) * maxBitsWordLen);
        std::memset(&counts[0],     0, sizeof(size_t) * maxCountWordLen);

        iterator iter;
        for (iter = first; iter < last; ++iter) {
            value_type idx = *iter - minVal;
            count_type old_count = counts[idx];
            counts[idx] = old_count + 1;
            if (old_count == 0) {
                size_t pos   = idx / kBitsPerWord;
                size_t shift = idx % kBitsPerWord;
                size_t mask  = size_t(1) << shift;
                assert(pos < kFixedBitsWordLen);
                count_bits[pos] |= mask;
            }
        }

        iter = first;
        for (diff_type i = 0; i < (diff_type)maxBitsWordLen; ++i) {
            size_t mask = count_bits[i];
            while (mask != 0) {
                size_t bit_pos = BitUtils::bsf(mask);
                mask ^= BitUtils::ls1b(mask);
                size_t dist = i * kBitsPerWord + bit_pos;
                value_type val = minVal + static_cast<value_type>(dist);
                count_type count = counts[dist];
                assert(count != 0);
                for (count_type n = 0; n < count; ++n) {
                    assert(iter != last);
                    *iter = val;
                    ++iter;
                }
            }
        }
        assert(iter == last);
    } else {
        size_t bitsAlignedBytes = ((distance + 1) + sizeof(size_t) - 1) / sizeof(size_t);
        size_t maxBitsWordLen = (bitsAlignedBytes + sizeof(size_t) - 1) / sizeof(size_t);
        std::unique_ptr<size_t[]> count_bits(new size_t[maxBitsWordLen]());
        std::unique_ptr<count_type[]> counts(new count_type[distance + 1]());
        //std::memset(&counts[0], 0, sizeof(uint32_t) * (distance + 1));

        iterator iter;
        for (iter = first; iter < last; ++iter) {
            value_type idx = *iter - minVal;
            count_type old_count = counts[idx];
            counts[idx] = old_count + 1;
            if (old_count == 0) {
                size_t pos   = idx / kBitsPerWord;
                size_t shift = idx % kBitsPerWord;
                size_t mask  = size_t(1) << shift;
                size_t * bits = count_bits.get();
                assert(pos < maxBitsWordLen);
                bits[pos] |= mask;
            }
        }

        iter = first;
        for (diff_type i = 0; i < (diff_type)maxBitsWordLen; ++i) {
            size_t mask = count_bits.get()[i];
            while (mask != 0) {
                size_t bit_pos = BitUtils::bsf(mask);
                mask ^= BitUtils::ls1b(mask);
                size_t dist = i * kBitsPerWord + bit_pos;
                count_type count = counts[dist];
                value_type val = minVal + static_cast<value_type>(dist);
                assert(count != 0);
                for (count_type n = 0; n < count; ++n) {
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
        if (likely(length <= 65536)) {
            // Short array [0, 65536]
            if (likely(distance < diff_type(8 * 65536))) {
                if (likely(distance > (length * 5 / 4)))
                    sparse_counting_bucket_sort<uint16_t>(first, last, compare, minVal, distance);
                else if (likely(distance != 0))
                    dense_counting_bucket_sort<uint16_t>(first, last, compare, minVal, distance);
            } else if (length < diff_type(8 * 65536)) {
                //
                //printf("bucket_sort_impl() unknown branch1\n");
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
                    //printf("bucket_sort_impl() unknown branch2\n");
                } else {
                    //printf("histogram_bucket_sort()\n");
                    histogram_bucket_sort<value_type>(first, last, compare, minVal, distance, bucketSize);
                }
            }
        } else {
            // Long array (65536, UInt32Max or UInt64Max]
            if (likely(distance < diff_type(8 * 65536))) {
                if (likely(distance > (length * 5 / 4)))
                    sparse_counting_bucket_sort<uint32_t>(first, last, compare, minVal, distance);
                else if (likely(distance != 0))
                    dense_counting_bucket_sort<uint32_t>(first, last, compare, minVal, distance);
            } else if (length < diff_type(8 * 65536)) {
                //
                //printf("bucket_sort_impl() unknown branch1\n");
            } else {
                //
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

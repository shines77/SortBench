
#ifndef JSTD_BUCKET_SORT_H
#define JSTD_BUCKET_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/support/BitUtils.h"
#include "jstd/support/Power2.h"

#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <limits>       // For std::min(), std::max()
#include <memory>       // For std::unique_ptr<T>
#include <cstring>      // For std::memset()
#include <bitset>       // For std::bitset<N>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace bucket_detail {

// The threshold of built-in insertion sort
static const size_t kInsertSortThreshold = 128;

// The threshold of std::sort()
static const size_t kStdSortThreshold = 128;

template <typename T, typename CountType>
struct SortBucket {
    typedef T                                            value_type;
    typedef typename std::make_unsigned<CountType>::type count_type;
    typedef count_type                                   size_type;

    count_type count;
    size_type  start;
    size_type  offset;
    size_type  reserve;

    explicit SortBucket(size_type start = 0, count_type count = 0) noexcept
        : count(count), start(0), offset(0), reserve(0) {
    }

    SortBucket(const SortBucket & src) noexcept
        : count(src.count), start(src.start),
          offset(src.offset), reserve(src.reserve) {
    }

    ~SortBucket() {}
};

template <typename T, typename CountType>
struct PackedBucket {
    typedef T                                            value_type;
    typedef typename std::make_unsigned<CountType>::type count_type;
    typedef count_type                                   size_type;

    size_type  first;
    size_type  last;

    explicit PackedBucket(size_type count = 0) noexcept
        : first(count), last(0) {
    }

    PackedBucket(size_type first, size_type last) noexcept
        : first(first), last(last) {
    }

    PackedBucket(const PackedBucket & src) noexcept
        : first(src.first), last(src.last) {
    }

    ~PackedBucket() {}
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

template <typename CountType, typename Iterator, typename Comparer, typename DiffType, typename ValueType>
inline void dense_counting_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                       DiffType distance, const ValueType & minVal) {
    typedef Iterator iterator;
    typedef typename std::make_unsigned<CountType>::type             count_type;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    static const size_t kFixedDistance = 65536;

    diff_type length = last - first;
    assert(length > 0);

    assert(distance > 0);
    if (likely(distance < diff_type(kFixedDistance))) {
        count_type counts[kFixedDistance];
        std::memset(&counts[0], 0, sizeof(count_type) * (distance + 1));

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
                    *iter =  std::move(val);
                    ++iter;
                }
            }
        }
        assert(iter == last);
    } else {
        std::unique_ptr<count_type[]> counts(new count_type[distance + 1]());

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
                    *iter =  std::move(val);
                    ++iter;
                }
            }
        }
        assert(iter == last);
    }
}

template <typename CountType, typename Iterator, typename Comparer, typename DiffType, typename ValueType>
inline void sparse_counting_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                        DiffType distance, const ValueType & minVal) {
    typedef Iterator iterator;
    typedef typename std::make_unsigned<CountType>::type             count_type;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    static const size_t kFixedDistance = 65536;
    static const size_t kFixedBitsWordLen = kFixedDistance / sizeof(size_t);
    static const size_t kBitsPerWord = sizeof(size_t) * 8;
    static const size_t kShiftPerWord = 6;

    diff_type length = last - first;
    assert(length > 0);

    size_t bitsAlignedBytes = ((distance + 1) + sizeof(size_t) - 1) / sizeof(size_t);
    size_t maxBitsWordLen = (bitsAlignedBytes + sizeof(size_t) - 1) / sizeof(size_t);

    assert(distance > 0);
    if (likely(distance < diff_type(kFixedDistance))) {
        size_t count_bits[kFixedBitsWordLen];
        count_type counts[kFixedDistance];
        size_t maxCountWordLen = ((distance + 1) * sizeof(count_type) + sizeof(size_t) - 1) / sizeof(size_t);
        assert(maxBitsWordLen <= kFixedBitsWordLen);
        assert(maxCountWordLen <= kFixedDistance);
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
                    *iter =  std::move(val);
                    ++iter;
                }
            }
        }
        assert(iter == last);
    } else {
        std::unique_ptr<size_t[]> count_bits(new size_t[maxBitsWordLen]());
        std::unique_ptr<count_type[]> counts(new count_type[distance + 1]());
        //std::memset(&counts[0], 0, sizeof(count_type) * (distance + 1));

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
                    *iter = std::move(val);
                    ++iter;
                }
            }
        }
        assert(iter == last);
    }
}

template <typename DiffType>
inline size_t calc_shift_factor(DiffType length, DiffType distance) {
    assert(length > kStdSortThreshold);
    assert(distance > 0);
    if (likely(length <= (distance + 1))) {
        size_t lengthBits = jstd::pow2::log2_int<size_t, kStdSortThreshold>(static_cast<size_t>(length));
        //     lengthBits = (lengthBits <= kMaxLengthBits) ? lengthBits : kMaxLengthBits;
        size_t distanceBits = jstd::pow2::log2_int<size_t, 1>(static_cast<size_t>(distance));
        assert(distanceBits >= lengthBits);
        size_t shiftBits = distanceBits - lengthBits;
        if (length > DiffType(65536 * 8)) {
            debug_print("length = %u, distance = %u\n",
                        (uint32_t)length, (uint32_t)distance);
            debug_print("lengthBits = %u, distanceBits = %u, shiftBits = %u\n",
                        (uint32_t)lengthBits, (uint32_t)distanceBits, (uint32_t)shiftBits);
        }
        // (length * shiftFactor * 1.5) > distance ?
        size_t ll = length << shiftBits;
        shiftBits += ((ll + (ll >> 1)) > (size_t)distance);
        return shiftBits;
    } else {
        return 0;
    }
}

template <typename DiffType>
inline std::pair<size_t, size_t> calc_bucket_count(DiffType length, DiffType distance) {
    static const size_t kMaxBucketSize = 256 * 1024;
    static const size_t kMaxLengthBits = 18;
    size_t shiftBits   = calc_shift_factor(length, distance);
    size_t shiftFactor = size_t(1) << shiftBits;
    size_t bucketCount = (((size_t)distance + 1) + (shiftFactor - 1)) >> shiftBits;
    assert(bucketCount >= 2);
    if (length > DiffType(65536 * 8)) {
        debug_print("length = %u, distance = %u\n",
                    (uint32_t)length, (uint32_t)distance);
        debug_print("shiftBits = %u, shiftFactor = 0x%08X, bucketCount = %u\n",
                    (uint32_t)shiftBits, (uint32_t)shiftFactor, (uint32_t)bucketCount);
    }
    if (bucketCount > kMaxBucketSize) {
        // If bucketCount is bigger than kMaxBucketSize,
        // recalculate the shiftBits, shiftFactor and bucketCount,
        // let the size of bucketCount approach kMaxBucketSize.
        shiftFactor = (((size_t)distance + 1) + (kMaxBucketSize - 1)) / kMaxBucketSize;
        shiftBits = jstd::pow2::log2_int<size_t, 2>(shiftFactor);
        shiftFactor = size_t(1) << shiftBits;
        bucketCount = (((size_t)distance + 1) + (shiftFactor - 1)) >> shiftBits;
        debug_print(">> length = %u, distance = %u\n",
                    (uint32_t)length, (uint32_t)distance);
        debug_print(">> shiftBits = %u, shiftFactor = 0x%08X, bucketCount = %u\n",
                    (uint32_t)shiftBits, (uint32_t)shiftFactor, (uint32_t)bucketCount);
    }
    return std::make_pair(bucketCount, shiftBits);
}

//
// Histogram sort
//
// See: https://zh.wikipedia.org/zh-cn/%E6%8F%92%E5%80%BC%E6%8E%92%E5%BA%8F
//
template <typename CountType, typename Iterator, typename Comparer, typename DiffType, typename ValueType>
inline void small_histogram_bucket_sort(Iterator first, Iterator last, Comparer compare,
                                        DiffType length, DiffType distance,
                                        const ValueType & minVal, const ValueType & maxVal) {
    typedef Iterator iterator;
    typedef typename std::make_unsigned<CountType>::type             count_type;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
//  typedef typename std::iterator_traits<iterator>::difference_type diff_type;
    typedef PackedBucket<value_type, count_type>                     bucket_type;

    static const count_type kEmptyBucket = static_cast<count_type>(-1);

    assert(length > kStdSortThreshold);
    assert(distance > 0);

    std::pair<size_t, size_t> shiftData = calc_bucket_count(length, distance);
    size_t bucketCount = shiftData.first;
    size_t shiftBits   = shiftData.second;

    std::unique_ptr<bucket_type[]> buckets(new bucket_type[bucketCount]());
    for (iterator iter = first; iter < last; ++iter) {
        size_t index = static_cast<size_t>(*iter - minVal) >> shiftBits;
        ++buckets[index].first;
    }

    size_t total = 0;
    size_t min_count = 65535, max_count = 0;
    for (size_t i = 0; i < bucketCount; i++) {
        size_t old_count = buckets[i].first;
#if 0
        count_type value = (old_count != 0) ? static_cast<count_type>(total) : kEmptyBucket;
        buckets[i].first = value;
        buckets[i].last  = value;
        total += old_count;
#else
        if (old_count != 0) {
            buckets[i].first = static_cast<count_type>(total);
            buckets[i].last  = static_cast<count_type>(total);
            total += old_count;
            min_count = (old_count < min_count) ? old_count : min_count;
            max_count = (old_count > max_count) ? old_count : max_count;
        } else {
#ifdef _DEBUG
            buckets[i].first = kEmptyBucket;
            buckets[i].last  = kEmptyBucket;
#endif
        }
#endif
    }

    {
        std::unique_ptr<value_type[]> sortedArray(new value_type[length]);

        for (iterator iter = first; iter < last; ++iter) {
            size_t bucketIndex = static_cast<size_t>(*iter - minVal) >> shiftBits;
            count_type insertFirst = buckets[bucketIndex].first;
            count_type insertLast  = buckets[bucketIndex].last;
            assert(insertFirst != kEmptyBucket);
            ++buckets[bucketIndex].last;
            if (bucketIndex < bucketCount - 1) {
                assert(buckets[bucketIndex].last <= buckets[bucketIndex + 1].first);
            }
            value_type * insert = &sortedArray[insertLast];
            if (likely(insertFirst != insertLast)) {
                value_type * target = std::prev(insert);
                if (compare(*iter, *target)) {
                    value_type * start = &sortedArray[insertFirst];
                    do {
                        *insert = std::move(*target);
                        --insert;
                    } while (insert > start && compare(*iter, *--target));
                }
            }
            *insert = std::move(*iter);
        }

        value_type * sorted = &sortedArray[0];
        for (iterator iter = first; iter < last; ++sorted, ++iter) {
            *iter = std::move(*sorted);
        }
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void bucket_sort(RandomAccessIter first, RandomAccessIter last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (likely((size_t)length <= kStdSortThreshold)) {
        if (likely((size_t)length <= kInsertSortThreshold))
            jstd::insert_sort(first, last, compare);
        else
            std::sort(first, last, compare);
    } else {
        assert(length > 0);
        value_type minVal = *first;
        value_type maxVal = *first;
        for (iterator iter = std::next(first); iter < last; ++iter) {
#if 1
            minVal = (*iter < minVal) ? *iter : minVal;
            maxVal = (*iter > maxVal) ? *iter : maxVal;
#else
            if (*iter < minVal) minVal = *iter;
            if (*iter > maxVal) maxVal = *iter;
#endif
        }

        diff_type distance = static_cast<diff_type>(maxVal - minVal);
        if (unlikely(distance == 0)) return;

        if (likely(length <= 65536)) {
            // Short array [0, 65536]
            if (likely(distance < diff_type(65536 * 8))) {
                if (likely(distance > (length * 5 / 4)))
                    sparse_counting_bucket_sort<uint16_t>(first, last, compare, distance, minVal);
                else if (likely(distance != 0))
                    dense_counting_bucket_sort<uint16_t>(first, last, compare, distance, minVal);
            } else {
                small_histogram_bucket_sort<uint16_t>(first, last, compare, length, distance, minVal, maxVal);
            }
        } else {
            // Long array (65536, UInt32Max or UInt64Max]
            if (likely(distance < diff_type(65536 * 8))) {
                if (likely(distance > (length * 5 / 4)))
                    sparse_counting_bucket_sort<uint32_t>(first, last, compare, distance, minVal);
                else if (likely(distance != 0))
                    dense_counting_bucket_sort<uint32_t>(first, last, compare, distance, minVal);
            } else if (length < diff_type(65536 * 8) && false) {
                //
                //printf("bucket_sort() unknown branch1\n");
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
                if (bucketSize <= kBucketSizeThreshold) {
                    //
                    //printf("bucket_sort() unknown branch2\n");
                } else {
                    //printf("histogram_bucket_sort()\n");
                    //histogram_bucket_sort<uint32_t>(first, last, compare, minVal, distance, bucketSize);
                }
                small_histogram_bucket_sort<uint32_t>(first, last, compare, length, distance, minVal, maxVal);
            }
        }
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void bucket_sort(BiDirectionalIter first, BiDirectionalIter last,
                        Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIter iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::bidirectional_iterator_tag>::value,
                  "bucket_detail::bucket_sort() is not supported std::bidirectional_iterator.");
}

template <typename ForwardIter, typename Comparer>
inline void bucket_sort(ForwardIter first, ForwardIter last,
                        Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::forward_iterator_tag>::value,
                  "bucket_detail::bucket_sort() is not supported std::forward_iterator.");
}

} // namespace bucket_detail

template <typename Iterator, typename Comparer>
inline void bucket_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    bucket_detail::bucket_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void bucket_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    bucket_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUCKET_SORT_H

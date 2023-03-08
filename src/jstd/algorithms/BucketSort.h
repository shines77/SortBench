
#ifndef JSTD_BUCKET_SORT_H
#define JSTD_BUCKET_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace detail {

template <typename RandomAccessIterator, typename Comparer>
inline void bucket_sort(RandomAccessIterator first, RandomAccessIterator last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      T;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (likely(length <= 256)) {
        std::sort(first, last, compare);
    } else {
        assert(first != last);
        T minVal = *first;
        T maxVal = *first;
        for (iterator iter = std::next(first); iter < last; ++iter) {
            if (*iter < minVal) minVal = *iter;
            if (*iter > maxVal) maxVal = *iter;
        }

        T distance = maxVal - minVal;
        if (likely(distance != 0)) {
            //
        }        
    }
}

template <typename BiDirectionalIterator, typename Comparer>
inline void bucket_sort(BiDirectionalIterator first, BiDirectionalIterator last,
                        Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIterator iterator;
    if (likely(first != last)) {
    }
}

template <typename ForwardIterator, typename Comparer>
inline void bucket_sort(ForwardIterator first, ForwardIterator last,
                        Comparer compare, std::forward_iterator_tag) {
    throw std::invalid_argument("detail::bucket_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iterator, typename Comparer>
inline void BucketSort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::bucket_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void BucketSort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    BucketSort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUCKET_SORT_H

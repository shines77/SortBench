
#ifndef JSTD_BUCKET_SORT_H
#define JSTD_BUCKET_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace detail {

template <typename RandomAccessIterator, typename Comparer>
inline void bucket_sort(RandomAccessIterator first, RandomAccessIterator last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
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

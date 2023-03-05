
#ifndef JSTD_INSERT_SORT_H
#define JSTD_INSERT_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"

#include <assert.h>

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <algorithm>
#include <utility>

namespace jstd {
namespace detail {

template <typename RandomAccessIterator, typename Comparer>
inline void insert_sort(RandomAccessIterator first, RandomAccessIterator last, Comparer compare,
                        std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type T;
    if (unlikely(first == last)) return;

    for (iterator cur = std::next(first); cur != last; ++cur) {
        iterator key = cur;
        iterator target = std::prev(cur);

        if (compare(*key, *target)) {
            T tmp = std::move(*key);

            do {
                *key = std::move(*target);
                --key;
            } while (key != first && compare(tmp, *--target));

            *key = std::move(tmp);
        }
    }
}

template <typename BiDirectionalIterator, typename Comparer>
inline void insert_sort(BiDirectionalIterator first, BiDirectionalIterator last, Comparer compare,
                        std::bidirectional_iterator_tag) {
    typedef BiDirectionalIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type T;
    if (unlikely(first == last)) return;

    for (iterator cur = std::next(first); cur != last; ++cur) {
        iterator key = cur;
        iterator target = std::prev(cur);

        if (compare(*key, *target)) {
            T tmp = std::move(*key);

            do {
                *key = std::move(*target);
                --key;
            } while (key != first && compare(tmp, *--target));

            *key = std::move(tmp);
        }
    }
}

template <typename ForwardIterator, typename Comparer>
inline void insert_sort(ForwardIterator first, ForwardIterator last, Comparer compare,
                        std::forward_iterator_tag) {
    throw std::invalid_argument("detail::insert_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iterator, typename Comparer>
inline void InsertSort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::insert_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void InsertSort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    InsertSort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_INSERT_SORT_H

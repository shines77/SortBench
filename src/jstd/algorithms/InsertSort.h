
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
#include <utility>
#include <algorithm>

namespace jstd {
namespace detail {

template <typename RandomAccessIter, typename Comparer>
inline void insert_sort(RandomAccessIter begin, RandomAccessIter end, Comparer comp,
                        std::random_access_iterator_tag) {
    typedef typename std::iterator_traits<RandomAccessIter>::value_type T;
    if (unlikely(begin == end)) return;

    for (RandomAccessIter cur = begin + 1; cur != end; ++cur) {
        RandomAccessIter key = cur;
        RandomAccessIter target = cur - 1;

        if (comp(*key, *target)) {
            T tmp = std::move(*key);

            do {
                *key = std::move(*target);
                --key;
            } while (key != begin && comp(tmp, *--target));

            *key = std::move(tmp);
        }
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void insert_sort(BiDirectionalIter begin, BiDirectionalIter end, Comparer comp,
                        std::bidirectional_iterator_tag) {
    typedef typename std::iterator_traits<BiDirectionalIter>::value_type T;
    if (unlikely(begin == end)) return;

    for (BiDirectionalIter cur = begin + 1; cur != end; ++cur) {
        BiDirectionalIter key = cur;
        BiDirectionalIter target = cur - 1;

        if (comp(*key, *target)) {
            T tmp = std::move(*key);

            do {
                *key = std::move(*target);
                --key;
            } while (key != begin && comp(tmp, *--target));

            *key = std::move(tmp);
        }
    }
}

template <typename ForwardIter, typename Comparer>
inline void insert_sort(ForwardIter begin, ForwardIter end, Comparer comp,
                        std::forward_iterator_tag) {
    throw std::invalid_argument("detail::insert_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iter, typename Comparer>
inline void InsertSort(Iter begin, Iter end, Comparer comp) {
    typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
    detail::insert_sort(begin, end, comp, iterator_category());
}

template <typename Iter>
inline void InsertSort(Iter begin, Iter end) {
    typedef typename std::iterator_traits<Iter>::value_type T;
    InsertSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_INSERT_SORT_H

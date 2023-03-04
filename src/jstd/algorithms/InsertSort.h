
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
inline void half_division_insert_sort(RandomAccessIter begin, RandomAccessIter end, Comparer comp, std::random_access_iterator_tag) {
    typedef typename std::iterator_traits<RandomAccessIter>::value_type T;
    typedef typename std::iterator_traits<RandomAccessIter>::difference_type difference_type;

    difference_type length = end - begin;
    if (likely(length <= 32)) {
        if (unlikely(length <= 0)) return;
        for (RandomAccessIter cur = begin + 1; cur != end; ++cur) {
            RandomAccessIter insert = cur;
            RandomAccessIter cursor = cur - 1;

            if (comp(*insert, *cursor)) {
                T tmp = std::move(*insert);

                do {
                    *insert-- = std::move(*cursor);
                } while (insert != begin && comp(tmp, *--cursor));

                *insert = std::move(tmp);
            }
        }
    } else {
        for (RandomAccessIter cur = begin + 1; cur != end; ++cur) {
            RandomAccessIter right = begin;
            RandomAccessIter left = cur;
            RandomAccessIter insert = cur;

            difference_type distance = (left - right);
            RandomAccessIter cursor = begin + distance / 2;
            
            while (distance > 1) {
                cursor = right + distance / 2;
                bool comp_result = comp(*insert, *cursor);
                if (!comp_result) {
                    right = cursor + 1;
                } else {
                    left = cursor;
                }
                distance = left - right;
            }

            if (comp(*insert, *cursor)) {
                T tmp = std::move(*insert);

                do {
                    *insert-- = std::move(*cursor);
                } while (insert != begin && cursor != begin && comp(tmp, *--cursor));

                *insert = std::move(tmp);
            }
        }
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void insert_sort(RandomAccessIter begin, RandomAccessIter end, Comparer comp, std::random_access_iterator_tag) {
    typedef typename std::iterator_traits<RandomAccessIter>::value_type T;
    if (unlikely(begin == end)) return;

    for (RandomAccessIter cur = begin + 1; cur != end; ++cur) {
        RandomAccessIter insert = cur;
        RandomAccessIter cursor = cur - 1;

        if (comp(*insert, *cursor)) {
            T tmp = std::move(*insert);

            do {
                *insert-- = std::move(*cursor);
            } while (insert != begin && comp(tmp, *--cursor));

            *insert = std::move(tmp);
        }
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void insert_sort(BiDirectionalIter begin, BiDirectionalIter end, Comparer comp, std::bidirectional_iterator_tag) {
    typedef typename std::iterator_traits<BiDirectionalIter>::value_type T;
    if (unlikely(begin == end)) return;

    for (BiDirectionalIter cur = begin + 1; cur != end; ++cur) {
        BiDirectionalIter insert = cur;
        BiDirectionalIter cursor = cur - 1;

        if (comp(*insert, *cursor)) {
            T tmp = std::move(*insert);

            do {
                *insert-- = std::move(*cursor);
            } while (insert != begin && comp(tmp, *--cursor));

            *insert = std::move(tmp);
        }
    }
}

template <typename ForwardIter, typename Comparer>
inline void insert_sort(ForwardIter begin, ForwardIter end, Comparer comp, std::forward_iterator_tag) {
    throw std::invalid_argument("jstd::detail::insert_sort() is not supported std::forward_iterator.");
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

template <typename Iter, typename Comparer>
inline void HalfDivisionInsertSort(Iter begin, Iter end, Comparer comp) {
    typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
    detail::half_division_insert_sort(begin, end, comp, iterator_category());
}

template <typename Iter>
inline void HalfDivisionInsertSort(Iter begin, Iter end) {
    typedef typename std::iterator_traits<Iter>::value_type T;
    HalfDivisionInsertSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_INSERT_SORT_H

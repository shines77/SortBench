
#ifndef JSTD_BINARY_INSERT_SORT_H
#define JSTD_BINARY_INSERT_SORT_H

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

//
// std::random_access_iterator_tag
//
// See: https://en.cppreference.com/w/cpp/iterator/random_access_iterator
//
// LegacyRandomAccessIterator
//
// See: https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
//
template <typename RandomAccessIter, typename Comparer>
inline void binary_insert_sort(RandomAccessIter begin, RandomAccessIter end,
                               Comparer comp, std::random_access_iterator_tag) {
    typedef typename std::iterator_traits<RandomAccessIter>::value_type T;
    typedef typename std::iterator_traits<RandomAccessIter>::difference_type difference_type;

    difference_type length = end - begin;
    if (likely(length <= 64)) {
        RandomAccessIter cur = begin;
        do {
            RandomAccessIter key = cur + 1;
            if (key >= end)
                break;
            RandomAccessIter target = cur;

            if (comp(*key, *target)) {
                T tmp = std::move(*key);

                do {
                    *key-- = std::move(*target);
                } while (key != begin && comp(tmp, *--target));

                *key = std::move(tmp);
            }
            cur++;
        } while (1);
    } else {
        for (RandomAccessIter cur = begin + 1; cur != end; ++cur) {
            RandomAccessIter left = begin;
            RandomAccessIter right = cur;
            RandomAccessIter key = cur;
#if 1
            RandomAccessIter mid;
            difference_type distance = (right - left);
            
            do {                
                mid = left + distance / 2;
                bool comp_result = comp(*key, *mid);
                if (!comp_result)
                    left = mid + 1;
                else
                    right = mid;
                distance = (right - left);
            } while (distance > 1);
#else
            RandomAccessIter mid;
            
            while (left < right) {
                mid = left + (right - left) / 2;
                bool comp_result = comp(*key, *mid);
                if (comp_result)
                    right = mid;
                else
                    left = mid + 1;
            }
#endif
            if (left < key) {
                T tmp = std::move(*key);

                RandomAccessIter target = cur - 1;
                do {
                    *key = std::move(*target);
                    --key;
                    --target;
                    assert(key != begin);
                } while (target >= left);

                *key = std::move(tmp);
            }
        }
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void binary_insert_sort2(RandomAccessIter begin, RandomAccessIter end, Comparer comp,
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
inline void binary_insert_sort(BiDirectionalIter begin, BiDirectionalIter end, Comparer comp,
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
inline void binary_insert_sort(ForwardIter begin, ForwardIter end, Comparer comp,
                               std::forward_iterator_tag) {
    throw std::invalid_argument("detail::binary_insert_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iter, typename Comparer>
inline void BinaryInsertSort(Iter begin, Iter end, Comparer comp) {
    typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
    detail::binary_insert_sort(begin, end, comp, iterator_category());
}

template <typename Iter>
inline void BinaryInsertSort(Iter begin, Iter end) {
    typedef typename std::iterator_traits<Iter>::value_type T;
    BinaryInsertSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BINARY_INSERT_SORT_H

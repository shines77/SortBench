
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
namespace insert_detail {

template <typename RandomAccessIter, typename Comparer>
inline void insert_sort(RandomAccessIter first, RandomAccessIter last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type T;
#ifdef _DEBUG
    if (unlikely(first == last)) return;
#endif

    for (iterator cur = std::next(first); cur < last; ++cur) {
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

template <typename BiDirectionalIter, typename Comparer>
inline void insert_sort(BiDirectionalIter first, BiDirectionalIter last,
                        Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIter iterator;
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

template <typename ForwardIter, typename Comparer>
inline void insert_sort(ForwardIter first, ForwardIter last,
                        Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::forward_iterator_tag>::value,
                  "insert_detail::insert_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iterator, typename Comparer>
inline void insert_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    insert_detail::insert_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void insert_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    insert_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_INSERT_SORT_H

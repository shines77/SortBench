
#ifndef JSTD_BUBBLE_SORT_H
#define JSTD_BUBBLE_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace detail {

template <typename RandomAccessIterator, typename Comparer>
inline void bubble_sort(RandomAccessIterator first, RandomAccessIterator last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
    iterator limit = std::prev(last);   // last - 1
    iterator last_pos = limit;
    for (iterator iter = first; iter < limit; ++iter) {
        iterator swapped_pos = last;
        for (iterator cur = first; cur < last_pos; ++cur) {
            iterator next = std::next(cur);
            if (!compare(*cur, *next)) {
                std::iter_swap(cur, next);
                swapped_pos = cur;
            }
        }
        last_pos = swapped_pos;
        if (swapped_pos == last)
            break;
    }
}

template <typename BiDirectionalIterator, typename Comparer>
inline void bubble_sort(BiDirectionalIterator first, BiDirectionalIterator last,
                        Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIterator iterator;
    iterator limit = std::prev(last);   // last - 1
    iterator last_pos = limit;
    for (iterator iter = 0; iter < limit; ++iter) {
        iterator swapped_pos = last;
        for (iterator cur = first; cur < last_pos; ++cur) {
            iterator next = std::next(cur);
            if (!compare(*cur, *next)) {
                std::iter_swap(cur, next);
                swapped_pos = cur;
            }
        }
        last_pos = swapped_pos;
        if (swapped_pos == last)
            break;
    }
}

template <typename ForwardIterator, typename Comparer>
inline void bubble_sort(ForwardIterator first, ForwardIterator last,
                        Comparer compare, std::forward_iterator_tag) {
    throw std::invalid_argument("detail::bubble_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iterator, typename Comparer>
inline void BubbleSort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::bubble_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void BubbleSort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    BubbleSort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUBBLE_SORT_H

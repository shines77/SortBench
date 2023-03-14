
#ifndef JSTD_BUBBLE_SORT_H
#define JSTD_BUBBLE_SORT_H

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
namespace bubble_detail {

//
// See: https://www.cnblogs.com/jyroy/p/11248691.html
//
template <typename RandomAccessIter, typename Comparer>
inline void bubble_sort(RandomAccessIter first, RandomAccessIter last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIter iterator;
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

template <typename BiDirectionalIter, typename Comparer>
inline void bubble_sort(BiDirectionalIter first, BiDirectionalIter last,
                        Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIter iterator;
    if (likely(first != last)) {
        iterator limit = std::prev(last);   // last - 1
        iterator last_pos = limit;
        for (iterator iter = first; iter != limit; ++iter) {
            iterator swapped_pos = last;
            for (iterator cur = first; cur != last_pos; ++cur) {
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
}

template <typename ForwardIter, typename Comparer>
inline void bubble_sort(ForwardIter first, ForwardIter last,
                        Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    if (likely(first != last)) {
        iterator last_pos = last;
        for (iterator iter = first; iter != last; ++iter) {
            iterator swapped_pos = last;
            for (iterator cur = first; (cur != last_pos && cur != last); ++cur) {
                iterator next = std::next(cur);
                if (next == last)
                    break;
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
}

} // namespace bubble_detail

template <typename Iterator, typename Comparer>
inline void bubble_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    bubble_detail::bubble_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void bubble_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    bubble_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUBBLE_SORT_H

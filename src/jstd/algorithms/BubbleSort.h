
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

template <typename RandomAccessIter, typename Comparer>
inline void bubble_sort(RandomAccessIter begin, RandomAccessIter end, Comparer comp,
                        std::random_access_iterator_tag) {
    RandomAccessIter limit = end - 1;
    RandomAccessIter last_pos = limit;
    for (RandomAccessIter iter = begin; iter < limit; ++iter) {
        RandomAccessIter swapped_pos = end;
        for (RandomAccessIter cur = begin; cur < last_pos; ++cur) {
            RandomAccessIter next = cur + 1;
            if (!comp(*cur, *next)) {
                std::iter_swap(cur, next);
                swapped_pos = cur;
            }
        }
        last_pos = swapped_pos;
        if (swapped_pos == end)
            break;
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void bubble_sort(BiDirectionalIter begin, BiDirectionalIter end, Comparer comp,
                        std::bidirectional_iterator_tag) {
    BiDirectionalIter limit = end - 1;
    BiDirectionalIter last_pos = limit;
    for (BiDirectionalIter iter = 0; iter < limit; ++iter) {
        BiDirectionalIter swapped_pos = end;
        for (BiDirectionalIter cur = begin; cur < last_pos; ++cur) {
            BiDirectionalIter next = cur + 1;
            if (!comp(*cur, *next)) {
                std::iter_swap(cur, next);
                swapped_pos = cur;
            }
        }
        last_pos = swapped_pos;
        if (swapped_pos == end)
            break;
    }
}

template <typename ForwardIter, typename Comparer>
inline void bubble_sort(ForwardIter begin, ForwardIter end, Comparer comp,
                        std::forward_iterator_tag) {
    throw std::invalid_argument("detail::bubble_sort() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iter, typename Comparer>
inline void BubbleSort(Iter begin, Iter end, Comparer comp) {
    typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
    detail::bubble_sort(begin, end, comp, iterator_category());
}

template <typename Iter>
inline void BubbleSort(Iter begin, Iter end) {
    typedef typename std::iterator_traits<Iter>::value_type T;
    BubbleSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUBBLE_SORT_H

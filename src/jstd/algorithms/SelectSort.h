
#ifndef JSTD_SELECT_SORT_H
#define JSTD_SELECT_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace select_detail {

template <typename RandomAccessIter, typename Comparer>
inline void select_sort(RandomAccessIter first, RandomAccessIter last,
                        Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIter iterator;
    iterator limit = std::prev(last);   // last - 1
    for (iterator iter = first; iter < limit; ++iter) {
        iterator min_pos = iter;
        for (iterator cur = std::next(iter); cur < last; ++cur) {
            if (compare(*cur, *min_pos)) {
                min_pos = cur;
            }
        }
        if (min_pos != iter) {
            std::iter_swap(iter, min_pos);
        }
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void select_sort(BiDirectionalIter first, BiDirectionalIter last,
                        Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIter iterator;
    iterator limit = std::prev(last);   // last - 1
    for (iterator iter = first; iter != limit; ++iter) {
        iterator min_pos = iter;
        for (iterator cur = std::next(iter); cur != last; ++cur) {
            if (compare(*cur, *min_pos)) {
                min_pos = cur;
            }
        }
        if (min_pos != iter) {
            std::iter_swap(iter, min_pos);
        }
    }
}

template <typename ForwardIter, typename Comparer>
inline void select_sort(ForwardIter first, ForwardIter last,
                        Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    for (iterator iter = first; iter != last; ++iter) {
        iterator min_pos = iter;
        for (iterator cur = std::next(iter); cur != last; ++cur) {
            if (compare(*cur, *min_pos)) {
                min_pos = cur;
            }
        }
        if (min_pos != iter) {
            std::iter_swap(iter, min_pos);
        }
    }
}

} // namespace select_detail

template <typename Iterator, typename Comparer>
inline void select_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    select_detail::select_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void select_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    select_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_SELECT_SORT_H

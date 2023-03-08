
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
namespace detail {

template <typename RandomAccessIterator, typename Comparer>
inline void select_sort_impl(RandomAccessIterator first, RandomAccessIterator last,
                             Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
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

template <typename BiDirectionalIterator, typename Comparer>
inline void select_sort_impl(BiDirectionalIterator first, BiDirectionalIterator last,
                             Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIterator iterator;
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

template <typename ForwardIterator, typename Comparer>
inline void select_sort_impl(ForwardIterator first, ForwardIterator last,
                             Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIterator iterator;
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

} // namespace jstd::detail

template <typename Iterator, typename Comparer>
inline void select_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::select_sort_impl(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void select_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    select_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_SELECT_SORT_H

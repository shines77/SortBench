
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

template <typename RandomAccessIter, typename Comparer>
inline void select_sort(RandomAccessIter begin, RandomAccessIter end, Comparer comp,
                        std::random_access_iterator_tag) {
    typedef typename std::iterator_traits<RandomAccessIter>::value_type T;

    RandomAccessIter limit = end - 1;
    for (RandomAccessIter iter = begin; iter < limit; ++iter) {
        RandomAccessIter min = iter;
        for (RandomAccessIter cur = iter + 1; cur < end; ++cur) {
            if (comp(*cur, *min)) {
                min = cur;
            }
        }
        if (min != iter) {
            std::iter_swap(iter, min);
        }
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void select_sort(BiDirectionalIter begin, BiDirectionalIter end, Comparer comp,
                        std::bidirectional_iterator_tag) {
    typedef typename std::iterator_traits<BiDirectionalIter>::value_type T;

    BiDirectionalIter limit = end - 1;
    for (BiDirectionalIter iter = begin; iter != limit; ++iter) {
        BiDirectionalIter min = iter;
        for (BiDirectionalIter cur = iter + 1; cur != end; ++cur) {
            if (comp(*cur, *min)) {
                min = cur;
            }
        }
        if (min != iter) {
            std::iter_swap(iter, min);
        }
    }
}

template <typename ForwardIter, typename Comparer>
inline void select_sort(ForwardIter begin, ForwardIter end, Comparer comp,
                        std::forward_iterator_tag) {
    typedef typename std::iterator_traits<forward_iterator_tag>::value_type T;

    for (forward_iterator_tag iter = begin; iter != end; ++iter) {
        forward_iterator_tag min = iter;
        for (forward_iterator_tag cur = iter + 1; cur != end; ++cur) {
            if (comp(*cur, *min)) {
                min = cur;
            }
        }
        if (min != iter) {
            std::iter_swap(iter, min);
        }
    }
}

} // namespace jstd::detail

template <typename Iter, typename Comparer>
inline void SelectSort(Iter begin, Iter end, Comparer comp) {
    typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
    detail::select_sort(begin, end, comp, iterator_category());
}

template <typename Iter>
inline void SelectSort(Iter begin, Iter end) {
    typedef typename std::iterator_traits<Iter>::value_type T;
    SelectSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_SELECT_SORT_H

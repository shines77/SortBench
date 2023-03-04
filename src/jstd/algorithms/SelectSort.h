
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
    RandomAccessIter limit = end - 1;
    for (RandomAccessIter iter = begin; iter < limit; ++iter) {
        RandomAccessIter min_pos = iter;
        for (RandomAccessIter cur = iter + 1; cur < end; ++cur) {
            if (comp(*cur, *min_pos)) {
                min_pos = cur;
            }
        }
        if (min_pos != iter) {
            std::iter_swap(iter, min_pos);
        }
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void select_sort(BiDirectionalIter begin, BiDirectionalIter end, Comparer comp,
                        std::bidirectional_iterator_tag) {
    BiDirectionalIter limit = end - 1;
    for (BiDirectionalIter iter = begin; iter != limit; ++iter) {
        BiDirectionalIter min_pos = iter;
        for (BiDirectionalIter cur = iter + 1; cur != end; ++cur) {
            if (comp(*cur, *min_pos)) {
                min_pos = cur;
            }
        }
        if (min_pos != iter) {
            std::iter_swap(iter, min_pos);
        }
    }
}

template <typename ForwardIter, typename Comparer>
inline void select_sort(ForwardIter begin, ForwardIter end, Comparer comp,
                        std::forward_iterator_tag) {
    for (ForwardIter iter = begin; iter != end; ++iter) {
        ForwardIter min_pos = iter;
        for (ForwardIter cur = iter + 1; cur != end; ++cur) {
            if (comp(*cur, *min_pos)) {
                min_pos = cur;
            }
        }
        if (min_pos != iter) {
            std::iter_swap(iter, min_pos);
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

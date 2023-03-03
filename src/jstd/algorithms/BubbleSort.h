
#ifndef JSTD_BUBBLE_SORT_H
#define JSTD_BUBBLE_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <xstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {

namespace detail {

} // namespace jstd::detail

template <typename ForwardIter, typename Comparer>
inline void BubbleSort(ForwardIter begin, ForwardIter end, Comparer comp) {
    //
}

template <typename ForwardIter>
inline void BubbleSort(ForwardIter begin, ForwardIter end) {
    typedef typename std::iterator_traits<ForwardIter>::value_type T;
    BubbleSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUBBLE_SORT_H

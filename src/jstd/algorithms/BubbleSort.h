
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

} // namespace jstd::detail

template <typename RandomIter, typename Comparer>
inline void BubbleSort(RandomIter begin, RandomIter end, Comparer comp) {
    //
}

template <typename RandomIter>
inline void BubbleSort(RandomIter begin, RandomIter end) {
    typedef typename std::iterator_traits<RandomIter>::value_type T;
    BubbleSort(begin, end, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BUBBLE_SORT_H


#ifndef JSTD_UTILS_ALGORITHM_H
#define JSTD_UTILS_ALGORITHM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <assert.h>
#include <cstdint>

namespace jstd {

//
// jstd::distance(first, last)
//
namespace detail {

template <typename RandomAccessIter>
inline std::ptrdiff_t distance(RandomAccessIter first, RandomAccessIter last,
                               std::random_access_iterator_tag) {
    return static_cast<std::ptrdiff_t>(last - first);
}

//
// Including std::bidirectional_iterator
//
template <typename ForwardIter>
inline std::ptrdiff_t distance(ForwardIter first, ForwardIter last,
                               std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    std::ptrdiff_t _distance = 0;
    for (iterator iter = first; iter != last; ++iter) {
        _distance++;
    }
    return _distance;
}

} // namespace detail

//
// jstd::distance(first, last)
//
template <typename Iterator>
inline std::ptrdiff_t distance(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    return detail::distance(first, last, iterator_category());
}

//
// jstd::udistance(first, last)
//
namespace detail {

template <typename RandomAccessIter>
inline std::size_t udistance(RandomAccessIter first, RandomAccessIter last,
                             std::random_access_iterator_tag) {
    assert(last >= first);
    return static_cast<std::size_t>(last - first);
}

//
// Including std::bidirectional_iterator
//
template <typename ForwardIter>
inline std::size_t udistance(ForwardIter first, ForwardIter last,
                             std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    std::size_t _distance = 0;
    for (iterator iter = first; iter != last; ++iter) {
        _distance++;
    }
    return _distance;
}

} // namespace detail

//
// jstd::udistance(first, last)
//
template <typename Iterator>
inline std::size_t udistance(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    return detail::udistance(first, last, iterator_category());
}

} // namespace jstd

#endif // JSTD_UTILS_ALGORITHM_H

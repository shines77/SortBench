
#ifndef JSTD_BINARY_INSERT_SORT_H
#define JSTD_BINARY_INSERT_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"

#include <assert.h>

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace jstd {
namespace detail {

//
// std::random_access_iterator_tag
//
// See: https://en.cppreference.com/w/cpp/iterator/random_access_iterator
//
// LegacyRandomAccessIterator
//
// See: https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
//
template <typename RandomAccessIterator, typename Comparer>
inline void binary_insert_sort_v1_impl(RandomAccessIterator first, RandomAccessIterator last,
                                       Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      T;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (likely(length <= 256)) {
        if (likely(length > 0)) {
            iterator cur = std::next(first);
            while (cur != last) {
                iterator key = cur;
                iterator target = std::prev(cur);

                if (compare(*key, *target)) {
                    T tmp = std::move(*key);

                    do {
                        *key = std::move(*target);
                        --key;
                    } while (key != first && compare(tmp, *--target));

                    *key = std::move(tmp);
                }
                ++cur;
            }
        }
    } else {
        for (iterator cur = std::next(first); cur != last; ++cur) {
            iterator left = first;
            iterator right = cur;
            iterator key = cur;
            iterator mid;
         
            while (left < right) {
                mid = left + (right - left) / 2;
#if 0
                // branchless version
                bool comp_result = compare(*key, *mid);
                left  = (comp_result ? left : (mid + 1));
                right = (comp_result ?  mid : right    );
#else
                // branched version
                bool comp_result = compare(*key, *mid);
                if (comp_result)
                    right = mid;            // mid - 0
                else
                    left = std::next(mid);  // mid + 1
#endif
            }

            if (likely(left < key)) {
                T tmp = std::move(*key);

                iterator target = std::prev(cur);
                do {
                    *key = std::move(*target);
                    assert(key != first);
                    --key;
#ifdef NDEBUG
                    --target;
#else
                    if (target != first)
                        --target;
                    else
                        break;
#endif
                } while (target >= left);

                *key = std::move(tmp);
            }
        }
    }
}

template <typename BiDirectionalIterator, typename Comparer>
inline void binary_insert_sort_v1_impl(BiDirectionalIterator first, BiDirectionalIterator last,
                                       Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type T;
    if (unlikely(first == last)) return;

    for (iterator cur = std::next(first); cur != last; ++cur) {
        iterator key = cur;
        iterator target = std::prev(cur);

        if (compare(*key, *target)) {
            T tmp = std::move(*key);

            do {
                *key = std::move(*target);
                --key;
            } while (key != first && compare(tmp, *--target));

            *key = std::move(tmp);
        }
    }
}

template <typename ForwardIterator, typename Comparer>
inline void binary_insert_sort_v1_impl(ForwardIterator first, ForwardIterator last,
                                       Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIterator iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::forward_iterator_tag>::value,
                  "detail::binary_insert_sort_v1() is not supported std::forward_iterator.");
}

template <typename RandomAccessIterator, typename Comparer>
inline void binary_insert_sort_v2_impl(RandomAccessIterator first, RandomAccessIterator last,
                                       Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIterator iterator;
    typedef typename std::iterator_traits<iterator>::value_type      T;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (likely(length <= 256)) {
        if (likely(length > 0)) {
            iterator key = std::next(first);
            while (key != last) {
                iterator tail = key;
                iterator prev = std::prev(key);

                if (compare(*tail, *prev)) {
                    T tmp = std::move(*tail);

                    do {
                        *tail = std::move(*prev);
                        --tail;
                    } while (tail != first && compare(tmp, *--prev));

                    *tail = std::move(tmp);
                }
                ++key;
            }
        }
    } else {
        for (iterator key = std::next(first); key != last; ++key) {
            iterator left = first;
            iterator right = key;
         
            do {
                diff_type distance = (right - left);
                if (likely(distance <= 64)) {
                    if (likely(distance > 0)) {
                        iterator tail = key;
                        iterator prev = std::prev(key);
                        T tmp = std::move(*tail);
                        if (right != key) {
                            while (prev >= right) {
                                *tail = std::move(*prev);
                                --tail;
                                --prev;
                            }
                        }
                        if (compare(tmp, *prev)) {
                            do {
                                *tail = std::move(*prev);
                                --tail;
                            } while (tail != left && compare(tmp, *--prev));
                        }
                        *tail = std::move(tmp);
                    }
                    goto NextLoop;
                } else {
                    iterator mid = left + distance / 2;
#if 0
                    // branchless version
                    bool comp_result = compare(*key, *mid);
                    left  = (comp_result ? left : (mid + 1));
                    right = (comp_result ?  mid : right);
#else
                    // branched version
                    bool comp_result = compare(*key, *mid);
                    if (comp_result)
                        right = mid;            // mid - 0
                    else
                        left = std::next(mid);  // mid + 1
#endif
                }
            } while (1);

            if (likely(left < key)) {
                T tmp = std::move(*key);

                iterator tail = key;
                iterator prev = std::prev(key);
                do {
                    *tail = std::move(*prev);
                    assert(tail != first);
                    --tail;
#ifdef NDEBUG
                    --prev;
#else
                    if (prev != first)
                        --prev;
                    else
                        break;
#endif
                } while (prev >= left);

                *tail = std::move(tmp);
            }
NextLoop:
            (void)(0);
        }
    }
}

template <typename BiDirectionalIterator, typename Comparer>
inline void binary_insert_sort_v2_impl(BiDirectionalIterator first, BiDirectionalIterator last,
                                       Comparer compare, std::bidirectional_iterator_tag) {
    binary_insert_sort_v1_impl(first, last, compare, std::bidirectional_iterator_tag());
}

template <typename ForwardIterator, typename Comparer>
inline void binary_insert_sort_v2_impl(ForwardIterator first, ForwardIterator last,
                                       Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIterator iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::forward_iterator_tag>::value,
                  "detail::binary_insert_sort_v2() is not supported std::forward_iterator.");
}

} // namespace detail

template <typename Iterator, typename Comparer>
inline void binary_insert_sort_v1(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::binary_insert_sort_v1_impl(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void binary_insert_sort_v1(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    binary_insert_sort_v1(first, last, std::less<T>());
}

template <typename Iterator, typename Comparer>
inline void binary_insert_sort_v2(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    detail::binary_insert_sort_v2_impl(first, last, compare, iterator_category());
}

template <typename Iterator>
inline void binary_insert_sort_v2(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    binary_insert_sort_v2(first, last, std::less<T>());
}

template <typename Iterator, typename Comparer>
inline void binary_insert_sort(Iterator first, Iterator last, Comparer compare) {
    binary_insert_sort_v2(first, last, compare);
}

template <typename Iterator>
inline void binary_insert_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    binary_insert_sort(first, last, std::less<T>());
}

} // namespace jstd

#endif // !JSTD_BINARY_INSERT_SORT_H

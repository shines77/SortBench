
#ifndef JSTD_SGI_INTRO_SORT_H
#define JSTD_SGI_INTRO_SORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/support/Power2.h"

#include <assert.h>

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <algorithm>
#include <utility>

namespace sgi {
namespace intro_detail {

// The threshold of built-in insertion sort
static const size_t kInsertSortThreshold = 64;

template <typename RandomAccessIter>
inline RandomAccessIter mean3_iter(RandomAccessIter first, RandomAccessIter last) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type value_type;
    iterator middle = first + (last - first) / 2;
    value_type & left  = *first;
    value_type & right = *(last - 1);
    value_type & mid   = *middle;
#if 0
    iterator mean_it = (right >= mid) ? ((mid >= left)   ? middle : ((right >= left) ? first  : last)) :
                                        ((right >= left) ? last   : ((left >= mid)   ? middle : first));
    return mean_it;
#elif 1
    if (right >= mid) {
        // right >= mid
        return (mid >= left) ? middle : ((right >= left) ? first : last);
    } else {
        // mid > right
        return (right >= left) ? last : ((left >= mid) ? middle : first);
    }
#elif 1
    if (right >= mid) {
        // right >= mid
        if (mid >= left) {
            return middle;
        } else {
            // left > mid
            return (right >= left) ? first : last;
        }
    } else {
        // mid > right
        if (right >= left) {
            return last;
        } else {
            // left > right
            return (left >= mid) ? middle : first;
        }
    }
#else
    if (right >= mid) {
        // right >= mid
        if (mid >= left) {
            return middle;
        } else {
            // left > mid
            if (right >= left)
                return first;
            else
                return last;    // left > right
        }
    } else {
        // mid > right
        if (right >= left) {
            return last;
        } else {
            // left > right
            if (left >= mid)
                return middle;
            else
                return first;   // mid > left
        }
    }
#endif
}

template <typename ValueType, typename RandomAccessIter>
inline ValueType mean3(RandomAccessIter first, RandomAccessIter last) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type value_type;
    //static_assert(!std::is_same<ValueType, value_type>::value,
    //              "sgi_intro_detail::mean3(first, last) ValueType is dismatch.");

    iterator middle = first + (last - first) / 2;
    value_type & left  = *first;
    value_type & right = *(last - 1);
    value_type & mid   = *middle;
#if 0
    value_type mean = (right >= mid) ? ((mid >= left)  ? mid  : ((right >= left) ? left : right)) :
                                      ((right >= left) ? right : ((left >= mid)  ? mid  : left));
    return mean;
#elif 1
    if (right >= mid) {
        // right >= mid
        return (mid >= left) ? mid : ((right >= left) ? left : right);
    } else {
        // mid > right
        return (right >= left) ? right : ((left >= mid) ? mid : left);
    }
#elif 1
    if (right >= mid) {
        // right >= mid
        if (mid >= left) {
            return mid;
        } else {
            // left > mid
            return (right >= left) ? left : right;
        }
    } else {
        // mid > right
        if (right >= left) {
            return right;
        } else {
            // left > right
            return (left >= mid) ? mid : left;
        }
    }
#else
    if (right >= mid) {
        // right >= mid
        if (mid >= left) {
            return mid;
        } else {
            // left > mid
            if (right >= left)
                return left;
            else
                return right;   // left > right
        }
    } else {
        // mid > right
        if (right >= left) {
            return right;
        } else {
            // left > right
            if (left >= mid)
                return mid;
            else
                return left;    // mid > left
        }
    }
#endif
}

template <typename RandomAccessIter, typename ValueType, typename Comparer>
inline void unguarded_linear_insert(RandomAccessIter first, RandomAccessIter last,
                                    const ValueType & value, Comparer compare) {
    typedef RandomAccessIter iterator;

    iterator prev = last;
    --prev;
    while (compare(value, *prev)) {
        assert(prev >= first);
        *last = std::move(*prev);
        --last;
        --prev;
    }
    assert(last >= first);
    *last = value;
}

template <typename RandomAccessIter, typename Comparer>
inline void unguarded_linear_insert(RandomAccessIter first, RandomAccessIter last,
                                    Comparer compare) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type value_type;

    iterator prev = last;
    --prev;
    if (compare(*last, *prev)) {
        value_type tmp = std::move(*last);
        do {
            assert(prev >= first);
            *last = std::move(*prev);
            --last;
            --prev;
        } while (compare(tmp, *prev));
        assert(last >= first);
        *last = tmp;
    }
}

template <typename RandomAccessIter, typename ValueType, typename Comparer>
inline void guarded_linear_insert(RandomAccessIter first, RandomAccessIter last,
                                  const ValueType & value, Comparer compare) {
    typedef RandomAccessIter iterator;

    iterator prev = last;
    --prev;
    while ((prev >= first) && compare(value, *prev)) {
        *last = std::move(*prev);
        --last;
        --prev;
    }
    assert(last >= first);
    *last = value;
}

template <typename RandomAccessIter, typename Comparer>
inline void linear_insert(RandomAccessIter first, RandomAccessIter last, Comparer compare) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type value_type;
    if (compare(*last, *first)) {
        value_type value = std::move(*last);
        std::copy_backward(first, last, last + 1);
        *first = std::move(value);
    } else {
        unguarded_linear_insert(first, last, compare);
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void insertion_sort(RandomAccessIter first, RandomAccessIter last, Comparer compare) {
    typedef RandomAccessIter iterator;
    for (iterator iter = first + 1; iter < last; ++iter) {
        linear_insert(first, iter, compare);
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void unguarded_insertion_sort(RandomAccessIter first, RandomAccessIter last,
                                     Comparer compare) {
    typedef RandomAccessIter iterator;
    for (iterator iter = first + 1; iter < last; ++iter) {
        unguarded_linear_insert(first, iter, compare);
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void final_insertion_sort(RandomAccessIter first, RandomAccessIter last,
                                 Comparer compare) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (size_t(length) > kInsertSortThreshold) {
        insertion_sort(first, first + kInsertSortThreshold, compare);
        unguarded_insertion_sort(first + kInsertSortThreshold, last, compare);
    } else {
        insertion_sort(first, last, compare);
    }
}

template <typename RandomAccessIter, typename ValueType, typename Comparer>
inline RandomAccessIter unguarded_partition(RandomAccessIter first, RandomAccessIter last,
                                            const ValueType & pivot, Comparer compare){
    do {
        while (compare(*first, pivot)) {
            ++first;
        }
        --last;
        while (compare(pivot, *last)) {
            --last;
        }
        if (first >= last)
            return first;
        std::iter_swap(first, last);
        ++first;
    } while (true);
}

template <typename RandomAccessIter, typename Comparer>
inline void heap_sort(RandomAccessIter first, RandomAccessIter last,
                      Comparer compare) {
    diff_type length = last - first;
    if (unlikely(size_t(length) <= kInsertSortThreshold)) {
        jstd::insert_sort(first, last, compare);
    } else {
        //
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void partial_sort(RandomAccessIter first, RandomAccessIter last, Comparer compare) {
    std::make_heap(first, last, compare);
    std::sort_heap(first, last, compare);
}

template <typename RandomAccessIter, typename Comparer>
inline void partial_sort(RandomAccessIter first, RandomAccessIter middle,
                         RandomAccessIter last, Comparer compare) {
    typedef RandomAccessIter iterator;
    std::make_heap(first, middle, compare);
    for (iterator iter = middle; iter < last; ++iter) {
        if (compare(*iter, *first)) {
            std::pop_heap(first, middle, compare);
        }
    }
    std:::sort_heap(first, middle, compare);
}

template <typename RandomAccessIter, typename Comparer>
inline void intro_sort_loop(RandomAccessIter first, RandomAccessIter last,
                            Comparer compare, size_t depth) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    while (likely(size_t(last - first) > kInsertSortThreshold)) {
        if (likely(depth > 1)) {
            depth--;

            value_type mean = mean3<value_type>(first, last);
            //value_type mean = *mean3_iter(first, last);
            iterator pivot = unguarded_partition(first, last, mean, compare);

            intro_sort_loop(first, pivot, compare, depth);
            // intro_sort_loop(pivot + 1, last, compare, depth);
            
            first = std::next(pivot);
        } else {
            // Change to heap sort
            partial_sort(first, last, compare);
            return;
        }
    }
}

template <typename RandomAccessIter, typename Comparer>
inline void intro_sort(RandomAccessIter first, RandomAccessIter last,
                       Comparer compare, std::random_access_iterator_tag) {
    typedef RandomAccessIter iterator;
    typedef typename std::iterator_traits<iterator>::value_type      value_type;
    typedef typename std::iterator_traits<iterator>::difference_type diff_type;

    diff_type length = last - first;
    if (likely(length > 1)) {
        assert(length >= 2);
        size_t log2N = jstd::pow2::log2_int<size_t, 2>(static_cast<size_t>(length));
        assert(log2N >= 1);
        size_t depth = 2 * log2N;
        // When depth >= 2 * log2(N), change to heap sort.
        intro_sort_loop(first, last, compare, depth);
        final_insertion_sort(first, last, compare);
    }
}

template <typename BiDirectionalIter, typename Comparer>
inline void intro_sort(BiDirectionalIter first, BiDirectionalIter last,
                       Comparer compare, std::bidirectional_iterator_tag) {
    typedef BiDirectionalIter iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::bidirectional_iterator_tag>::value,
                  "sgi_intro_detail::intro_sort() is not supported std::bidirectional_iterator.");
}

template <typename ForwardIter, typename Comparer>
inline void intro_sort(ForwardIter first, ForwardIter last,
                       Comparer compare, std::forward_iterator_tag) {
    typedef ForwardIter iterator;
    typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;
    static_assert(!std::is_same<iterator_category, std::forward_iterator_tag>::value,
                  "sgi_intro_detail::intro_sort() is not supported std::forward_iterator.");
}

} // namespace intro_detail

//
// Introspective sort
//
template <typename Iterator, typename Comparer>
void intro_sort(Iterator first, Iterator last, Comparer compare) {
    typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
    intro_detail::intro_sort(first, last, compare, iterator_category());
}

template <typename Iterator>
void intro_sort(Iterator first, Iterator last) {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    intro_sort(first, last, std::less<T>());
}

} // namespace sgi

#endif // !JSTD_SGI_INTRO_SORT_H

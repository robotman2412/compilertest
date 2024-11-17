
// Copyright © 2024, Badge.Team
// https://github.com/badgeteam/badgelib
// SPDX-License-Identifier: MIT

#pragma once

#include "strong_malloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Return value for array_binsearch.
typedef struct {
    // Element or insertion index.
    size_t index;
    // Element was found in the array.
    bool   found;
} array_binsearch_t;



// Comparator function for sorting functions.
typedef int (*array_sort_comp_t)(void const *a, void const *b);
// Sort a contiguous array given a comparator function.
// The array is sorted into ascending order.
void              array_sort(void *array, size_t ent_size, size_t ent_count, array_sort_comp_t comparator);
// Binary search for a value in a sorted (ascending order) array.
array_binsearch_t array_binsearch(
    void const *array, size_t ent_size, size_t ent_count, void const *value, array_sort_comp_t comparator
);



// Copy elements between arrays.
static inline void array_copy(
    void *array_dst, void const *array_src, size_t ent_size, size_t dst_index, size_t src_index, size_t ent_count
) {
    __builtin_memmove(
        (void *)((size_t)array_dst + ent_size * dst_index),
        (void *)((size_t)array_src + ent_size * src_index),
        ent_size * ent_count
    );
}

// Insert multiple elements into an array.
// If `insert` is not NULL, the value is copied into the new space in the array.
static inline void array_insert_n(
    void *array, size_t ent_size, size_t ent_count, void const *insert, size_t index, size_t insert_count
) {
    // Copy the remainder of the array forward.
    array_copy(array, array, ent_size, index + insert_count, index, ent_count - index);
    // Copy the new data into the array.
    if (insert)
        array_copy(array, insert, ent_size, index, 0, insert_count);
}

// Remove multiple elements from an array.
// If `removed` is not NULL, the removed value is copied into it.
static inline void
    array_remove_n(void *array, size_t ent_size, size_t ent_count, void *removed, size_t index, size_t remove_count) {
    // Copy the old data out of the array.
    if (removed)
        array_copy(removed, array, ent_size, 0, index, remove_count);
    // Copy the remainder of the array backward.
    array_copy(array, array, ent_size, index, index + remove_count, ent_count - index);
}

// Insert an element into an array.
// If `insert` is not NULL, the value is copied into the new space in the array.
static inline void array_insert(void *array, size_t ent_size, size_t ent_count, void const *insert, size_t index) {
    array_insert_n(array, ent_size, ent_count, insert, index, 1);
}

// Remove an element from an array.
// If `removed` is not NULL, the removed value is copied into it.
static inline void array_remove(void *array, size_t ent_size, size_t ent_count, void *removed, size_t index) {
    array_remove_n(array, ent_size, ent_count, removed, index, 1);
}

// Insert an element into a sorted array.
static inline void array_sorted_insert(
    void *array, size_t ent_size, size_t ent_count, void const *insert, array_sort_comp_t comparator
) {
    size_t index = array_binsearch(array, ent_size, ent_count, insert, comparator).index;
    array_insert(array, ent_size, ent_count, insert, index);
}



// Resize a length-based dynamically allocated array.
static inline bool array_lencap_resize(
    void *_array_ptr, size_t ent_size, size_t *ent_count_ptr, size_t *ent_cap_ptr, size_t new_ent_count
) {
    void **array_ptr = _array_ptr;
    if (*ent_cap_ptr >= new_ent_count) {
        *ent_count_ptr = new_ent_count;
        return true;
    }
    size_t new_cap = *ent_cap_ptr ? *ent_cap_ptr : 2;
    while (new_cap < new_ent_count) new_cap *= 2;
    void *mem = realloc(*array_ptr, ent_size * new_cap);
    if (!mem)
        return false;
    *array_ptr     = mem;
    *ent_count_ptr = new_ent_count;
    *ent_cap_ptr   = new_cap;
    return true;
}

// Resize a length-based dynamically allocated array, abort if out of memory.
static inline void array_lencap_resize_strong(
    void *_array_ptr, size_t ent_size, size_t *ent_count_ptr, size_t *ent_cap_ptr, size_t new_ent_count
) {
    void **array_ptr = _array_ptr;
    if (*ent_cap_ptr >= new_ent_count) {
        *ent_count_ptr = new_ent_count;
        return;
    }
    size_t new_cap = *ent_cap_ptr ? *ent_cap_ptr : 2;
    while (new_cap < new_ent_count) new_cap *= 2;
    void *mem      = strong_realloc(*array_ptr, ent_size * new_cap);
    *array_ptr     = mem;
    *ent_count_ptr = new_ent_count;
    *ent_cap_ptr   = new_cap;
}

// Insert a multiple elements into a dynamically allocated array.
static inline bool array_lencap_insert_n(
    void       *_array_ptr,
    size_t      ent_size,
    size_t     *ent_count_ptr,
    size_t     *ent_cap_ptr,
    void const *insert,
    size_t      index,
    size_t      insert_count
) {
    void **array_ptr = _array_ptr;
    if (array_lencap_resize(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, *ent_count_ptr + insert_count)) {
        array_insert_n(*array_ptr, ent_size, *ent_count_ptr - insert_count, insert, index, insert_count);
        return true;
    }
    return false;
}

// Insert a multiple elements into a dynamically allocated array, abort if out of memory.
static inline void array_lencap_insert_n_strong(
    void       *_array_ptr,
    size_t      ent_size,
    size_t     *ent_count_ptr,
    size_t     *ent_cap_ptr,
    void const *insert,
    size_t      index,
    size_t      insert_count
) {
    void **array_ptr = _array_ptr;
    array_lencap_resize_strong(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, *ent_count_ptr + insert_count);
    array_insert_n(*array_ptr, ent_size, *ent_count_ptr - insert_count, insert, index, insert_count);
}

// Remove an element from a dynamically allocated array.
static inline void array_lencap_remove_n(
    void   *_array_ptr,
    size_t  ent_size,
    size_t *ent_count_ptr,
    size_t *ent_cap_ptr,
    void   *removed,
    size_t  index,
    size_t  remove_count
) {
    void **array_ptr = _array_ptr;
    array_remove_n(*array_ptr, ent_size, *ent_count_ptr, removed, index, remove_count);
    array_lencap_resize(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, *ent_count_ptr - remove_count);
}

// Insert an element into a dynamically allocated array.
static inline bool array_lencap_insert(
    void *_array_ptr, size_t ent_size, size_t *ent_count_ptr, size_t *ent_cap_ptr, void const *insert, size_t index
) {
    void **array_ptr = _array_ptr;
    if (array_lencap_resize(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, *ent_count_ptr + 1)) {
        array_insert(*array_ptr, ent_size, *ent_count_ptr - 1, insert, index);
        return true;
    }
    return false;
}

// Insert an element into a dynamically allocated array, abort if out of memory.
static inline void array_lencap_insert_strong(
    void *_array_ptr, size_t ent_size, size_t *ent_count_ptr, size_t *ent_cap_ptr, void const *insert, size_t index
) {
    void **array_ptr = _array_ptr;
    array_lencap_resize_strong(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, *ent_count_ptr + 1);
    array_insert(*array_ptr, ent_size, *ent_count_ptr - 1, insert, index);
}

// Remove an element from a dynamically allocated array.
static inline void array_lencap_remove(
    void *_array_ptr, size_t ent_size, size_t *ent_count_ptr, size_t *ent_cap_ptr, void *removed, size_t index
) {
    void **array_ptr = _array_ptr;
    array_remove(*array_ptr, ent_size, *ent_count_ptr, removed, index);
    array_lencap_resize(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, *ent_count_ptr - 1);
}

// Insert an element into a sorted array.
static inline bool array_lencap_sorted_insert(
    void             *_array_ptr,
    size_t            ent_size,
    size_t           *ent_count_ptr,
    size_t           *ent_cap_ptr,
    void const       *insert,
    array_sort_comp_t comparator
) {
    void **array_ptr = _array_ptr;
    size_t index     = array_binsearch(*array_ptr, ent_size, *ent_count_ptr, insert, comparator).index;
    return array_lencap_insert(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, insert, index);
}

// Insert an element into a sorted array, abort if out of memory.
static inline void array_lencap_sorted_insert_strong(
    void             *_array_ptr,
    size_t            ent_size,
    size_t           *ent_count_ptr,
    size_t           *ent_cap_ptr,
    void const       *insert,
    array_sort_comp_t comparator
) {
    void **array_ptr = _array_ptr;
    size_t index     = array_binsearch(*array_ptr, ent_size, *ent_count_ptr, insert, comparator).index;
    array_lencap_insert_strong(array_ptr, ent_size, ent_count_ptr, ent_cap_ptr, insert, index);
}

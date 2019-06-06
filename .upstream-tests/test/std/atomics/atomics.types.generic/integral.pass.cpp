//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads, pre-sm-32

// <cuda/std/atomic>

// template <>
// struct atomic<integral>
// {
//     bool is_lock_free() const volatile;
//     bool is_lock_free() const;
//     void store(integral desr, memory_order m = memory_order_seq_cst) volatile;
//     void store(integral desr, memory_order m = memory_order_seq_cst);
//     integral load(memory_order m = memory_order_seq_cst) const volatile;
//     integral load(memory_order m = memory_order_seq_cst) const;
//     operator integral() const volatile;
//     operator integral() const;
//     integral exchange(integral desr,
//                       memory_order m = memory_order_seq_cst) volatile;
//     integral exchange(integral desr, memory_order m = memory_order_seq_cst);
//     bool compare_exchange_weak(integral& expc, integral desr,
//                                memory_order s, memory_order f) volatile;
//     bool compare_exchange_weak(integral& expc, integral desr,
//                                memory_order s, memory_order f);
//     bool compare_exchange_strong(integral& expc, integral desr,
//                                  memory_order s, memory_order f) volatile;
//     bool compare_exchange_strong(integral& expc, integral desr,
//                                  memory_order s, memory_order f);
//     bool compare_exchange_weak(integral& expc, integral desr,
//                                memory_order m = memory_order_seq_cst) volatile;
//     bool compare_exchange_weak(integral& expc, integral desr,
//                                memory_order m = memory_order_seq_cst);
//     bool compare_exchange_strong(integral& expc, integral desr,
//                                 memory_order m = memory_order_seq_cst) volatile;
//     bool compare_exchange_strong(integral& expc, integral desr,
//                                  memory_order m = memory_order_seq_cst);
//
//     integral
//         fetch_add(integral op, memory_order m = memory_order_seq_cst) volatile;
//     integral fetch_add(integral op, memory_order m = memory_order_seq_cst);
//     integral
//         fetch_sub(integral op, memory_order m = memory_order_seq_cst) volatile;
//     integral fetch_sub(integral op, memory_order m = memory_order_seq_cst);
//     integral
//         fetch_and(integral op, memory_order m = memory_order_seq_cst) volatile;
//     integral fetch_and(integral op, memory_order m = memory_order_seq_cst);
//     integral
//         fetch_or(integral op, memory_order m = memory_order_seq_cst) volatile;
//     integral fetch_or(integral op, memory_order m = memory_order_seq_cst);
//     integral
//         fetch_xor(integral op, memory_order m = memory_order_seq_cst) volatile;
//     integral fetch_xor(integral op, memory_order m = memory_order_seq_cst);
//
//     atomic() = default;
//     constexpr atomic(integral desr);
//     atomic(const atomic&) = delete;
//     atomic& operator=(const atomic&) = delete;
//     atomic& operator=(const atomic&) volatile = delete;
//     integral operator=(integral desr) volatile;
//     integral operator=(integral desr);
//
//     integral operator++(int) volatile;
//     integral operator++(int);
//     integral operator--(int) volatile;
//     integral operator--(int);
//     integral operator++() volatile;
//     integral operator++();
//     integral operator--() volatile;
//     integral operator--();
//     integral operator+=(integral op) volatile;
//     integral operator+=(integral op);
//     integral operator-=(integral op) volatile;
//     integral operator-=(integral op);
//     integral operator&=(integral op) volatile;
//     integral operator&=(integral op);
//     integral operator|=(integral op) volatile;
//     integral operator|=(integral op);
//     integral operator^=(integral op) volatile;
//     integral operator^=(integral op);
// };

#include <cuda/std/atomic>
#include <cuda/std/new>
#include <cuda/std/cassert>

#include <cmpxchg_loop.h>

#include "test_macros.h"

template <class A, class T>
__host__ __device__
void
do_test()
{
    A obj(T(0));
    bool b0 = obj.is_lock_free();
    ((void)b0); // mark as unused
    obj.store(T(0));
    assert(obj == T(0));
    obj.store(T(1), cuda::std::memory_order_release);
    assert(obj == T(1));
    assert(obj.load() == T(1));
    assert(obj.load(cuda::std::memory_order_acquire) == T(1));
    assert(obj.exchange(T(2)) == T(1));
    assert(obj == T(2));
    assert(obj.exchange(T(3), cuda::std::memory_order_relaxed) == T(2));
    assert(obj == T(3));
    T x = obj;
    assert(cmpxchg_weak_loop(obj, x, T(2)) == true);
    assert(obj == T(2));
    assert(x == T(3));
    assert(obj.compare_exchange_weak(x, T(1)) == false);
    assert(obj == T(2));
    assert(x == T(2));
    x = T(2);
    assert(obj.compare_exchange_strong(x, T(1)) == true);
    assert(obj == T(1));
    assert(x == T(2));
    assert(obj.compare_exchange_strong(x, T(0)) == false);
    assert(obj == T(1));
    assert(x == T(1));
    assert((obj = T(0)) == T(0));
    assert(obj == T(0));
    assert(obj++ == T(0));
    assert(obj == T(1));
    assert(++obj == T(2));
    assert(obj == T(2));
    assert(--obj == T(1));
    assert(obj == T(1));
    assert(obj-- == T(1));
    assert(obj == T(0));
    obj = T(2);
    assert((obj += T(3)) == T(5));
    assert(obj == T(5));
    assert((obj -= T(3)) == T(2));
    assert(obj == T(2));
    assert((obj |= T(5)) == T(7));
    assert(obj == T(7));
    assert((obj &= T(0xF)) == T(7));
    assert(obj == T(7));
    assert((obj ^= T(0xF)) == T(8));
    assert(obj == T(8));

    {
        TEST_ALIGNAS_TYPE(A) char storage[sizeof(A)] = {23};
        A& zero = *new (storage) A();
        assert(zero == 0);
        zero.~A();
    }
}

template <class A, class T>
__host__ __device__
void test()
{
    do_test<A, T>();
    do_test<volatile A, T>();
}

template<template<typename, cuda::thread_scope> typename Atomic, cuda::thread_scope Scope>
__host__ __device__
void test_for_all_types()
{
    test<Atomic<char, Scope>, char>();
    test<Atomic<signed char, Scope>, signed char>();
    test<Atomic<unsigned char, Scope>, unsigned char>();
    test<Atomic<short, Scope>, short>();
    test<Atomic<unsigned short, Scope>, unsigned short>();
    test<Atomic<int, Scope>, int>();
    test<Atomic<unsigned int, Scope>, unsigned int>();
    test<Atomic<long, Scope>, long>();
    test<Atomic<unsigned long, Scope>, unsigned long>();
    test<Atomic<long long, Scope>, long long>();
    test<Atomic<unsigned long long, Scope>, unsigned long long>();
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<Atomic<char16_t, Scope>, char16_t>();
    test<Atomic<char32_t, Scope>, char32_t>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<Atomic<wchar_t, Scope>, wchar_t>();

    test<Atomic<int8_t, Scope>,    int8_t>();
    test<Atomic<uint8_t, Scope>,  uint8_t>();
    test<Atomic<int16_t, Scope>,   int16_t>();
    test<Atomic<uint16_t, Scope>, uint16_t>();
    test<Atomic<int32_t, Scope>,   int32_t>();
    test<Atomic<uint32_t, Scope>, uint32_t>();
    test<Atomic<int64_t, Scope>,   int64_t>();
    test<Atomic<uint64_t, Scope>, uint64_t>();
}

template<typename T, cuda::thread_scope Scope>
using cuda_std_atomic = cuda::std::atomic<T>;

template<typename T, cuda::thread_scope Scope>
using cuda_atomic = cuda::atomic<T, Scope>;

int main(int, char**)
{
#if !defined(__CUDA_ARCH__) || __CUDA_ARCH__ >= 600
    test_for_all_types<cuda_std_atomic, cuda::thread_scope_system>();
    test_for_all_types<cuda_atomic, cuda::thread_scope_system>();
#endif
    test_for_all_types<cuda_atomic, cuda::thread_scope_device>();
    test_for_all_types<cuda_atomic, cuda::thread_scope_block>();

  return 0;
}

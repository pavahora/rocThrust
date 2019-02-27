// MIT License
//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Google Test
#include <gtest/gtest.h>
#include "test_utils.hpp"

// Thrust
#include <thrust/swap.h>
#include <thrust/iterator/iterator_traits.h> 
#include <thrust/iterator/retag.h>
#include <thrust/system/cpp/memory.h>
#include <thrust/device_vector.h>

// HIP API
#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC
#include <hip/hip_runtime_api.h>
#include <hip/hip_runtime.h>

#define HIP_CHECK(condition) ASSERT_EQ(condition, hipSuccess)
#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC

template< class InputType >
struct Params
{
    using input_type = InputType;
};

template<class Params>
class SwapRangesTests : public ::testing::Test
{
public:
    using input_type = typename Params::input_type;
};

template<class Params>
class PrimitiveSwapRangesTests : public ::testing::Test
{
public:
    using input_type = typename Params::input_type;
};

typedef ::testing::Types<
    Params<thrust::host_vector<short>>,
    Params<thrust::host_vector<int>>,
    Params<thrust::host_vector<long long>>,
    Params<thrust::host_vector<unsigned short>>,
    Params<thrust::host_vector<unsigned int>>,
    Params<thrust::host_vector<unsigned long long>>,
    Params<thrust::host_vector<float>>,
    Params<thrust::host_vector<double>>,
    Params<thrust::device_vector<short>>,
    Params<thrust::device_vector<int>>,
    Params<thrust::device_vector<long long>>,
    Params<thrust::device_vector<unsigned short>>,
    Params<thrust::device_vector<unsigned int>>,
    Params<thrust::device_vector<unsigned long long>>,
    Params<thrust::device_vector<float>>,
    Params<thrust::device_vector<double>>
> SwapRangesTestsParams;

typedef ::testing::Types<
    Params<short>,
    Params<int>,
    Params<long long>,
    Params<unsigned short>,
    Params<unsigned int>,
    Params<unsigned long long>,
    Params<float>,
    Params<double>
> SwapRangesTestsPrimitiveParams;

TYPED_TEST_CASE(SwapRangesTests, SwapRangesTestsParams);
TYPED_TEST_CASE(PrimitiveSwapRangesTests, SwapRangesTestsPrimitiveParams);

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC

TEST(SwapRangesTests, UsingHip)
{
  ASSERT_EQ(THRUST_DEVICE_SYSTEM, THRUST_DEVICE_SYSTEM_HIP);
}

template<typename ForwardIterator1,
         typename ForwardIterator2>
ForwardIterator2 swap_ranges(my_system &system,
                             ForwardIterator1,
                             ForwardIterator1,
                             ForwardIterator2 first2)
{
    system.validate_dispatch();
    return first2;
}

TEST(SwapRangesTests, SwapRangesDispatchExplicit)
{
    thrust::device_vector<int> vec(1);

    my_system sys(0);
    thrust::swap_ranges(sys, vec.begin(), vec.begin(), vec.begin());

    ASSERT_EQ(true, sys.is_valid());
}

template<typename ForwardIterator1,
         typename ForwardIterator2>
ForwardIterator2 swap_ranges(my_tag,
                             ForwardIterator1,
                             ForwardIterator1,
                             ForwardIterator2 first2)
{
    *first2 = 13;
    return first2;
}

TEST(SwapRangesTests, SwapRangesDispatchImplicit)
{
    thrust::device_vector<int> vec(1);

    thrust::swap_ranges(thrust::retag<my_tag>(vec.begin()),
                        thrust::retag<my_tag>(vec.begin()),
                        thrust::retag<my_tag>(vec.begin()));

    ASSERT_EQ(13, vec.front());
}

TYPED_TEST(SwapRangesTests, SwapRangesSimple)
{
    using Vector = typename TestFixture::input_type;

    Vector v1(5);
    v1[0] = 0; v1[1] = 1; v1[2] = 2; v1[3] = 3; v1[4] = 4;

    Vector v2(5);
    v2[0] = 5; v2[1] = 6; v2[2] = 7; v2[3] = 8; v2[4] = 9;

    thrust::swap_ranges(v1.begin(), v1.end(), v2.begin());

    ASSERT_EQ(v1[0], 5);
    ASSERT_EQ(v1[1], 6);
    ASSERT_EQ(v1[2], 7);
    ASSERT_EQ(v1[3], 8);
    ASSERT_EQ(v1[4], 9);
    
    ASSERT_EQ(v2[0], 0);
    ASSERT_EQ(v2[1], 1);
    ASSERT_EQ(v2[2], 2);
    ASSERT_EQ(v2[3], 3);
    ASSERT_EQ(v2[4], 4);
}

TYPED_TEST(PrimitiveSwapRangesTests, SwapRanges)
{
    using T = typename TestFixture::input_type;

    const std::vector<size_t> sizes = get_sizes();
    T error_margin = 0.01;
    for(auto size : sizes)
    {
        thrust::host_vector<T> a1 = get_random_data<T>( size,
                                                        std::numeric_limits<T>::min(),
                                                        std::numeric_limits<T>::max());
        thrust::host_vector<T> a2 = get_random_data<T>( size,
                                                        std::numeric_limits<T>::min(),
                                                        std::numeric_limits<T>::max());

        thrust::host_vector<T>    h1 = a1;
        thrust::host_vector<T>    h2 = a2;
        thrust::device_vector<T>  d1 = a1;
        thrust::device_vector<T>  d2 = a2;
    
        thrust::swap_ranges(h1.begin(), h1.end(), h2.begin());
        thrust::swap_ranges(d1.begin(), d1.end(), d2.begin());

        thrust::host_vector<T> d1_h = d1;
        thrust::host_vector<T> d2_h = d2;

        for (size_t i = 0; i < size; i++){
            ASSERT_NEAR(h1[i], a2[i], error_margin);  
            ASSERT_NEAR(d1_h[i], a2[i], error_margin);
            ASSERT_NEAR(h2[i], a1[i], error_margin);
            ASSERT_NEAR(d2_h[i], a1[i], error_margin);
        }
    }
}

struct type_with_swap
{
  inline __host__ __device__
  type_with_swap()
    : m_x(), m_swapped(false)
  {}

  inline __host__ __device__
  type_with_swap(int x)
    : m_x(x), m_swapped(false)
  {}

  inline __host__ __device__
  type_with_swap(int x, bool s)
    : m_x(x), m_swapped(s)
  {}

  inline __host__ __device__
  type_with_swap(const type_with_swap &other)
    : m_x(other.m_x), m_swapped(other.m_swapped)
  {}

  inline __host__ __device__
  bool operator==(const type_with_swap &other) const
  {
    return m_x == other.m_x && m_swapped == other.m_swapped;
  }

  int m_x;
  bool m_swapped;
};

inline __host__ __device__
void swap(type_with_swap &a, type_with_swap &b)
{
  thrust::swap(a.m_x, b.m_x);
  a.m_swapped = true;
  b.m_swapped = true;
}

template<class T, class H>
void inline careful_assert(T first, H second){
    bool cond = (first == second);
    ASSERT_EQ(cond, true);
}

TEST(SwapRangesTests, SwapRangesUserSwap)
{
  thrust::host_vector<type_with_swap> h_A(3, type_with_swap(0));
  thrust::host_vector<type_with_swap> h_B(3, type_with_swap(1));

  thrust::device_vector<type_with_swap> d_A = h_A;
  thrust::device_vector<type_with_swap> d_B = h_B;

  // check that nothing is yet swapped
  type_with_swap ref = type_with_swap(0, false);
    
  ASSERT_EQ(ref, h_A[0]);
  ASSERT_EQ(ref, h_A[1]);
  ASSERT_EQ(ref, h_A[2]);

  
  careful_assert(ref, d_A[0]);
  careful_assert(ref, d_A[1]);
  careful_assert(ref, d_A[2]);

  ref = type_with_swap(1, false);
    
  ASSERT_EQ(ref, h_B[0]);
  ASSERT_EQ(ref, h_B[1]);
  ASSERT_EQ(ref, h_B[2]);

  careful_assert(ref, d_B[0]);
  careful_assert(ref, d_B[1]);
  careful_assert(ref, d_B[2]);

  // swap the ranges

  thrust::swap_ranges(h_A.begin(), h_A.end(), h_B.begin());
  thrust::swap_ranges(d_A.begin(), d_A.end(), d_B.begin());

  // check that things were swapped
  ref = type_with_swap(1, true);

  ASSERT_EQ(ref, h_A[0]);
  ASSERT_EQ(ref, h_A[1]);
  ASSERT_EQ(ref, h_A[2]);

  careful_assert(ref, d_A[0]);
  careful_assert(ref, d_A[1]);
  careful_assert(ref, d_A[2]);

  ref = type_with_swap(0, true);

  ASSERT_EQ(ref, h_B[0]);
  ASSERT_EQ(ref, h_B[1]);
  ASSERT_EQ(ref, h_B[2]);

  careful_assert(ref, d_B[0]);
  careful_assert(ref, d_B[1]);
  careful_assert(ref, d_B[2]);
  
}

#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC
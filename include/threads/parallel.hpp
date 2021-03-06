/*******************************************************
 * Copyright (c) 2021, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#pragma once

#include "async_queue.hpp"

#include <numeric>
#include <array>
#include <vector>
#include <cmath>

using dim_t = std::array<size_t, 4>;

#define NTHREADS 8
static std::vector<threads::async_queue> queues(NTHREADS);

template<typename FUNC, size_t DIM>
struct work;

class parallel_mat
{
    const void * const func;  //void * makes me sad :(
    const dim_t &bound;

public:
    template<typename FUNC>
    parallel_mat(const dim_t &iterations, FUNC func)
        : func(static_cast<void*>(&func))
        , bound(iterations)
    {
        using std::array;
        using std::begin;
        using std::end;
        using std::partial_sum;

        array<size_t, 4> w = {{0, 0, 0, 0}};
        func(w);
        //array<size_t, 4> nelems;
        //partial_sum(begin(bound), end(bound), begin(nelems));
        work<FUNC, 3> ww;
        ww(this, w);
    }
    const dim_t& getBound() const {return bound;}
    const void* getFunc() const {return func;}
};

template<typename FUNC, size_t DIM>
struct work {
    void operator()(const parallel_mat *ref, dim_t iterations)
    {
        using std::get;

        const size_t &b    = get<DIM>(ref->getBound());
              size_t &iter = get<DIM>(iterations);
        if(DIM==3) {
            work<FUNC, DIM-1> w;
            for(;  iter < b; iter++) {
                queues[iter % NTHREADS].enqueue( w, ref, iterations);
            }
            for(auto &q : queues) q.sync();
        }
        else {
            work<FUNC, DIM-1> w;
            for(;  iter < b; iter++) {
                w(ref, iterations);
            }
        }
    }
};

template<typename FUNC>
struct work<FUNC, 0> {
    void operator()(const parallel_mat *ref, dim_t iterations)
    {
        using std::get;

        const size_t &b    = get<0>(ref->getBound());
              size_t &iter = get<0>(iterations);
        const auto f = *(static_cast<const FUNC * const>(ref->getFunc()));
        for(;  iter < b; iter++) {
            f(iterations);
        }
    }
};

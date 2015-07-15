#include <thread>
#include <functional>
#include <iostream>
#include <tuple>
#include <numeric>
#include <array>

using std::function;
using std::tuple;
using std::get;
using std::partial_sum;
using std::array;
using std::begin;
using std::end;

using dim_t = array<size_t, 4>;

class parallel_mat
{
    const function<void(const dim_t&)> func;
    const dim_t bound;

    template<size_t DIM>
    void work (dim_t iterations)
    {
        const size_t &b    = get<DIM>(bound);
              size_t &iter = get<DIM>(iterations);
        for(;  iter < b; iter++) {
            work<DIM-1>(iterations);
        }
    }

public:
    parallel_mat(dim_t iterations, function<void(const dim_t&)> func)
        : func(func)
        , bound(iterations)
    {
        auto w = array<size_t, 4>{0, 0, 0, 0};
        work<3>(w);
    }
};

template<>
void parallel_mat::work<0>(dim_t iterations) {
    const size_t &b    = get<0>(bound);
          size_t &iter = get<0>(iterations);
    for(;  iter < b; iter++) {
        func(iterations);
    }
}

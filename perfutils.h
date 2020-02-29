#pragma once
#include <type_traits>
#include <limits>
#include <algorithm>

namespace perf
{
    
    // From https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
    template <class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type AlmostEqual(T x, T y, int ulp = 2)
    {
        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        return std::abs(x - y) <= std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
            // unless the result is subnormal
            || std::abs(x - y) < (std::numeric_limits<T>::min)();
    }

    struct Stats
    {
        virtual void push(double x)
        {
            _samples.emplace_back(x);
            _dirty = true;
        }

        void reset()
        {
            _samples.clear();
            _dirty = true;
        }

        double median()
        {            
            recalc();
            return _median;
        }

        double first_quartile()
        {
            recalc();
            return _1stquart;
        }

        double third_quartile()
        {
            recalc();
            return _3rdquart;
        }

        enum class Shape
        {
            kLeft,
            kRight,
            kSymmetric
        };

        Shape shape()
        {
            const double d1 = abs(median() - first_quartile());
	        const double d3 = abs(median() - third_quartile());
	        if(AlmostEqual(d1,d3))
                return Shape::kSymmetric;
            return (d1 < d3) ? Shape::kLeft : Shape::kRight;
        }

        size_t size() const { return _samples.size(); }

    private:
        std::vector<double> _samples;
        double _median = 0.0;
        double _1stquart = 0.0;
        double _3rdquart = 0.0;
        bool _dirty = true;

        void recalc()
        {
            if (_dirty)
            {
                const auto mid_iter = _samples.begin() + _samples.size() / 2;
                //NOTE: we're not doing this "properly" for odd sizes but that's not a big deal for our use
                std::nth_element(_samples.begin(), mid_iter, _samples.end());
                _median = _samples[_samples.size()/2];
                
                std::nth_element(_samples.begin(), _samples.begin() + _samples.size() / 4, mid_iter);
                _1stquart = _samples[_samples.size()/4];

                std::nth_element(mid_iter, mid_iter+_samples.size()/4,_samples.end());
                _3rdquart = _samples[_samples.size()/2 + _samples.size()/4];

                _dirty = false;
            }
        }
    };

    // Incremental statistics class
    // See https://www.johndcook.com/blog/standard_deviation/
    struct RunningStat : Stats
    {
        void push(double x) override
        {
            // See Knuth TAOCP vol 2, 3rd edition, page 232
            if(size() > 1)
            {
                const auto previousMean{ _mean };
                _mean = _mean + (x - _mean) / size();
                _S = _S + (x - previousMean) * (x - _mean);
            }
            else
            {
                _mean = x;
            }
            
            Stats::push(x);
        }

        void reset()
        {
            _mean = 0.0;
            _S = 0.0;
            Stats::reset();
        }

        double mean() const
        {
            return _mean;
        }

        double variance() const
        {
            return (size() > 1) ? _S / (size() - 1) : 0.0;
        }

        double stdev() const
        {
            return sqrt(variance());
        }

    private:
        double _mean{ 0.0 };
        double _S{ 0.0 };
    };
}

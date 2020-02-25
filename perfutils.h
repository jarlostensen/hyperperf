#pragma once
#include <type_traits>
#include <limits>

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

    // Incremental statistics class
    // See https://www.johndcook.com/blog/standard_deviation/
    class RunningStat
    {
    public:
        void Push(const double x)
        {
            m_numSamples++;

            // See Knuth TAOCP vol 2, 3rd edition, page 232
            if(m_numSamples == 1)
            {
                m_mean = x;
            }
            else
            {
                const auto previousMean{ m_mean };
                m_mean = m_mean + (x - m_mean) / m_numSamples;
                m_S = m_S + (x - previousMean) * (x - m_mean);
            }
        }

        void Reset()
        {
            m_numSamples = 0;
            m_mean = 0.0;
            m_S = 0.0;
        }

        int NumDataValues() const
        {
            return m_numSamples;
        }

        double Mean() const
        {
            return m_mean;
        }

        double Variance() const
        {
            return (m_numSamples > 1) ? m_S / (m_numSamples - 1) : 0.0;
        }

        double StandardDeviation() const
        {
            return sqrt(Variance());
        }

        bool operator==(const RunningStat &other) const
        {
            return m_numSamples == other.m_numSamples && AlmostEqual(m_mean, other.m_mean) && AlmostEqual(m_S, other.m_S);
        }

    private:
        int m_numSamples{ 0 };
        double m_mean{ 0.0 };
        double m_S{ 0.0 };
    };
}

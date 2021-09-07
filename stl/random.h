/*
 *	This file defines objects from the C++ Standard Template Library (STL)
 *   pseudo-random number generation library..
 *
 *	***************************************************************************
 *
 *	File: random.h
 *	Date: September 5, 2021
 *	Version: 1.0
 *	Author: Michael Brodsky
 *	Email: mbrodskiis@gmail.com
 *	Copyright (c) 2012-2021 Michael Brodsky
 *
 *	***************************************************************************
 *
 *  This file is part of "Pretty Good" (Pg). "Pg" is free software:
 *	you can redistribute it and/or modify it under the terms of the
 *	GNU General Public License as published by the Free Software Foundation,
 *	either version 3 of the License, or (at your option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *	along with this file. If not, see <http://www.gnu.org/licenses/>.
 *
 *	***************************************************************************
 *
 *	Description:
 *
 *		This file defines objects in the <random> header of the C++ Standard
 *		Template Library (STL) implementation - to the extent they are
 *		supported by the Arduino implementation. The objects behave according
 *		to the ISO C++11 Standard: (ISO/IEC 14882:2011).
 *
 *	**************************************************************************/

#if !defined RANDOM_H__
# define RANDOM_H__ 20210905L

# include "pg.h"
# include "numeric_limits.h"
# include "initializer_list.h"
# include "array.h"

# if defined __HAS_NAMESPACES 

namespace std
{
#pragma region linear_congruential_engine

    // A random number engine based on a Linear congruential generator (LCG) 
    // with recurrence relation: x = (ax + c) mod m.
    template<class UIntType, UIntType a, UIntType c, UIntType m> 
    class linear_congruential_engine
    {
        static_assert(
            m > 0 && 
            a > 0 && a < m && 
            c >= 0 && c < m,
            "invalid linear_congruential_engine parameters"
            );
    public:
        using result_type = UIntType;

        // The default seed value.
        static constexpr UIntType default_seed = 1;
        // Template parameter a, the multiplier term.
        static constexpr UIntType multiplier = a;
        // Template parameter c, the increment term.
        static constexpr UIntType increment = c;
        // Template parameter m, the modulus term.
        static constexpr UIntType modulus = m;
        // Returns the smallest possible value in the output range.
        static constexpr result_type min() { return numeric_limits<result_type>::min(); }
        // Returns the largest possible value in the output range.
        static constexpr result_type max() { return m - 1; }

    public:
        // Constructs the engine using default seed.
        linear_congruential_engine() : linear_congruential_engine(default_seed) {} 

        // Constructs the engine with the seed value. 
        explicit linear_congruential_engine(result_type value) : lcg_(_seed(value)) {}

        /* TODO */
        template<class Sseq>
        explicit linear_congruential_engine(Sseq& s);

        // Constructs the engine using seeds in range [first, last).
        template<class OutpuIt>
        linear_congruential_engine(OutpuIt first, OutpuIt last);

        // Copy constructor.
        linear_congruential_engine(const linear_congruential_engine&) = default;

    public:
        // Seeds the engine with value.
        void seed(result_type value = default_seed) 
        { 
            lcg_ = _seed(value); 
        }

        /* TODO */
        template<class Sseq>
        void seed(Sseq& seq);

        template<class OutpuIt>
        void seed(OutpuIt first, OutpuIt last);

        // Advances the engine's state and returns the generated value.
        result_type operator()() 
        { 
            return (lcg_ = (a * lcg_ + c) % m); 
        }

        // Advances the internal state by z times, discarding the result.
        void discard(unsigned long long z) 
        { 
            while(z--) 
                (void)operator()(); 
        }

    public:
        // Returns true if lhs and rhs have the same internal state.
        friend bool operator==(const linear_congruential_engine<UIntType, a, c, m>& lhs,
                const linear_congruential_engine<UIntType, a, c, m>& rhs)
        {
            return lhs.lcg_ == rhs.lcg_;
        }

        // Returns true if lhs and rhs do not have the same internal state.
        friend bool operator!=(const linear_congruential_engine<UIntType, a, c, m>& lhs,
                const linear_congruential_engine<UIntType, a, c, m>& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        // Returns the computed seed from value.
        result_type _seed(result_type value)
        {
            // seed = value % m, unless both value and c are multiples of m, 
            // in which case seed = default_seed;
            return value % m == 0 && c % m == 0
                ? default_seed
                : value % m;
        }

    private:
        result_type lcg_;   // The internal state buffer.
    };

    // "Minimal Standard" adopted in 1988 by Park and Miller.
    using minstd_rand0 = std::linear_congruential_engine<uint_fast32_t, 16807, 0, 2147483647>;
    // Newer "Minimum Standard", recommended by Park, Miller, and Stockmeyer in 1993.
    using minstd_rand = std::linear_congruential_engine<uint_fast32_t, 48271, 0, 2147483647>;

#pragma endregion
#pragma region xorshift128_engine

    // A random number engine based on a linear-feedback shift register (LFSR) generator.
    template<class UIntType>
    class xorshift128_engine
    {
        // This implementation is based on the one proposed by George Marsaglia, 
        // Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs".
        // 
        // Performance is based on carefully chosen seed values, which cannot 
        // all be zero.

    public:
        using result_type = UIntType;

        // The default seed value.
        static constexpr UIntType default_seed = 1;
        // Size of the internal state engine in elements.
        static constexpr std::size_t state_size = 4;
        // Returns the smallest possible value in the output range.
        static constexpr result_type min() { return numeric_limits<result_type>::min(); }
        // Returns the largest possible value in the output range.
        static constexpr result_type max() { return numeric_limits<result_type>::max() - 1; }

    private:
        using state_buffer_type = std::array<UIntType, state_size>;
        using size_type = typename state_buffer_type::size_type;

        // Default seed values.
        static const state_buffer_type Seeds;

    public:
        // Constructs the engine using default seeds.
        xorshift128_engine() : buf_(Seeds), n_() {} 
        // Constructs the engine with one of the default seeds substitued by seed.
        xorshift128_engine(UIntType seed) : buf_(Seeds), n_() { buf_[n_] = seed; }
        // Constructs the engine using seeds in range [first, last).
        template<class OutpuIt>
        xorshift128_engine(OutpuIt first, OutpuIt last) : buf_(first, last), n_() 
        { assert(std::accumulate(buf_.begin(), buf_.end(), 0) != 0); }
        // Copy constructor.
        xorshift128_engine(const xorshift128_engine& other) = default;

    public:
        // Seeds the engine with value.
        void seed(result_type value = default_seed) 
        { 
            buf_[n_] = value; 
        }

        /* TODO */
        template<class Sseq>
        void seed(Sseq& seq); 

        template<class OutpuIt>
        void seed(OutpuIt first, OutpuIt last);

        // Advances the engine's state and returns the generated value.
        result_type operator()() 
        { 
            // TODO: Needs a circular iterator for better performance.

            const result_type x = buf_[++n_ % state_size], w = buf_[(n_ - 1) % state_size], t = (x ^ (x << 11));
            return (buf_[n_ % state_size] = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
        }

        // Advances the internal state by z times, discarding the result.
        void discard(unsigned long long z) 
        { 
            while (z--) (void)operator()(); 
        }

    public:
        // Returns true if lhs and rhs have the same internal state.
        friend bool operator==(const xorshift128_engine& lhs,
            const xorshift128_engine& rhs)
        {
            return lhs.buf_ == rhs.buf_;
        }

        // Returns true if lhs and rhs do not have the same internal state.
        friend bool operator!=(const xorshift128_engine& lhs,
            const xorshift128_engine& rhs)
        {
            return !(lhs == rhs); 
        }

    private:
        state_buffer_type   buf_; // The internal state buffer.
        size_type           n_;   // The current state buffer index.
    };

    template<class UIntType>
    const typename xorshift128_engine<UIntType>::state_buffer_type 
        xorshift128_engine<UIntType>::Seeds = { 88675123, 123456789, 362436069, 521288629 };

    namespace experimental
    {
        // A random number engine based on a linear-feedback shift register (LFSR) generator.
        template<class UIntType>
        class xorshift128_engine
        {
            // This implementation is based on the one proposed by George Marsaglia, 
            // Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs".
            // 
            // Performance is based on carefully chosen seed values, which cannot 
            // all be zero.
            //
            // This implementation uses std::experimental::circular_iterator
        public:
            using result_type = UIntType;

            // The default seed value.
            static constexpr UIntType default_seed = 1;
            // Size of the internal state engine in elements.
            static constexpr std::size_t state_size = 4;
            // Returns the smallest possible value in the output range.
            static constexpr result_type min() { return numeric_limits<result_type>::min(); }
            // Returns the largest possible value in the output range.
            static constexpr result_type max() { return numeric_limits<result_type>::max() - 1; }

        private:
            using state_buffer_type = std::array<UIntType, state_size>;
            using size_type = typename state_buffer_type::size_type;

            // Default seed values.
            static const state_buffer_type Seeds;

        public:
            // Constructs the engine using default seeds.
            xorshift128_engine() : buf_(Seeds), it_(buf_) {}
            // Constructs the engine with one of the default seeds substitued by seed.
            xorshift128_engine(UIntType seed) : buf_(Seeds), it_(buf_) { *it_ = seed; }
            // Constructs the engine using seeds in range [first, last).
            template<class OutpuIt>
            xorshift128_engine(OutpuIt first, OutpuIt last) : buf_(first, last), it_(buf_) 
            {
                assert(std::accumulate(buf_.begin(), buf_.end(), 0) != 0);
            }
            // Copy constructor.
            xorshift128_engine(const xorshift128_engine& other) = default;

        public:
            // Seeds the engine with value.
            void seed(result_type value = default_seed)
            {
                *it_ = value;
            }

            /* TODO */
            template<class Sseq>
            void seed(Sseq& seq);

            template<class OutpuIt>
            void seed(OutpuIt first, OutpuIt last);

            // Advances the engine's state and returns the generated value.
            result_type operator()()
            {
                const result_type x = *++it_, w = *(it_ - 1), t = (x ^ (x << 11));
                return (*it_ = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
            }

            // Advances the internal state by z times, discarding the result.
            void discard(unsigned long long z)
            {
                while (z--) 
                    (void)operator()();
            }

        public:
            // Returns true if lhs and rhs have the same internal state.
            friend bool operator==(const xorshift128_engine& lhs,
                const xorshift128_engine& rhs)
            {
                return lhs.buf_ == rhs.buf_;
            }

            // Returns true if lhs and rhs do not have the same internal state.
            friend bool operator!=(const xorshift128_engine& lhs,
                const xorshift128_engine& rhs)
            {
                return !(lhs == rhs);
            }

        private:
            state_buffer_type   buf_; // The internal state buffer.
            circular_iterator<state_buffer_type>  it_;
        };

        template<class UIntType>
        const typename xorshift128_engine<UIntType>::state_buffer_type
            xorshift128_engine<UIntType>::Seeds = { 88675123, 123456789, 362436069, 521288629 };
    }
#pragma endregion
#pragma region subtract_with_carry_engine

    // A random number engine that uses the "subtract with carry" algorithm 
    // based on a "lagged Fibonacci" generator (LFG).
    template<class UIntType, std::size_t w, std::size_t s, std::size_t r> 
    class subtract_with_carry_engine
    {
        // consists of a sequence {Xi}(0 <= i < r), where every Xi is in interval [0, 2**w).
        // X[i] = (X[i - s] - X[i - r] - cy[i - 1]) mod M, 
        // M = 2**w, 
        // cy[i] = X[i - s] - X[i - r] - cy[i - 1] < 0 ? 1 : 0,
        // 0 < s < r, 
        // 0 < w
        static_assert(
            0 < w && 
            0 < s && s < r,
            "invalid subtract_with_carry_engine parameters"
            );

    private:
        using state_buffer_type = std::array <UIntType, r>;
        using size_type = typename state_buffer_type::size_type;

        // Modulus value.
        static constexpr UIntType M = ((UIntType)1 << r);

    public: 
        using result_type = UIntType;

        // The default seed value.
        static constexpr UIntType default_seed = 19780503U;
        // Template parameter w, the word size.
        static constexpr std::size_t word_size = w;
        // Template parameter s, the short lag.
        static constexpr std::size_t short_lag = s;
        // Template parameter r, the long lag.
        static constexpr std::size_t long_lag = r;
        // Returns the smallest possible value in the output range.
        static constexpr result_type min() { return numeric_limits<result_type>::min(); }
        // Returns the largest possible value in the output range.
        static constexpr result_type max() { return M - 1; } 

    public:
        // Constructs the engine with the default seed. 
        subtract_with_carry_engine() : subtract_with_carry_engine(default_seed) {} 

        // Constructs the engine with the seed value. 
        explicit subtract_with_carry_engine(result_type value) : X_(), cy_(), i_() 
        { 
            seed(value); 
        }

        /* TODO */
        template<class Sseq>
        explicit subtract_with_carry_engine(Sseq& seq);

        // Constructs the engine using seeds in range [first, last).
        template<class OutpuIt>
        subtract_with_carry_engine(OutpuIt first, OutpuIt last) : X_(first, last), cy_(), i_() 
        {
            for (size_type i = 0; i < r; ++i)
                cy_[i] = (typename std::make_signed<result_type>::type)(sN(i)) < 0 ? 1 : 0;
        }

        // Copy constructor.
        subtract_with_carry_engine(const subtract_with_carry_engine& other) = default;

    public:
        // Initializes the internal state using value.
        void seed(result_type value = default_seed)
        {
            minstd_rand lcg(value); 
            // Seed from output generated by lcg seeded with value.
            for (size_type i = 0; i < r; ++i)
            {
                X_[i] = lcg();
                cy_[i] = (typename std::make_signed<result_type>::type)(sN(i)) < 0 ? 1 : 0;
            }
        }

        /* TODO */
        template<class Sseq>
        void seed(Sseq& seq);

        // Advances the engine's state and returns the generated value.
        result_type operator()()
        {
            // TODO: Needs a circular iterator for better performance.

            const result_type x = sN(i_);

            cy_[i_ % r] = (typename std::make_signed<result_type>::type)(x) < 0 ? 1 : 0;

            return (X_[i_++ % r] = x % M);
        }

        // Advances the internal state by z times, discarding the result.
        void discard(unsigned long long z) 
        { 
            while (z--) 
                (void)operator()(); 
        }

        void dump()
        {
            for (size_type i = 0; i < r; ++i)
            {
                Print(i); Print(": "); Print(X_[i]); Print("\t"); PrintLn(cy_[i]);
            }
            PrintLn();
        }
    public:
        // Returns true if lhs and rhs have the same internal state.
        friend bool operator==(const subtract_with_carry_engine& lhs,
            const subtract_with_carry_engine& rhs)
        {
            return lhs.X_ == rhs.X_ && lhs.cy_ == rhs.cy_;
        }

        // Returns true if lhs and rhs do not have the same internal state.
        friend bool operator!=(const subtract_with_carry_engine& lhs,
            const subtract_with_carry_engine& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        // Computes the next term in the sequence.
        inline result_type sN(const size_type i)
        {
            // TODO: Needs a circular iterator for better performance.

            return X_[(i - s) % r] - X_[(i - r) % r] - cy_[(i - 1) % r];
        }

    private:
        state_buffer_type   X_;   // The internal state buffer.
        state_buffer_type   cy_;  // The internal carry buffer.
        size_type           i_;   // The current buffer index.
    };

    // A subtract-with-carry pseudo-random generator of 24-bit numbers.
    using ranlux24_base = std::subtract_with_carry_engine<uint_fast32_t, 24, 10, 24>;
    // A subtract-with-carry pseudo-random generator of 48-bit numbers.
    using ranlux48_base = std::subtract_with_carry_engine<uint_fast64_t, 48, 5, 12>;

#pragma endregion
#pragma region discard_block_engine

    // A pseudo-random number engine adaptor that discards a certain amount of data 
    // produced by the base engine. 
    template<class Engine, std::size_t P, std::size_t R> 
    class discard_block_engine 
    {
    public:
        using base_type = Engine;
        using result_type = typename base_type::result_type;

        // Template parameter P, the size of the block.
        static constexpr std::size_t block_size = P;
        // Template parameter R, the number of used values per block.
        static constexpr std::size_t used_block = R;
        // Returns the smallest possible value in the output range.
        static constexpr result_type min() { return Engine::min(); }
        // Returns the largest possible value in the output range.
        static constexpr result_type max() { return Engine::max(); }

    public:
        // Constructs the engine with the underlying engine default-constructed.
        discard_block_engine() : engine_(Engine()), n_() {}
        // Constructs the engine with the underlying engine seeded with seed.
        explicit discard_block_engine(result_type seed) : engine_(Engine(seed)), n_() {}
        /* TODO */
        template<class Sseq>
        explicit discard_block_engine(Sseq& seq);
        // Constructs the engine seeding the underlying engine with range [first, last).
        template<class OutpuIt>
        discard_block_engine(OutpuIt first, OutpuIt last);
        // Copy constructor.
        explicit discard_block_engine(const Engine& other) : engine_(other), n_() {}
        // Move constructor.
        explicit discard_block_engine(Engine&& other) : engine_(std::move(other)), n_() {}

    public:
        // Returns the underlying engine.
        const Engine& base() const noexcept 
        { 
            return engine_; 
        }

        // Seeds the underlying engine using its default value.
        void seed() 
        { 
            engine_.seed(); 
        }

        // Seeds the underlying engine using value.
        void seed(result_type value) 
        { 
            engine_.seed(value); 
        }

        /* TODO */
        template <class Sseq>
        void seed(Sseq& seq);

        // Advances the engine's state and returns the generated value.
        result_type operator()()
        {
            if (n_ >= used_block) 
            { 
                engine_.discard(block_size - used_block);
                n_ = 0; 
            }
            ++n_;

            return engine_();
        }

        // Advances the internal state by z times, discarding the result.
        void discard(unsigned long long z)
        {
            while (z--)
                (void)operator()();
        }

    public:
        // Returns true if lhs.base() and rhs.base() have the same internal state.
        friend bool operator==(const discard_block_engine& lhs,
            const discard_block_engine& rhs)
        {
            return lhs.base() == rhs.base();
        }

        // Returns true if lhs.base() and rhs.base() do not have the same internal state.
        friend bool operator!=(const discard_block_engine& lhs,
            const discard_block_engine& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        Engine engine_; // The base engine.
        std::size_t n_; // The use/discard counter.
    };

    // A subtract-with-carry pseudo-random generator of 24-bit numbers with accelerated advancement.
    using ranlux24 = std::discard_block_engine<std::ranlux24_base, 223, 23>;
    // A subtract-with-carry pseudo-random generator of 48-bit numbers with accelerated advancement.
    using ranlux48 = std::discard_block_engine<std::ranlux48_base, 389, 11>;

#pragma endregion
}

# else // !defined __HAS_NAMESPACES
#  error Requires C++11 and namespace support.
# endif // defined __HAS_NAMESPACES 
#endif // !defined RANDOM_H__

/*
 *	This file defines objects in C++ Standard Template Library (STL) general
 *	utility library.
 *
 *	***************************************************************************
 *
 *	File: tuple.h
 *	Date: September 2, 2021
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
 *	GNU General Public License for more detail.
 *
 *  You should have received a copy of the GNU General Public License
 *	along with this file. If not, see <http://www.gnu.org/licenses/>.
 *
 *	***************************************************************************
 *
 *	Description:
 *
 *		This file defines objects in the <tuple> header of a C++ Standard
 *		Template Library (STL) implementation - to the extent they are
 *		supported by the Arduino implementation. The objects behave according
 *		to the ISO C++11 Standard: (ISO/IEC 14882:2011), except as noted. 
 * 
 *  Notes:
 * 
 *      This file defines only a very rudimentary implementation of the STL 
 *      std::tuple type, with basic functions to create and access tuple 
 *      members and size. The implementation uses recursive data structures by 
 *      composition
 *
 *	**************************************************************************/

#if !defined __TUPLE_H
# define __TUPLE_H 20210902L

# include "pg.h"    // Pg environment.

# if defined __HAS_NAMESPACES

namespace std
{
    template <class T, class... Ts>
    class tuple;

    template <class T> 
    struct tuple_size;

    template<std::size_t I, class T>
    struct tuple_element;

    namespace detail
    {
        template <std::size_t N, class T, class... Ts>
        struct get_nth_type : get_nth_type<N - 1, Ts...> 
        {
            static_assert(N < sizeof...(Ts) + 1, "index out of range");
        };

        template <class T, class... Ts>
        struct get_nth_type<0, T, Ts...> { T value; };

        template <std::size_t N>
        struct get_impl 
        {
            template <class... Ts>
            static decltype(get_nth_type<N, Ts...>::value)& get(tuple<Ts...>& t) 
            {
                return get_impl<N - 1>::get(t.rest);
            }
        };

        template <>
        struct get_impl<0> 
        {
            template <class T, class... Ts>
            static T& get(tuple<T, Ts...>& t) 
            {
                return t.value;
            }
        };
    }

    // Extracts the Ith element from the tuple. I must be an integer value in[0, sizeof...(Types)).
    template <std::size_t N, class... Ts>
    decltype(detail::get_nth_type<N, Ts...>::value)& get(tuple<Ts...>& t) 
    {
        return detail::get_impl<N>::get(t);
    }

    // Type capable to hold a collection of heterogeneous elements.
    template <class T, class... Ts>
    class tuple {
        template <std::size_t N>
        friend class detail::get_impl;

    public:
        constexpr tuple() = default;
        tuple(const T& t, const Ts&... ts) : value(t), rest(ts...) {}

    public:
        // Returns the number of tuple elements.
        constexpr int size() const { return 1 + rest.size(); }

    private:
        T value;            // The 0th tuple element.
        tuple<Ts...> rest;  // The rest of the tuple elements.
    };

    // Type capable of holding a collection of heterogeneous elements.
    template <class T>
    class tuple<T> {
        template <std::size_t N>
        friend class detail::get_impl;

    public:
        constexpr tuple() = default;
        tuple(const T& t) : value(t) {}

    public:
        constexpr std::size_t size() const { return 1; }

    private:
        T value; // The 0th tuple element.
    };

    // Returns the number of elements in a tuple as a compile-time constant expression.
    template<class... Types>
    struct tuple_size<std::tuple<Types...>>
        : std::integral_constant<std::size_t, sizeof...(Types)> 
    {};

    // Provides compile-time indexed access to the types of the elements of the tuple. recursive case
    template<std::size_t I, class Head, class... Tail>
    struct tuple_element<I, std::tuple<Head, Tail...>>
        : std::tuple_element<I - 1, std::tuple<Tail...>> 
    {};

    // Provides compile-time indexed access to the types of the elements of the tuple. base case
    template<class Head, class... Tail>
    struct tuple_element<0, std::tuple<Head, Tail...>> 
    {
        using type = Head;
    };
}

# else // !defined __HAS_NAMESPACES
#  error Requires C++11 and namespace support.
# endif // defined __HAS_NAMESPACES
#endif // !defined __TUPLE_H

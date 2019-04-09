
// Copyright (c) 2010-2019 niXman (i dot nixman dog gmail dot com). All
// rights reserved.
//
// This file is part of YAS(https://github.com/niXman/yas) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef __yas__types__utility__object_hpp
#define __yas__types__utility__object_hpp

#include <yas/detail/type_traits/type_traits.hpp>
#include <yas/detail/type_traits/serializer.hpp>
#include <yas/detail/tools/cast.hpp>
#include <yas/detail/tools/tuple_element_switch.hpp>
#include <yas/detail/tools/json_tools.hpp>

#include <yas/object.hpp>

namespace yas {
namespace detail {

/***************************************************************************/

template<std::size_t F, typename KVI, typename... Pairs>
struct serializer<
    type_prop::not_a_fundamental,
    ser_case::use_internal_serializer,
    F,
    object<KVI, Pairs...>
> {
    template<typename Archive>
    static Archive& save(Archive &ar, const object<KVI, Pairs...> &o) {
        __YAS_CONSTEXPR_IF ( F & yas::json ) {
            ar.write("{", 1);
        }

        apply(ar, o.pairs);

        __YAS_CONSTEXPR_IF ( F & yas::json ) {
            ar.write("}", 1);
        }

        return ar;
    }

    template<typename Archive>
    static Archive& load(Archive &ar, object<KVI, Pairs...> &o) {
        __YAS_CONSTEXPR_IF ( F & yas::json ) {
            __YAS_CONSTEXPR_IF ( !(F & yas::compacted) ) {
                json_skipws(ar);
            }
            __YAS_THROW_IF_BAD_JSON_CHARS(ar, "{");

            apply(ar, o.map, o.pairs);

            __YAS_CONSTEXPR_IF ( !(F & yas::compacted) ) {
                json_skipws(ar);
            }
            __YAS_THROW_IF_BAD_JSON_CHARS(ar, "}");
        } else {
            apply(ar, o.map, o.pairs);
        }

        return ar;
    }

private:
    // save
    template<std::size_t I = 0, typename Archive, typename... Tp>
    static typename std::enable_if<I == sizeof...(Tp), Archive &>::type
    apply(Archive &ar, const std::tuple<Tp...> &) { return ar; }

    template<std::size_t I = 0, typename Archive, typename... Tp>
    static typename std::enable_if<I < sizeof...(Tp), Archive &>::type
    apply(Archive &ar, const std::tuple<Tp...> &t) {
        ar & std::get<I>(t);

        __YAS_CONSTEXPR_IF ( (F & yas::json) && I+1 < sizeof...(Tp) ) {
            ar.write(",", 1);
        }

        return apply<I+1>(ar, t);
    }

    // load
    template<std::size_t I = 0, typename Archive, typename M, typename... Tp>
    static typename std::enable_if<I == sizeof...(Tp), Archive &>::type
    apply(Archive &ar, const M &, std::tuple<Tp...> &) {
        __YAS_CONSTEXPR_IF ( F & yas::json ) {
            json_skipws(ar);

            const char ch = ar.peekch();
            if ( ch == '}' ) {
                return ar;
            } else {
                __YAS_THROW_IF_BAD_JSON_CHARS(ar, ",");
                json_skipws(ar);
                json_skip_object(ar);
                ar.ungetch('}');
            }
        }

        return ar;
    }

    template<std::size_t I = 0, typename Archive, typename M, typename... Tp>
    static typename std::enable_if<I < sizeof...(Tp), Archive &>::type
    apply(Archive &ar, const M &m, std::tuple<Tp...> &t) {
        __YAS_CONSTEXPR_IF ( F & yas::json ) {
            __YAS_CONSTEXPR_IF ( F & yas::compacted ) {
                ar & std::get<I>(t);
            } else {
                while ( true ) {
                    char key[1024];
                    json_skipws(ar);
                    json_read_key(ar, key, sizeof(key));
                    json_skipws(ar);

                    const std::uint32_t hash = fnv1a(key);
                    const auto it = m.find(hash);
                    if ( it.key ) {
                        tuple_switch(ar, it.val, t);
                        break;
                    } else {
                        json_skipws(ar);
                        json_skip_val(ar);
                        json_skipws(ar);

                        const char ch = ar.peekch();
                        if ( ch == '}' ) {
                            ar.getch();

                            if ( ar.empty() ) {
                                __YAS_THROW_NO_EXPECTED_JSON_KEY("no expected JSON key");
                            }

                            return ar;
                        }

                        __YAS_THROW_IF_BAD_JSON_CHARS(ar, ",");
                    }
                }
            }
            __YAS_CONSTEXPR_IF ( I+1 < sizeof...(Tp) ) {
                json_skipws(ar);

                __YAS_THROW_IF_BAD_JSON_CHARS(ar, ",");
            }
        } else {
            ar & std::get<I>(t);
        }

        return apply<I+1>(ar, m, t);
    }
};

/***************************************************************************/

} // namespace detail
} // namespace yas

#endif // __yas__types__utility__object_hpp
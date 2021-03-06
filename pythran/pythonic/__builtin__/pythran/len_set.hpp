#ifndef PYTHONIC_BUILTIN_PYTHRAN_LEN_SET_HPP
#define PYTHONIC_BUILTIN_PYTHRAN_LEN_SET_HPP

#include "pythonic/utils/proxy.hpp"

#include <set>

namespace pythonic {
    namespace __builtin__ {
        namespace pythran {

            template <class Iterable>
                size_t len_set(Iterable const& s) {
                    return std::set<typename Iterable::iterator::value_type>(s.begin(), s.end()).size();
                }

            PROXY(pythonic::__builtin__::pythran, len_set);
        }
    }
}

#endif


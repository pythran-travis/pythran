#ifndef PYTHONIC_MODULE_NUMPY_H
#define PYTHONIC_MODULE_NUMPY_H

#include <vector>
#include <cmath>
#include <cstdint>

namespace pythonic {
    namespace numpy {

        /* a few classical constants */
        double const pi = 3.141592653589793238462643383279502884;
        double const e = 2.718281828459045235360287471352662498;

        /* numpy standard types */
        namespace proxy {
            // these typedefs are not functions, but the default constructor
            // make it legal to write pythonic::proxy::double_
            // as generated by pythran
            // so we put these typedefs in the proxy namespace
            typedef std::complex<double> complex;
            typedef std::complex<float> complex32;
            typedef std::complex<double> complex64;
            typedef std::complex<long double> complex128;
            typedef float float_;
            typedef float float32;
            typedef double float64;
            typedef double float128;
            typedef double double_;
            typedef int8_t int8;
            typedef int16_t int16;
            typedef int32_t int32;
            typedef int64_t int64;
            typedef uint8_t uint8;
            typedef uint16_t uint16;
            typedef uint32_t uint32;
            typedef uint64_t uint64;
        }



        template<class T>
            struct finalType
            {
                typedef T Type;
            };

        template<class T>
            struct finalType<core::list<T>>
            {
                typedef typename finalType<T>::Type Type;
            };

        template<class type>
            struct depth
            {
                enum { Value = 0 };
            };

        template<class type>
            struct depth<core::list<type>>
            {
                enum { Value = 1 + depth<type>::Value };
            };

        template<class... T, class type>
            core::ndarray<type,sizeof...(T)> build_array(core::list<type> const& prev_l, type const& val, T const& ...l)
            {
                core::ndarray<type, sizeof...(l)> a({l...});
                for(int i=0;i<prev_l.size(); i++)
                    a.data->data[i] = prev_l[i];
                return a;
            }

        template<class... T, class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value + sizeof...(T)> build_array(core::list<core::list<type> > const &prev_l, core::list<type> const& l, T const& ...s)
            {
                core::list<type> accumul(0);
                for(size_t i=0;i<prev_l.size(); i++)
                    for(size_t j=0; j<l.size(); j++)
                        accumul.push_back(prev_l[i][j]);
                return build_array(accumul, l[0], s..., l.size());
            }

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> array(core::list<type> const& l)
            {
                return build_array(l, l[0], l.size());
            }

        PROXY(pythonic::numpy, array);

        template<class... T>
            core::ndarray<double, sizeof...(T)> build_cst_array(double val, T... t)
            {
                return core::ndarray<double, sizeof...(t)>({(size_t)t...}, val);
            }

        template<int N>
            struct apply_to_tuple
            {
                template<typename... T, typename... S>
                    static core::ndarray<double, sizeof...(T)> builder(double val, std::tuple<T...> const& t, S... s)
                    {
                        return apply_to_tuple<N-1>::builder(val, t, std::get<N-1>(t), s...);
                    }
            };

        template<>
            struct apply_to_tuple<0>
            {
                template<typename... T, typename... S>
                    static core::ndarray<double, sizeof...(T)> builder(double val, std::tuple<T...> const& t, S... s)
                    {
                        return build_cst_array(val, s...);
                    }
            };

#define NOT_INIT_ARRAY(NAME)\
        template<class... T>\
            core::ndarray<double, sizeof...(T)> NAME(std::tuple<T...> const& t)\
            {\
                return apply_to_tuple<sizeof...(T)-1>::builder(0, t, std::get<sizeof...(T)-1>(t));\
            }\
\
        template<unsigned long N>\
            core::ndarray<double, N> NAME(std::array<long, N> const &t)\
            {\
                return core::ndarray<double, N>(t);\
            }\
\
        core::ndarray<double,1> NAME(size_t size)\
        {\
            return core::ndarray<double, 1>({size});\
        }

        NOT_INIT_ARRAY(zeros)

        NOT_INIT_ARRAY(empty)

        PROXY(pythonic::numpy, zeros);

        template<class... T>
            core::ndarray<double, sizeof...(T)> ones(std::tuple<T...> const& t)
            {
                return apply_to_tuple<sizeof...(T)-1>::builder(double(1), t, std::get<sizeof...(T)-1>(t));
            }

        template<unsigned long N>
            core::ndarray<double, N> ones(std::array<long, N> const &t)
            {
                return core::ndarray<double, N>(t);
            }

        template<class Type = proxy::double_>
        core::ndarray<Type,1> ones(size_t size, Type dtype= Type())
        {
            return core::ndarray<Type, 1>({size}, Type(1));
        }

        PROXY(pythonic::numpy, ones);

        PROXY(pythonic::numpy, empty);

        template<class U, class V, class T=long>
        core::ndarray<decltype(std::declval<U>()+std::declval<V>()+std::declval<T>()), 1> arange(U begin, V end, T step=T(1))
        {
            typedef decltype(begin+end+step) combined_type;
            size_t n = std::max(combined_type(0),combined_type(std::ceil((end - begin)/step)));
            core::ndarray<combined_type, 1> a({n});
            if(n>0)
            {
                a[0] = begin;
                for(size_t i = 1; i< n; ++i) {
                    a[i] = a[i-1] + step;
                }
            }
            return a;
        }

        core::ndarray<long, 1> arange(long end)
        {
            __builtin__::xrange xr(end);
            return core::ndarray<long, 1>(xr.begin(), xr.end());
        }

        PROXY(pythonic::numpy, arange);

        core::ndarray<double, 1> linspace(double start, double stop, size_t num=50, bool endpoint = true)
        {
            double step = (stop - start) / (num - endpoint);
            core::ndarray<double, 1> a({num});
            if(num>0)
                a[0] = start;
            std::transform(a.data->data, a.data->data + num - 1, a.data->data + 1, std::bind(std::plus<double>(), step, std::placeholders::_1));
            return a;
        }

        PROXY(pythonic::numpy, linspace);

        template<class... T, class type>
            core::ndarray<type,sizeof...(T)> build_cst_array_from_list(type cst, core::list<type> const& prev_l, type const& val, T const& ...l)
            {
                return core::ndarray<type, sizeof...(l)>({l...}, cst);
            }

        template<class... T, class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value + sizeof...(T)> build_cst_array_from_list(typename finalType<type>::Type cst, core::list<core::list<type> > const &prev_l, core::list<type> const& l, T const& ...s)
            {
                return build_cst_array_from_list(cst, l, l[0], s..., (size_t)l.size());
            }

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> ones_like(core::list<type> const& l)
            {
                return build_cst_array_from_list(1, l, l[0], (size_t)l.size());
            }

        PROXY(pythonic::numpy, ones_like);

        template<class... T, class type>
            core::ndarray<type,sizeof...(T)> build_not_init_array_from_list(core::list<type> const& prev_l, type const& val, T ...l)
            {
                return core::ndarray<type, sizeof...(l)>({l...});
            }

        template<class... T, class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value + sizeof...(T)> build_not_init_array_from_list(core::list<core::list<type> > const &prev_l, core::list<type> const& l, T ...s)
            {
                return build_not_init_array_from_list(l, l[0], s..., (size_t)l.size());
            }

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> zeros_like(core::list<type> const& l)
            {
                return build_cst_array_from_list(0, l, l[0], (size_t)l.size());
            }
        PROXY(pythonic::numpy, zeros_like);

        template<class type>
            core::ndarray<typename finalType<type>::Type, depth<core::list<type>>::Value> empty_like(core::list<type> const& l)
            {
                return build_not_init_array_from_list(l, l[0], (size_t)l.size());
            }

        PROXY(pythonic::numpy, empty_like);

        template<class T, unsigned long N, class ...S>
            core::ndarray<T,sizeof...(S)> reshape(core::ndarray<T,N> const& array, S ...s)
            {
                long shp[] = {s...};
                return core::ndarray<T,sizeof...(s)>(array.data, 0, shp);
            }

        PROXY(pythonic::numpy, reshape);

        template<class T, unsigned long N>
            core::ndarray<T,1> cumsum(core::ndarray<T,N> const& array)
            {
                core::ndarray<T,1> a({array.data->n});
                a.data->data[0] = array.data->data[0];
                std::transform(a.begin(), a.end()-1, array.data->data + 1, a.begin()+1, std::plus<T>());
                return a;
            }

        template<class T, unsigned int N>
        struct axis_helper
        {
            static core::ndarray<T,N> axis_cumsum( core::ndarray<T,N> const& array, long axis)
            {
                if(axis<0 || axis >=N)
                    throw __builtin__::ValueError("axis out of bounds");

                core::ndarray<T,N> a(*array.shape);
                if(axis==0)
                {
                    std::copy(array.data->data + *array.offset_data, array.data->data + *array.offset_data + (*array.shape)[N-1], a.data->data);
                    std::transform(a.begin(), a.end()-1, array.begin() + 1, a.begin() + 1, std::plus<core::ndarray<T,N-1>>());
                }
                else
                    std::transform(array.begin(), array.end(), a.begin(), std::bind(axis_helper<T,N-1>::axis_cumsum, std::placeholders::_1, axis-1));
                return a;
            }

            static typename std::remove_reference<typename core::ndarray_helper<T,N>::result_type>::type axis_sum( core::ndarray<T,N> const& array, long axis)
            {
                if(axis<0 || axis >=N)
                    throw __builtin__::ValueError("axis out of bounds");

                if(axis==0)
                {
                    assert(array.begin() < array.end());
                    core::ndarray<T,N> a(*(array.shape));
                    *a.begin() = *array.begin(); 
                    return std::accumulate(array.begin() + 1, array.end(), *array.begin());
                }
                else
                {
                    std::array<T,N-1> shp;
                    std::copy(array.shape->begin(), array.shape->end() - 1, shp.begin());
                    core::ndarray<T,N-1> a(shp);
                    std::transform(array.begin(), array.end(), a.begin(), std::bind(axis_helper<T,N-1>::axis_sum, std::placeholders::_1, axis-1));
                    return a;
                }
            }

            template<class Op>
            static typename std::remove_reference<typename core::ndarray_helper<T,N>::result_type>::type axis_minmax( core::ndarray<T,N> const& array, long axis, Op op)
            {
                if(axis<0 || axis>=N)
                    throw __builtin__::ValueError("axis out of bounds");

                if(axis==0)
                {
                    std::array<T,N-1> shp;
                    size_t size = 1;
                    for(auto i= array.shape->begin() + 1, j = shp.begin(); i<array.shape->end(); i++, j++)
                        size*=(*j = *i);
                    core::ndarray<T,N-1> a(shp);
                    core::ndarray_flat<T,N-1> a_iter(a);
                    std::copy(array.data->data + *array.offset_data, array.data->data + *array.offset_data + size, a_iter.begin());
                    for(auto i = array.begin() + 1; i<array.end(); i++)
                    {
                        auto next_subarray = *i;  //we need this variable to keep this ndarray alive while iter is used
                        core::ndarray_flat_const<T,N-1> iter(next_subarray);
                        auto k = a_iter.begin();
                        for(auto j = iter.begin(); j<iter.end(); j++)
                        {
                            *k=op(*k,*j);
                            k++;
                        }
                    }
                    return a;
                }
                else
                {
                    std::array<T,N-1> shp;
                    std::copy(array.shape->begin(), array.shape->end() - 1, shp.begin());
                    core::ndarray<T,N-1> a(shp);
                    std::transform(array.begin(), array.end(), a.begin(), std::bind(axis_helper<T,N-1>::template axis_minmax<Op>, std::placeholders::_1, axis-1, op));
                    return a;
                }
            }
        };

        template<class T>
        struct axis_helper<T,1>
        {
            static core::ndarray<T,1> axis_cumsum( core::ndarray<T,1> const& array, long axis)
            {
                if(axis!=0)
                    throw __builtin__::ValueError("axis out of bounds");

                core::ndarray<T,1> a(*array.shape);
                std::copy(array.begin(), array.end(), a.begin());
                std::transform(a.begin(), a.end()-1, array.begin() + 1, a.begin() + 1, std::plus<T>());
                return a;
            }

            static T axis_sum( core::ndarray<T,1> const& array, long axis)
            {
                if(axis!=0)
                    throw __builtin__::ValueError("axis out of bounds");

                return std::accumulate(array.begin(), array.end(), 0);
            }

            template<class Op>
            static T axis_minmax( core::ndarray<T,1> const& array, long axis, Op op)
            {
                if(axis!=0)
                    throw __builtin__::ValueError("axis out of bounds");

                T res = *array.begin();
                for(auto i = array.begin() + 1; i<array.end(); i++)
                    res = op(res, *i);
                return res;
            }
        };

        template<class T, unsigned long N>
            core::ndarray<T,N> cumsum( core::ndarray<T,N> const& array, long axis)
            {
                return axis_helper<T,N>::axis_cumsum(array, axis);
            }

        PROXY(pythonic::numpy, cumsum);

        template<class T, unsigned long N>
            T min(core::ndarray<T,N> const& array)
            {
                core::ndarray_flat_const<T,N> iter(array);
                T res = *iter.begin();
                for(auto i = iter.begin() + 1; i<iter.end(); i++)
                    res = std::min(res, *i);
                return res;
            }

        template<class T, unsigned long N>
            typename std::remove_reference<typename core::ndarray_helper<T,N>::result_type>::type min( core::ndarray<T,N> const& array, long axis)
            {
                return axis_helper<T,N>::axis_minmax(array, axis, (T const& (*)(T const&, T const&))std::min<T>);
            }

        PROXY(pythonic::numpy, min);

        template<class T, unsigned long N>
            T sum(core::ndarray<T,N> const& array)
            {
                core::ndarray_flat_const<T,N> iter(array);
                return std::accumulate(iter.begin(), iter.end(), 0);
            }

        template<class T, unsigned long N>
            typename std::remove_reference<typename core::ndarray_helper<T,N>::result_type>::type sum( core::ndarray<T,N> const& array, long axis)
            {
                return axis_helper<T,N>::axis_sum(array, axis);
            }

        PROXY(pythonic::numpy, sum);

        template<class T, unsigned long N>
            bool all(core::ndarray<T,N> const& array)
            {

                core::ndarray_flat_const<T,N> iter(array);
                return pythonic::__builtin__::all(iter);
            }

        PROXY(pythonic::numpy, all);

        template<class T, unsigned long N, class... C>
            core::ndarray<T,N> apply_transpose(core::ndarray<T,N> const & a, long l[N])
            {
                std::array<long,N> shp;
                for(unsigned long i=0; i<N; i++)
                {    
                    shp[i] = (*a.shape)[l[i]];
                }

                core::ndarray<T,N> new_array(shp);

                std::array<long, N> new_strides;
                new_strides[N-1] = 1;
                std::transform(new_strides.rbegin(), new_strides.rend() -1, new_array.shape->rbegin(), new_strides.rbegin() + 1, std::multiplies<long>());

                std::array<long, N> old_strides;
                old_strides[N-1] = 1;
                std::transform(old_strides.rbegin(), old_strides.rend() -1, a.shape->rbegin(), old_strides.rbegin() + 1, std::multiplies<long>());

                core::ndarray_flat_const<T,N> iter(a);
                long i = 0;
                long offset = 0;
                for(auto val= iter.begin(); val!=iter.end(); val++, i++)
                {
                    offset = 0;
                    for(unsigned long s=0; s<N; s++)
                        offset += ((long)(i/old_strides[l[s]])%(*a.shape)[l[s]])*new_strides[s];

                    *(new_array.data->data + offset) = *val;
                }

                return new_array;
            }

        template<unsigned long N>
            struct transpose_tuple
            {
                template<typename... T, typename... S, class type>
                    static core::ndarray<type, sizeof...(T)> builder(core::ndarray<type, sizeof...(T)> const& a, std::tuple<T...> const& t, S... s)
                    {
                        long val = std::get<N-1>(t);
                        if(val>=sizeof...(T))
                            throw __builtin__::ValueError("invalid axis for this array");
                        return transpose_tuple<N-1>::builder(a, t, val, s...);
                    }
            };

        template<>
            struct transpose_tuple<0>
            {
                template<typename... T, typename... S, class type>
                    static core::ndarray<type, sizeof...(T)> builder(core::ndarray<type, sizeof...(T)> const& a, std::tuple<T...> const& t, S... s)
                    {
                        long axes[sizeof...(T)] = {s...};
                        return apply_transpose(a, axes);
                    }
            };

        template<class T, unsigned long N, class... C>
            core::ndarray<T,N> transpose(core::ndarray<T,N> const & a, std::tuple<C...> const& t)
            {
                if(sizeof...(C) != N)
                    throw __builtin__::ValueError("axes don't match array");

                long val = std::get<sizeof...(C)-1>(t);
                if(val>=N)
                    throw __builtin__::ValueError("invalid axis for this array");

                return transpose_tuple<sizeof...(C)-1>::builder(a, t, val);
            }

        template<class T, unsigned long N>
            core::ndarray<T,N> transpose(core::ndarray<T,N> const & a)
            {
                long t[N];
                for(unsigned long i = 0; i<N; i++)
                    t[N-1-i] = i;
                return apply_transpose(a, t);
            }

        PROXY(pythonic::numpy, transpose);

#define NP_PROXY(name)\
        using nt2::name;\
        using pythonic::core::name;\
        PROXY(pythonic::numpy, name)
#define NP_PROXY_ALIAS(name, alias)\
        ALIAS(alias, name)\
        using pythonic::core::name;\
        PROXY(pythonic::numpy, name)
#define NP_PROXY_OP(name)\
        using pythonic::numpy_expr::ops::name;\
        using pythonic::core::name;\
        PROXY(pythonic::numpy, name)


        NP_PROXY_ALIAS(absolute, nt2::abs);

        NP_PROXY_OP(add);

        NP_PROXY_ALIAS(arccos, nt2::acos);

        NP_PROXY_ALIAS(arccosh, nt2::acosh);

        NP_PROXY_ALIAS(arcsin, nt2::asin);

        NP_PROXY_ALIAS(arcsinh, nt2::asinh);

        NP_PROXY_ALIAS(arctan, nt2::atan);

        NP_PROXY_ALIAS(arctan2, nt2::atan2);

        NP_PROXY_ALIAS(arctanh, nt2::atanh);

        NP_PROXY_OP(bitwise_and);

        NP_PROXY_OP(bitwise_not);

        NP_PROXY_OP(bitwise_or);

        NP_PROXY_OP(bitwise_xor);

        NP_PROXY(ceil);

        // TODO
        // using pythonic::math::conj;
        // NP_PROXY(conj);
        //
        // using pythonic::math::conjugate;
        // NP_PROXY(conjugate);

        NP_PROXY(copysign);

        NP_PROXY(cos);

        NP_PROXY(cosh);

        NP_PROXY_ALIAS(deg2rad, nt2::inrad);

        NP_PROXY_ALIAS(degrees, nt2::indeg);

        NP_PROXY_OP(divide);

        NP_PROXY_OP(equal);

        NP_PROXY(exp);

        NP_PROXY(expm1);

        NP_PROXY_ALIAS(fabs, nt2::abs);

        NP_PROXY(floor);

        NP_PROXY_ALIAS(floor_divide, nt2::divfloor);

        NP_PROXY_ALIAS(fmax, nt2::max);

        NP_PROXY_ALIAS(fmin, nt2::min);

        NP_PROXY_ALIAS(fmod, nt2::mod);

        // NP_PROXY(frexp); // TODO

        NP_PROXY_OP(greater);

        NP_PROXY_OP(greater_equal);

        NP_PROXY(hypot);

        NP_PROXY_ALIAS(invert, pythonic::numpy_expr::ops::bitwise_not); 

        NP_PROXY_ALIAS(isfinite, nt2::is_finite);

        NP_PROXY_ALIAS(isinf, nt2::is_inf);

        NP_PROXY_ALIAS(isnan, nt2::is_nan);

        NP_PROXY(ldexp);

        NP_PROXY_OP(left_shift);

        NP_PROXY_OP(less);

        NP_PROXY_OP(less_equal);

        NP_PROXY(log10);

        NP_PROXY(log1p);

        NP_PROXY(log2);

        NP_PROXY_ALIAS(logaddexp, pythonic::numpy_expr::ops::logaddexp);

        NP_PROXY_ALIAS(logaddexp2, pythonic::numpy_expr::ops::logaddexp2);

        NP_PROXY_OP(logical_and);

        NP_PROXY_OP(logical_not);

        NP_PROXY_OP(logical_or);

        NP_PROXY_OP(logical_xor);

        NP_PROXY_ALIAS(maximum, nt2::max);

        NP_PROXY_ALIAS(minimum, nt2::min);

        NP_PROXY(mod);

        NP_PROXY_OP(multiply);

        NP_PROXY_OP(negative);

        NP_PROXY(nextafter);

        NP_PROXY_OP(not_equal);

        NP_PROXY_ALIAS(power, nt2::pow);

        NP_PROXY_ALIAS(rad2deg, nt2::indeg);

        NP_PROXY_ALIAS(radians, nt2::inrad);

        NP_PROXY(sin);

        NP_PROXY(tan);

#undef NP_PROXY
#undef NAMED_OPERATOR
#undef NAMED_UOPERATOR
    }
}

#endif

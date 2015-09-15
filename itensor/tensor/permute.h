//
// Distributed under the ITensor Library License, Version 1.1.
//    (See accompanying LICENSE file.)
//
#ifndef __ITENSOR_PERMUTE_H
#define __ITENSOR_PERMUTE_H

#include <array>
#include "itensor/tensor/ten.h"
#include "itensor/tensor/permutation.h"
#include "itensor/util/safe_ptr.h"
#include "itensor/detail/gcounter.h"

namespace itensor {

template<typename R1, typename R2>
void 
do_permute(TenRefc<R1> const& from, 
           Permutation  const& P, 
           TenRef<R2>  const& to);

template<typename R>
void 
do_permute(TenRefc<R> const& from, 
           Permutation const& P, 
           Tensor& to);

template<typename R>
Tensor
do_permute(TenRefc<R> const& from, 
           Permutation const& P);

//Callable is any function func(Real& x, Real y)
//default is func = [](Real& x, Real y) { x = y; };
template<typename R1, typename R2, typename Callable>
void 
do_permute(TenRefc<R1> from, 
           Permutation const& P, 
           TenRef<R2> to,
           Callable const& func);

template<typename R1, typename R2, typename Callable>
void 
do_permute(TenRefc<R1> const& from, 
           Label const& fL, 
           TenRef<R2> const& to,
           Label const& tL, 
           Callable const& func);

///
/// Implementations
///


template<typename R1, typename R2, typename Callable>
void 
do_permute(TenRefc<R1> from, 
           Permutation const& P, 
           TenRef<R2> to,
           Callable const& func)
    {
    using size_type = decltype(P.size());
    auto r = P.size();
#ifdef DEBUG
    if(r != size_type(from.r())) throw std::runtime_error("Mismatched Permutation size in do_permute");
    if(to.r() != from.r()) throw std::runtime_error("Mismatched tensor ranks in do_permute");
    if(to.size() != from.size()) throw std::runtime_error("Mismatched storage sizes in do_permute");
    for(decltype(r) j = 0; j < r; ++j)
        {
        if(to.extent(P.dest(j)) != from.extent(j))
            throw std::runtime_error("Incompatible extents in do_permute");
        }
#endif

    if(r == 0)
        {
        func(*to.data(),*from.data());
        return;
        }

    //find largest index of from,
    //size "bigsize" and position "bigind"
    size_type bigind = 0, 
              bigsize = from.extent(0);
    for(decltype(r) j = 1; j < r; ++j)
        if(bigsize < size_type(from.extent(j)))
            {
            bigsize = from.extent(j); 
            bigind = j;
            }

    auto stepfrom = from.stride(bigind);
    auto stepto = to.stride(P.dest(bigind));

    auto c = detail::GCounter(r);
    for(decltype(r) i = 0; i < r; ++i)
        c.setRange(i,0,from.extent(i)-1);
    //Leave bigind fixed to zero, will
    //increment manually in the loop below
    c.setRange(bigind,0,0);

    auto ti = Label(r);
    for(; c.notDone(); ++c)
        {
        for(decltype(r) j = 0; j < r; ++j)
            ti[P.dest(j)] = c[j];

        //effectively pto = to.data() + offset(to,ti);
        auto pto = MAKE_SAFE_PTR3(to.data(),offset(to,ti),to.size());
        //effectively pfrom = from.data() + offset(from,c.i);
        auto pfrom = MAKE_SAFE_PTR3(from.data(),offset(from,c.i),from.size());
        for(decltype(bigsize) b = 0; b < bigsize; ++b)
            {
            //func defaults to (*pto = *pfrom) but can also 
            //be operations such as (*pto += *pfrom)
            func(*pto,*pfrom);
            pto += stepto;
            pfrom += stepfrom;
            }
        }
    }

namespace detail {
template<typename T>
void 
assign(T& r1, T r2) { r1 = r2; }
template<typename T>
void
plusEq(T& r1, T r2) { r1 += r2; }
}

template<typename R1, typename R2>
void 
do_permute(TenRefc<R1> const& from, 
           Permutation const& P, 
           TenRef<R2> const& to)
    {
    do_permute(from,P,to,detail::assign<Real>);
    }

template<typename R>
void 
do_permute(TenRefc<R> const& from, 
           Permutation const& P, 
           Tensor& to)
    {
    do_permute(from,P,makeRef(to));
    }

//template<typename R>
//Tensor
//do_permute(RTenRefc<R> const& from, 
//        Permutation const& P)
//    {
//    return Tensor(do_permute(from,P));
//    }

template<typename R1, typename R2, typename Callable>
void 
do_permute(TenRefc<R1> const& from, 
           Label const& fL, 
           TenRef<R2> const& to,
           Label const& tL, 
           Callable const& func)
    {
#ifdef DEBUG
    if(fL.size() != tL.size()) throw std::runtime_error("Mismatched sizes in do_permute");
#endif
    if(fL.empty())
        {
        *to.data() = *from.data();
        return;
        }
    do_permute(from,calcPerm(fL,tL),to,func);
    }

template<typename R1, typename R2>
void 
do_permute(TenRefc<R1> const& from, 
           Label const& fL, 
           TenRef<R2> const& to,
           Label const& tL)
    {
    do_permute(from,fL,to,tL,detail::assign<Real>);
    }

} //namespace itensor

#endif
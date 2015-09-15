//
// Distributed under the ITensor Library License, Version 1.2.
//    (See accompanying LICENSE file.)
//
#ifndef __ITENSOR_SLICETEN_H_
#define __ITENSOR_SLICETEN_H_

#include "itensor/util/count.h"
#include "itensor/tensor/ten.h"
#include "itensor/tensor/slicemat.h"
#include "itensor/tensor/slicerange.h"

namespace itensor {

template<typename Ten_, typename C1, typename C2>
ref_type<Ten_>
subTensor(Ten_ && T,
          C1 const& start,
          C2 const& stop);

template<typename Ten_>
ref_type<Ten_>
subIndex(Ten_ && T,
         size_t ind,
         size_t start,
         size_t stop);

//group contiguous indices
template<typename Ten_>
ref_type<Ten_>
groupInds(Ten_ && T,
          size_t istart,
          size_t iend);

//group non-contiguous indices
//allocates a new Tensor to hold result
template<typename Ten_, typename Inds_>
Tensor
groupInds(Ten_      && T,
          Inds_ const& inds);

template<typename Ten_, typename Perm_>
ref_type<Ten_>
permute(Ten_  const& t,
        Perm_ const& P);


///
/// Implementations
/// 

template<typename Ten_, typename C1, typename C2>
auto
subTensor(Ten_ && T,
          C1 const& start,
          C2 const& stop) -> ref_type<Ten_>
    {
    static_assert(!std::is_same<Ten_&&,Tensor&&>::value,"Cannot pass temp/rvalue Tensor to subTensor");
    static_assert(!std::is_same<Ten_&&,Vector&&>::value,"Cannot pass temp/rvalue Vector to subTensor");
    static_assert(!std::is_same<Ten_&&,Matrix&&>::value,"Cannot pass temp/rvalue Matrix to subTensor");
    using range_type = decltype(T.range());
    using stop_type = decltype(*stop.begin());
    auto r = T.r();
#ifdef DEBUG
    if(r != decltype(r)(start.size())) throw std::runtime_error("subTensor: wrong size of start");
    if(r != decltype(r)(stop.size()))  throw std::runtime_error("subTensor: wrong size of stop");
    auto st_ = start.begin();
    auto sp_ = stop.begin();
    for(decltype(r) j = 0; j < r; ++j, ++st_, ++sp_)
        {
        if(*sp_ > stop_type(T.extent(j))) throw std::runtime_error("subTensor: stop value too large");
        if(*st_ >= *sp_)    throw std::runtime_error("subTensor: start value >= stop value");
        }
#endif
    size_t offset = 0;
    auto rb = RangeBuilderT<range_type>(r);
    auto st = start.begin();
    auto sp = stop.begin();
    for(decltype(r) j = 0; j < r; ++j, ++st, ++sp) 
        {
        offset += T.stride(j) * (*st);
        rb.setIndStr(j,(*sp)-(*st),T.stride(j));
        }
    return makeRef(T.store()+offset,rb.build());
    }

template<typename Ten_>
auto
subIndex(Ten_ && T,
         size_t ind,
         size_t start,
         size_t stop) -> ref_type<Ten_>
    {
    static_assert(!std::is_same<Ten_&&,Tensor&&>::value,"Cannot pass temp/rvalue Tensor to subIndex");
    static_assert(!std::is_same<Ten_&&,Vector&&>::value,"Cannot pass temp/rvalue Vector to subIndex");
    static_assert(!std::is_same<Ten_&&,Matrix&&>::value,"Cannot pass temp/rvalue Matrix to subIndex");
#ifdef DEBUG
    if(ind >= size_t(T.r())) throw std::runtime_error("subIndex: index out of range");
#endif
    auto R = T.range();
    R[ind].ind = stop-start;
    return makeRef(T.store()+T.stride(ind)*start,std::move(R));
    }


template<typename Ten_>
auto
groupInds(Ten_ && T,
          size_t istart,
          size_t iend) -> ref_type<Ten_>
    {
    return makeRef(T.store(),groupIndsRange(T.range(),istart,iend));
    }

template<typename Ten_, typename Inds_>
auto
groupInds(Ten_      && T,
          Inds_ const& inds) -> Tensor
    {
    //Does permute followed by contiguous groupInds; returns a Tensor
    using value_t = decltype(inds[0]);
    auto r = T.r();
    auto P = Label(r);
    auto inds_has = [&inds](value_t j) -> long
        { 
        for(auto n : index(inds)) if(j==inds[n]) return true;
        return false;
        };
    size_t tofront = 0,
           toback = inds.size();
    for(decltype(r) j = 0; j < r; ++j)
        {
        if(inds_has(j)) P[j] = tofront++;
        else            P[j] = toback++;
        }
    auto PT = Tensor{permute(T,P)};
    if(inds.size() <= 1) return PT;
    return Tensor{std::move(PT.store()),groupIndsRange(PT.range(),0,inds.size())};
    }

template<typename Ten_, typename Perm_>
auto
permute(Ten_  && t,
        Perm_ const& P) -> ref_type<Ten_>
    {
    return makeRef(t.store(),permuteRange(t.range(),P));
    }


} //namespace itensor

#endif
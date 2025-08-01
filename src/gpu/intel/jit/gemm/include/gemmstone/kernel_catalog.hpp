/*******************************************************************************
* Copyright 2022-2025 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef GEMMSTONE_GUARD_KERNEL_CATALOG_HPP
#define GEMMSTONE_GUARD_KERNEL_CATALOG_HPP

#include <string>
#include <tuple>
#include <array>
#include <utility>

#include "gemmstone/config.hpp"
#include "gemmstone/driver_info.hpp"

GEMMSTONE_NAMESPACE_START

namespace kcatalog {

// There are two versions of the kernel catalog structures:
//   - a mutable version, used inside of ktool for easier modification
//   - an immutable vesion, which is an aggregate type and used when the catalog
//       is loaded.
using string = const char *;
#define DEFAULT(val)
#define DEFAULT3(v1,v2,v3)

struct Restrictions {
    int steppingMin        DEFAULT(-1);             // If >= 0, minimum supported stepping (inclusive)
    int steppingMax        DEFAULT(-1);             // If >= 0, maximum supported stepping (exclusive)
    int acceptSizesMin[3]  DEFAULT3(-1, -1, -1);    // m/n/k ranges where kernel always accepted.
    int acceptSizesMax[3]  DEFAULT3(-1, -1, -1);    //   (see kernel_evaluator.cpp for more details)
    int allowedSizesMin[3] DEFAULT3(-1, -1, -1);    // m/n/k ranges outside of which kernel always rejected.
    int allowedSizesMax[3] DEFAULT3(-1, -1, -1);
    int alignment[3]       DEFAULT3(1, 1, 1);       // A/B/C alignment requirements.
    string tags;                                    // see RestrictionTags for entries.

};

enum RestrictionTags : char {
    ReqDisabled = '@',
    ReqAlignFallback = '#',
    ReqBlock2DA = 'A',       ReqNoBlock2DA = 'a',
    ReqBlock2DB = 'B',       ReqNoBlock2DB = 'b',
    ReqBlock2DC = 'C',       ReqNoBlock2DC = 'c',
    Req64BitA = 'X',         ReqNo64BitA = 'x',
    Req64BitB = 'Y',         ReqNo64BitB = 'y',
    Req64BitC = 'Z',         ReqNo64BitC = 'z',
    ReqBatch = 'V',          ReqNoBatch = 'v',
    ReqBatchMultiDim = 'W',  ReqNoBatchMultiDim = 'w',
    ReqSumA = 'Q',           ReqNoSumA = 'q',
    ReqSumB = 'P',           ReqNoSumB = 'p',
    ReqABOffset = 'R',       ReqNoABOffset = 'r',
    ReqOffsetMultiDim = 'O', ReqNoOffsetMultiDim = 'o',
    ReqSystolic = 'I',       ReqNoSystolic = 'i',
    ReqIntegrated = 'H',     ReqNoIntegrated = 'h',
    ReqBatchN = 'N',         ReqNoBatchN = 'n',
    ReqCustom1 = 'D',        ReqNoCustom1 = 'd',
    ReqXe2Block2D = 'G',     ReqNoXe2Block2D = 'g',
};

enum HWTags : char {
    HWTagGen12LP = 'C',
    HWTagXeHPG = 'E',
    HWTagXeHPC = 'F',
    HWTagXe2 = 'G',
    HWTagXe3 = 'H',
};

struct Selector {
    char hw;    // see HWTags for entries
    string kernelType;
    string precisions[3];
    string layouts[3];

    friend bool operator<(const Selector &sel1, const Selector &sel2) {
        auto tupleize = [](const Selector &sel) {
            return std::make_tuple(sel.hw,
                                   sel.precisions[0][0] & 0x1F, sel.precisions[1][0] & 0x1F,
                                   sel.layouts[0][0], sel.layouts[1][0]);
        };
        return tupleize(sel1) < tupleize(sel2);
    };
    friend bool operator>(const Selector &sel1, const Selector &sel2) { return sel2 < sel1; }
    friend bool operator<=(const Selector &sel1, const Selector &sel2) { return !(sel2 < sel1); }
    friend bool operator>=(const Selector &sel1, const Selector &sel2) { return !(sel1 < sel2); }

    std::string str(const int (&alignments)[3]) const {
        std::string result{hw};
        result.append(" ");
        result.append(kernelType);
        result.append(" ");
        for (auto &p : precisions) {
            result.append(p);
        }
        result.append(" ");
        for (int i = 0; i < 3; i++) {
            result.append(layouts[i]);
            if (alignments[i] != 1) {
                result.append("@");
                result.append(std::to_string(alignments[i]));
            }
        }
        return result;
    }
};

enum : int {
    // Model 'W' parameters
    ParamWPriority = 0,
    ParamWCount,

    // Model 'S' parameters
    ParamS_Cm0 = 0,     // Minimum constant overhead, beta = 0
    ParamS_Cm1,         // Minimum constant overhead, beta = 1
    ParamS_C00,         // Overhead per partial wave, constant coefficient, beta = 0
    ParamS_C01,         // Overhead per partial wave, constant coefficient, beta = 1
    ParamS_C10,         // Overhead per partial wave, linear coefficient, beta = 0
    ParamS_C11,         // Overhead per partial wave, linear coefficient, beta = 1
    ParamS_Ma,          // A per-element load cost
    ParamS_Mb,          // B per-element load cost
    ParamS_Ef,          // Peak efficiency, full waves
    ParamS_Ep0,         // Peak efficiency, partial wave, constant coefficient
    ParamS_Ep1,         // Peak efficiency, partial wave, linear coefficient
    ParamS_Em,          // Load balancing weight factor (0 = ignore load balancing, 1 = full weight for load balancing term)
    ParamS_Fp,          // Max sustained frequency ratio nominal freq/actual freq.
    ParamS_Fr0,         // FMA count at which frequency starts dropping.
    ParamS_Fr1,         // FMA count at which frequency stops dropping.
    ParamSCount,

    // Model 'E' parameters
    ParamE_C0 = 0,      // Constant overhead per partial wave, constant coefficient
    ParamE_C1,          // Constant overhead per partial wave, linear coefficient
    ParamE_Ck0,         // Per-wgK overhead per partial wave, constant coefficient
    ParamE_Ck1,         // Per-wgK overhead per partial wave, linear coefficient
    ParamE_Cb0,         // Fused beta overhead per partial wave, constant coefficient
    ParamE_Cb1,         // Fused beta overhead per partial wave, linear coefficient
    ParamE_Ma,          // A per-element load cost
    ParamE_Mb,          // B per-element load cost
    ParamE_Mc,          // C per-element update cost (store/atomic add only)
    ParamE_Mcu,         // C per-element update cost (load + store)
    ParamE_Ef,          // Peak efficiency, full waves
    ParamE_Ep0,         // Peak efficiency, partial wave, constant coefficient
    ParamE_Ep1,         // Peak efficiency, partial wave, linear coefficient
    ParamE_Em,          // Load balancing weight factor (0 = ignore load balancing, 1 = full weight for load balancing term)
    ParamE_Fp,          // Max sustained frequency ratio nominal freq/actual freq.
    ParamE_Fr0,         // FMA count at which frequency starts dropping.
    ParamE_Fr1,         // FMA count at which frequency stops dropping.
    ParamECount,

    ParamE_Cp0 = ParamE_Cb1,    // Fused post-op overhead per partial wave, constant coefficient

    // Maximum possible parameter count
    MaxParamCount = ParamECount,
};

struct Model {
    char id;
    int paramCount;
    double params[MaxParamCount];
};

struct Entry {
    Selector selector;
    Restrictions restrictions;
    string strategy;
    CommonDriverInfo driverInfo;
    Model model;

    friend bool operator<(const Entry &e1, const Entry &e2)   { return e1.selector <  e2.selector; }
    friend bool operator>(const Entry &e1, const Entry &e2)   { return e1.selector >  e2.selector; }
    friend bool operator<=(const Entry &e1, const Entry &e2)  { return e1.selector <= e2.selector; }
    friend bool operator>=(const Entry &e1, const Entry &e2)  { return e1.selector >= e2.selector; }
    friend bool operator<(const Entry &e, const Selector &s)  { return e.selector <  s; }
    friend bool operator>(const Entry &e, const Selector &s)  { return e.selector >  s; }
    friend bool operator<=(const Entry &e, const Selector &s) { return e.selector <= s; }
    friend bool operator>=(const Entry &e, const Selector &s) { return e.selector >= s; }
    friend bool operator<(const Selector &s, const Entry &e)  { return s <  e.selector; }
    friend bool operator>(const Selector &s, const Entry &e)  { return s >  e.selector; }
    friend bool operator<=(const Selector &s, const Entry &e) { return s <= e.selector; }
    friend bool operator>=(const Selector &s, const Entry &e) { return s >= e.selector; }

    std::string str() const {
        std::string result = selector.str(restrictions.alignment);
        result.append(" ");
        result.append(std::to_string(driverInfo.unroll[LoopM]));
        result.append(" ");
        result.append(std::to_string(driverInfo.unroll[LoopN]));
        result.append(" ");
        result.append(strategy);
        return result;
    }
};

struct Catalog {
    template <std::size_t N>
    Catalog(const std::array<Entry, N> &a): entryCount(N), entries(a.data()) {}
    int entryCount          DEFAULT(0);

    const Entry *entries;
};

#if __cplusplus >= 202002L && __cpp_lib_to_array >= 201907L
template <std::size_t N>
constexpr std::array<Entry, N> toArray(Entry (&&a)[N]) {
    return std::to_array(std::move(a));
}
#elif __cplusplus >= 201402L && __cpp_lib_integer_sequence >= 201304L
template <std::size_t N, std::size_t... I>
constexpr std::array<Entry, N>
toArrayImpl(Entry (&&a)[N], std::index_sequence<I...>) {
    return {{std::move(a[I])...}};
}
template <std::size_t N>
constexpr std::array<Entry, N> toArray(Entry (&&a)[N]) {
    return toArrayImpl(std::move(a), std::make_index_sequence<N>{});
}
#else
template <std::size_t N>
std::array<Entry, N> toArray(Entry (&&a)[N]) {
    std::array<Entry, N> ret;
    for(size_t i = 0; i < N; i++) {
        ret[i] = std::move(a[i]);
    }
    return ret;
}
#endif

} /* namespace kcatalog */

GEMMSTONE_NAMESPACE_END

#endif /* header guard */

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

#ifndef GPU_INTEL_JIT_IR_REDUCE_HPP
#define GPU_INTEL_JIT_IR_REDUCE_HPP

#include "gpu/intel/jit/ir/ir.hpp"
#include "gpu/intel/jit/ir/reorder.hpp"
#include "gpu/intel/jit/ir/tensor.hpp"

namespace dnnl {
namespace impl {
namespace gpu {
namespace intel {
namespace jit {

// Implements reduction of GRF buffer for given layout.
class reduce_t : public func_impl_t {
public:
    IR_DECL_TYPE(reduce_t)

    static func_t make(const layout_t &src_layout, const layout_t &dst_layout) {
        return func_t(new reduce_t(src_layout, dst_layout));
    }

    std::string str() const override {
        ostringstream_t oss;
        oss << "reduce[" << src_layout << ", " << dst_layout << "]";
        return oss.str();
    }

    IR_DEFINE_ARG_GET(dst_buf, 0)
    IR_DEFINE_ARG_GET(src_buf, 1)

    layout_t src_layout;
    layout_t dst_layout;

private:
    reduce_t(const layout_t &src_layout, const layout_t &dst_layout)
        : func_impl_t(_type_info())
        , src_layout(src_layout)
        , dst_layout(dst_layout) {}
};

stmt_t create_reduce_stmt(const layout_t &src, const layout_t &dst,
        const expr_t &src_buf, const expr_t &dst_buf,
        const tile_coord_t &_sub_tile_coord, uint32_t reduction_mask,
        bool drop_dims = true);

stmt_t create_reduce_stmt(const layout_t &src, const layout_t &dst,
        const expr_t &src_buf, const expr_t &dst_buf);

} // namespace jit
} // namespace intel
} // namespace gpu
} // namespace impl
} // namespace dnnl

#endif

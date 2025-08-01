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

#include <stdio.h>
#include <string>

#include "flex_rewrite.hpp"
#include "graph.hpp"
#include "graph_memory.hpp"
#include "parser.hpp"
#include "utils/parser.hpp"

namespace graph {

void check_correctness(const settings_t &s) {
    for_(const auto &i_in_shapes : s.in_shapes_vec)
    for_(const auto &i_op_attrs : s.op_attrs_vec)
    for_(const auto &i_expected_n_partition : s.expected_n_partition_vec)
    for_(const auto &i_fpmath_mode : s.fpmath_mode_vec)
    for_(const auto &i_op_kind_map : s.op_kind_map)
    for_(const auto &i_dt : s.dt)
    for_(const auto &i_dt_map : s.dt_map)
    for (const auto &i_mb : s.mb) {
        deserialized_graph_t dg;
        dg.load(locate_file(s.json_file));

        res_t res {};
        const auto &cpp_pstr = case_to_str(s.json_file, i_in_shapes, i_op_attrs,
                i_fpmath_mode, i_expected_n_partition, i_mb, i_dt, i_dt_map,
                i_op_kind_map);
        const char *pstr = cpp_pstr.c_str();

        // rewrite the graph
        flex_rewrite_t fw(i_in_shapes, i_op_attrs, i_fpmath_mode, i_mb, i_dt,
                i_dt_map, i_op_kind_map);
        // TODO(zhitao): use const fw as the parameter of rewrite for the dg
        // instead, as all the states should the updated in the ctor.
        if (fw.rewrite(dg) != OK) {
            res.state = UNTESTED;
            res.reason = "Rewriting unsupported";
            parse_result(res, pstr);
            continue;
        }

        BENCHDNN_PRINT(7, "[INFO] Graph dump:\n%s\n", dg.get_string().c_str());
        const prb_t prb(dg, i_expected_n_partition);
        BENCHDNN_PRINT(1, "run: %s\n", pstr);

        // A timer for each test case.
        auto &tct = res.timer_map.get_timer(timer::names::test_case_timer);
        tct.start();
        doit(&prb, &res);
        tct.stamp();

        // Reset the memory size args for the graph after testing.
        reset_graph_mem_req();

        parse_result(res, pstr);
        if (has_bench_mode_bit(mode_bit_t::perf)) {
            perf_report_t pr(cpp_pstr, s.perf_template);
            pr.report(&res, pstr);
        }
    }
}

int bench(int argc, char **argv) {
    driver_name = "graph";
    using namespace parser;
    static settings_t s;
    static const settings_t def {};

    for (; argc > 0; --argc, ++argv) {
        const bool parsed_options = parse_bench_settings(argv[0])
                || parse_batch(bench, argv[0])
                || parse_dt(s.dt, s.dt_map, argv[0])
                || parse_input_shapes(s.in_shapes_vec, argv[0])
                || parse_op_attrs(s.op_attrs_vec, argv[0])
                || parse_op_kind(s.op_kind_map, argv[0])
                || parse_graph_expected_n_partitions(
                        s.expected_n_partition_vec, argv[0])
                || parse_graph_fpmath_mode(s.fpmath_mode_vec, argv[0])
                || parse_mb(s.mb, def.mb, argv[0]) || parse_reset(s, argv[0]);
        if (!parsed_options) {
            if (!parse_input_file(s.json_file, argv[0]))
                catch_unknown_options(argv[0]);
            check_correctness(s);
            flush_temp_memory();
        }
    }
    return OK;
}
} // namespace graph

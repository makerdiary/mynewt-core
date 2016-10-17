/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdio.h>
#include <string.h>

#include "syscfg/syscfg.h"
#include "os/os.h"
#include "testutil/testutil.h"

#include "fcb/fcb.h"
#include "fcb/../../src/fcb_priv.h"

#include "fcb_test.h"

struct fcb test_fcb;

struct flash_area test_fcb_area[] = {
    [0] = {
        .fa_device_id = 0,
        .fa_off = 0,
        .fa_size = 0x4000, /* 16K */
    },
    [1] = {
        .fa_device_id = 0,
        .fa_off = 0x4000,
        .fa_size = 0x4000
    },
    [2] = {
        .fa_device_id = 0,
        .fa_off = 0x8000,
        .fa_size = 0x4000
    },
    [3] = {
        .fa_device_id = 0,
        .fa_off = 0xc000,
        .fa_size = 0x4000
    }
};

void
fcb_test_wipe(void)
{
    int i;
    int rc;
    struct flash_area *fap;

    for (i = 0; i < sizeof(test_fcb_area) / sizeof(test_fcb_area[0]); i++) {
        fap = &test_fcb_area[i];
        rc = flash_area_erase(fap, 0, fap->fa_size);
        TEST_ASSERT(rc == 0);
    }
}

int
fcb_test_empty_walk_cb(struct fcb_entry *loc, void *arg)
{
    TEST_ASSERT(0);
    return 0;
}

uint8_t
fcb_test_append_data(int msg_len, int off)
{
    return (msg_len ^ off);
}

int
fcb_test_data_walk_cb(struct fcb_entry *loc, void *arg)
{
    uint16_t len;
    uint8_t test_data[128];
    int rc;
    int i;
    int *var_cnt = (int *)arg;

    len = loc->fe_data_len;

    TEST_ASSERT(len == *var_cnt);

    rc = flash_area_read(loc->fe_area, loc->fe_data_off, test_data, len);
    TEST_ASSERT(rc == 0);

    for (i = 0; i < len; i++) {
        TEST_ASSERT(test_data[i] == fcb_test_append_data(len, i));
    }
    (*var_cnt)++;
    return 0;
}

int
fcb_test_cnt_elems_cb(struct fcb_entry *loc, void *arg)
{
    struct append_arg *aa = (struct append_arg *)arg;
    int idx;

    idx = loc->fe_area - &test_fcb_area[0];
    aa->elem_cnts[idx]++;
    return 0;
}

TEST_CASE_DECL(fcb_test_len)
TEST_CASE_DECL(fcb_test_init)
TEST_CASE_DECL(fcb_test_empty_walk)
TEST_CASE_DECL(fcb_test_append)
TEST_CASE_DECL(fcb_test_append_too_big)
TEST_CASE_DECL(fcb_test_append_fill)
TEST_CASE_DECL(fcb_test_reset)
TEST_CASE_DECL(fcb_test_rotate)
TEST_CASE_DECL(fcb_test_multiple_scratch)

TEST_SUITE(fcb_test_all)
{
    fcb_test_len();
    fcb_test_init();
    fcb_test_empty_walk();
    fcb_test_append();
    fcb_test_append_too_big();
    fcb_test_append_fill();
    fcb_test_reset();
    fcb_test_rotate();
    fcb_test_multiple_scratch();
}

#if MYNEWT_VAL(SELFTEST)
int
main(int argc, char **argv)
{
    ts_config.ts_print_results = 1;
    tu_init();

    fcb_test_all();

    return tu_any_failed;
}
#endif

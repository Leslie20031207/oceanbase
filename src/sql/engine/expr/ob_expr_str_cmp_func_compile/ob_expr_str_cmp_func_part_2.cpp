/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#include "ob_expr_str_cmp_func_common.ipp"

namespace oceanbase
{
namespace sql
{
DEF_COMPILE_STR_FUNC_INIT(CS_TYPE_GBK_BIN, 6);
DEF_COMPILE_STR_FUNC_INIT(CS_TYPE_UTF16_UNICODE_CI, 7);
DEF_COMPILE_STR_FUNC_INIT(CS_TYPE_UTF8MB4_UNICODE_CI, 8);
} // end sql
} // end oceanbase
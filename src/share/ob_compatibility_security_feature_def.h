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

// define the feature list here whose behavior is different in MySQL5.7, MySQL8.0 or OB
// DEF_COMPAT_CONTROL_FEATURE(type, id, is_dynanmic, description, lastest_version, patch_versions ...)

#ifdef DEF_COMPAT_CONTROL_FEATURE
DEF_COMPAT_CONTROL_FEATURE(MYSQL_PRIV_ENHANCE, "add privilege check to some command",
    CLUSTER_VERSION_4_2_3_0)
#endif

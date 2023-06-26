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

#include "storage/multi_data_source/ob_mds_table_merge_dag_param.h"

using namespace oceanbase::common;

namespace oceanbase
{
namespace storage
{
namespace mds
{
ObMdsTableMergeDagParam::ObMdsTableMergeDagParam()
  : ls_id_(),
    tablet_id_(),
    flush_scn_(share::SCN::invalid_scn())
{
}
} // namespace mds
} // namespace storage
} // namespace oceanbase
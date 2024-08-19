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
 * This file contains implementation for st_pointn.
 */

#ifndef OCEANBASE_SQL_OB_EXPR_ST_POINTN_
#define OCEANBASE_SQL_OB_EXPR_ST_POINTN_

#include "sql/engine/expr/ob_expr_operator.h"
#include "observer/omt/ob_tenant_srs.h"
#include "lib/geo/ob_geo.h"

namespace oceanbase
{
namespace sql
{
class ObExprSTPointN : public ObFuncExprOperator
{
public:
  explicit ObExprSTPointN(common::ObIAllocator &alloc);
  virtual ~ObExprSTPointN() {}
  virtual int calc_result_type2(ObExprResType &type,
                                ObExprResType &type1,
                                ObExprResType &type2,
                                common::ObExprTypeCtx &type_ctx)
                                const override;
  static int eval_st_pointn(const ObExpr &expr, ObEvalCtx &ctx, ObDatum &res);
  virtual int cg_expr(ObExprCGCtx &expr_cg_ctx,
                      const ObRawExpr &raw_expr,
                      ObExpr &rt_expr) const override;
private:
  template<typename MLS, typename PT>
  static int get_sub_point(const ObGeometry *g,
                           const int N,
                           bool& is_null_result,
                           ObIAllocator &allocator,
                           ObGeometry *&sub_geo);
  DISALLOW_COPY_AND_ASSIGN(ObExprSTPointN);
};
} // sql
} // oceanbase
#endif // OCEANBASE_SQL_OB_EXPR_ST_POINTN_
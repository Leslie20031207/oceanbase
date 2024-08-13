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
 * This file contains implementation for eval_st_disjoint.
 */

#ifndef OCEANBASE_SQL_OB_EXPR_ST_DISJOINT_H_
#define OCEANBASE_SQL_OB_EXPR_ST_DISJOINT_H_

#include "sql/engine/expr/ob_expr_operator.h"
#include "sql/engine/expr/ob_geo_expr_utils.h"

namespace oceanbase
{
namespace sql
{
class ObExprSTDisjoint : public ObFuncExprOperator
{
public:
  explicit ObExprSTDisjoint(common::ObIAllocator &alloc);
  virtual ~ObExprSTDisjoint();
  virtual int calc_result_type2(ObExprResType &type,
                                ObExprResType &type1,
                                ObExprResType &type2,
                                common::ObExprTypeCtx &type_ctx) const override;
  static int eval_st_disjoint(const ObExpr &expr, ObEvalCtx &ctx, ObDatum &res);
  virtual int cg_expr(ObExprCGCtx &expr_cg_ctx,
                      const ObRawExpr &raw_expr,
                      ObExpr &rt_expr) const override;
  virtual bool need_rt_ctx() const override { return true; }
private:
  static int process_input_geometry(omt::ObSrsCacheGuard &srs_guard, const ObExpr &expr, ObEvalCtx &ctx, ObIAllocator &allocator, ObGeometry *&geo1,
      ObGeometry *&geo2, bool &is_null_res, const ObSrsItem *&srs);
  DISALLOW_COPY_AND_ASSIGN(ObExprSTDisjoint);
};
} // sql
} // oceanbase
#endif // OCEANBASE_SQL_OB_EXPR_ST_DISJOINT_H_
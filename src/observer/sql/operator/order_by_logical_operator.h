#pragma once

#include <memory>
#include <vector>

#include "sql/expr/expression.h"
#include "sql/operator/logical_operator.h"

/**
 * @brief order by 表示排序运算
 * @ingroup LogicalOperator
 * @details 从表中获取数据后，可能需要过滤，投影，连接等等。
 */
class OrderByLogicalOperator : public LogicalOperator
{
public:
  OrderByLogicalOperator(std::vector<std::unique_ptr<Expression>> &&expressions,
                         std::vector<bool> &&order_by_descs);
  virtual ~OrderByLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::ORDER_BY; }

  std::vector<std::unique_ptr<Expression>>       &expressions() { return expressions_; }
  std::vector<bool> &order_by_descs() { return order_by_descs_; }

private:
  std::vector<bool> order_by_descs_;
};
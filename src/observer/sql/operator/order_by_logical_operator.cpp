#include "sql/operator/order_by_logical_operator.h"

using namespace std;

OrderByLogicalOperator::OrderByLogicalOperator(vector<unique_ptr<Expression>> &&expressions,
                                               vector<bool> &&order_by_descs)
{
  expressions_ = std::move(expressions);
  order_by_descs_ = std::move(order_by_descs);
}
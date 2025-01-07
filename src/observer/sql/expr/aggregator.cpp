/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2024/05/29.
//

#include "sql/expr/aggregator.h"
#include "common/log/log.h"

RC CountAggregator::accumulate(const Value &value)//count实现
{
  if (!value.is_null())
  count_++;
  return RC::SUCCESS;
}

RC CountAggregator::evaluate(Value &result)
{
  result = Value(count_);
  return RC::SUCCESS;
}

RC SumAggregator::accumulate(const Value &value)//sum实现
{
  if (value.is_null()) {
    return RC::SUCCESS;
  }
  if (value_.attr_type() == AttrType::UNDEFINED) {
    value_ = value;
    return RC::SUCCESS;
  }
  
  ASSERT(value.attr_type() == value_.attr_type(), "type mismatch. value type: %s, value_.type: %s", 
        attr_type_to_string(value.attr_type()), attr_type_to_string(value_.attr_type()));
  
  Value::add(value, value_, value_);
  return RC::SUCCESS;
}

RC SumAggregator::evaluate(Value& result)
{
  result = value_;
  return RC::SUCCESS;
}

RC AvgAggregator::accumulate(const Value &value)//avg实现
{
  if (value.is_null()) {
    return RC::SUCCESS;
  }
  if (value_.attr_type() == AttrType::UNDEFINED) { // first value
    value_ = value;
    count_ = 1;
    return RC::SUCCESS;
  }
  // ASSERT(value.attr_type() == AttrType::FLOATS, "only float_type support divide");
  ASSERT(value.attr_type() == value_.attr_type(), "type mismatch. value type: %s, value_.type: %s", 
        attr_type_to_string(value.attr_type()), attr_type_to_string(value_.attr_type()));

  Value::add(value, value_, value_); // value_ += value
  count_ ++;
  return RC::SUCCESS;
}

RC AvgAggregator::evaluate(Value& result)
{
  if (count_ == 0) {
    result.set_null(true);
    return RC::SUCCESS;
  }
  result.set_type(AttrType::FLOATS);
  LOG_DEBUG("value_ = %s, count_ = %d", value_.to_string().c_str(), count_);
  Value::divide(value_, Value(count_), result); // only float_type support divide
  return RC::SUCCESS;
}

MaxMinAggregator::MaxMinAggregator(int32_t is_max) : is_max_(is_max) {}

RC MaxMinAggregator::accumulate(const Value &value)//maxmin实现
{
  if (value.is_null()) {
    return RC::SUCCESS;
  }
  if (value_.attr_type() == AttrType::UNDEFINED) {
    value_ = value;
    return RC::SUCCESS;
  }

  ASSERT(value_.attr_type() == value.attr_type(), "type mismatch. value type: %s, value_.type: %s", 
        attr_type_to_string(value.attr_type()), attr_type_to_string(value_.attr_type()));

  value_ = value_.compare(value) == is_max_ ? value_ : value; 
  // compare return 1 if value_ > value, 0 if value_ == value, -1 if value_ < value
  return RC::SUCCESS;
}

RC MaxMinAggregator::evaluate(Value& result)
{
  result = value_;
  return RC::SUCCESS;
}
/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/date_type.h"
#include "common/value.h"

int DateType::compare(const Value &left, const Value &right) const
{

   date left_val  = left.get_date();
   date right_val = right.get_date();
  return common::compare_date((void *)&left_val, (void *)&right_val);
}


RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    default: return RC::UNIMPLEMENTED;
  }
  return RC::SUCCESS;
}

int DateType::cast_cost(AttrType type)
{
  if (type == AttrType::DATES) {
    return 0;
  }
  return INT32_MAX;
}

RC DateType::to_string(const Value &val, string &result) const
{
  unsigned year = 0, month = 0, day = 0;
  stringstream os;
      date t = val.get_date();
      year = (t >> 16);
      month = (t >> 8) & 0xff;
      day = t & 0xff;
      os << std::setw(4) << std::setfill('0') << year << "-";
      os << std::setw(2) << std::setfill('0') << month << "-";
      os << std::setw(2) << std::setfill('0') << day;
      result = os.str();
  return RC::SUCCESS;
}
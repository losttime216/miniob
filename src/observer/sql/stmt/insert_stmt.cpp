/* Copyright (c) 2021OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/insert_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

InsertStmt::InsertStmt(Table *table, const std::vector<std::vector<Value>> &items)
    : table_(table), items_(items)
{}

RC InsertStmt::create(Db *db, const InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();
  bool has_empty_item = false;
  for(auto &item : inserts.items) {
    if (item.empty()) has_empty_item = true;
  }

  if (nullptr == db || nullptr == table_name || inserts.items.empty() || has_empty_item) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, items_num=%d",
        db, table_name, static_cast<int>(inserts.items.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  for(auto &item : inserts.items) {
    // check the fields number
    const TableMeta &table_meta = table->table_meta();
    const int        field_num  = table_meta.field_num() - table_meta.sys_field_num();
    int item_size = static_cast<int>(item.size());
    if (field_num != item_size) {
      LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", item_size, field_num);
      return RC::SCHEMA_FIELD_MISSING;
    }
    // check fields type
    const int sys_field_num = table_meta.sys_field_num();
    for (int i = 0; i < item_size; i++) {
      const FieldMeta *field_meta = table_meta.field(i + sys_field_num);
      const AttrType field_type = field_meta->type();
      const AttrType item_type = item[i].attr_type();
      if (field_type != item_type) {  // TODO try to convert the value type to field type
        LOG_WARN("field type mismatch. table=%s, field=%s, field type=%d, item_type=%d",
            table_name, field_meta->name(), field_type, item_type);
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      if (item_type == AttrType::DATES && item[i].get_date() == 0) {
        return RC::VARIABLE_NOT_VALID;
    }
   }
  }
  // everything alright
  stmt = new InsertStmt(table, std::move(inserts.items));
  return RC::SUCCESS;
}

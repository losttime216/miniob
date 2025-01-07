#pragma once
#include "sql/parser/parse_defs.h"
#include "sql/operator/logical_operator.h"
/**
 * @brief 逻辑算子，用于执行update语句
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
    UpdateLogicalOperator(Table *table, Value *values, FieldMeta *field_meta);
    virtual ~UpdateLogicalOperator() = default;
    LogicalOperatorType type() const override   { return LogicalOperatorType::UPDATE; }
    Table       *table()    const   { return table_; }
    const Value *values()   const   { return values_; }
    Value       *values()           { return values_; }
    FieldMeta   *field_meta() const  { return field_meta_; }

private:
    Table *table_   = nullptr;
    Value *values_  = nullptr;
    FieldMeta *field_meta_ = nullptr;
};
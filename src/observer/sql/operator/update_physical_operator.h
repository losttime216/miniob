#pragma once
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse.h"
class Trx;
class UpdateStmt;
/**
 * @brief 物理算子，更新
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
    UpdatePhysicalOperator(Table *table, Value *values, FieldMeta *field_meta);
    
    virtual ~UpdatePhysicalOperator() = default;
    
    PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }
    
    RC open(Trx *trx) override;
    RC next() override;
    RC close() override;
    
    Tuple *current_tuple() override { return nullptr; }
private:
    Table               *table_ = nullptr;
    Trx                 *trx_ = nullptr;
    Value               *values_ = nullptr;
    FieldMeta           *field_meta_ = nullptr;
    std::vector<Record> records_;
};
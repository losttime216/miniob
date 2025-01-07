#include "sql/operator/update_physical_operator.h"
#include "sql/stmt/update_stmt.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
#include "common/log/log.h"
UpdatePhysicalOperator::UpdatePhysicalOperator(Table *table, Value *values, FieldMeta *field_meta)
    : table_(table), values_(values), field_meta_(field_meta)
{}
RC UpdatePhysicalOperator::open(Trx *trx)
{
    // 如果经过读表和筛选后，没有满足条件的 records 可供更新，则直接返回成功
    if (children_.empty()) { 
        return RC::SUCCESS;
    }
    std::unique_ptr<PhysicalOperator> &child = children_[0];
    RC rc = child->open(trx);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to open child operator: %s", strrc(rc));
        return rc;
    }
    trx_ = trx;
    while(OB_SUCC(rc = child->next())) {
        Tuple *tuple = child->current_tuple();
        if (nullptr == tuple) {
            LOG_WARN("failed to get current record: %s", strrc(rc));
            return rc;
        }
        RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
        Record &record = row_tuple->record();
        records_.emplace_back(std::move(record));
    }
    child->close();
    // 先收集记录再更新
    
    if (field_meta_->type() != values_->attr_type()) { // 类型不一致，需要转换，例如 str 转 date
        Value real_value;
        rc = Value::cast_to(*values_, field_meta_->type(), real_value);
        if (rc != RC::SUCCESS) {
            LOG_WARN("failed to cast value: %s", strrc(rc));
            return rc;
        }
        values_ = &real_value;
    }

    vector<Record> new_records;
    // 记录的有效性由事务来保证，如果事务不保证更新的有效性，那说明此事务类型不支持并发控制，比如 VacuousTrx
    for (Record &record : records_) {
        Record new_record;
        new_record.copy_data(record.data(), record.len());
        new_record.set_rid(record.rid());
        new_record.set_field(field_meta_->offset(), field_meta_->len(), const_cast<char*>(values_->data()));
        new_records.emplace_back(std::move(new_record));
        trx_->delete_record(table_, record); // 由于 unique index 的存在，需要先全部删除再插入，而不能删除一条就插入一条
    }
    for (Record &record : new_records) {
        rc = trx_->insert_record(table_, record);
        if (rc != RC::SUCCESS) {
            LOG_WARN("failed to update record: %s", strrc(rc));
            return rc;
        }
    }
    return RC::SUCCESS;
}
RC UpdatePhysicalOperator::next()   { return RC::RECORD_EOF; }
RC UpdatePhysicalOperator::close()  { return RC::SUCCESS; }
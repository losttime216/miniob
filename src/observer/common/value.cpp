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
// Created by WangYunlai on 2023/06/28.
//

#include "common/value.h"

#include <iomanip>
#include "common/lang/comparator.h"
#include "common/lang/exception.h"
#include "common/lang/sstream.h"
#include "common/lang/string.h"
#include "common/log/log.h"

Value::Value(int val) { set_int(val); }

Value::Value(float val) { set_float(val); }

Value::Value(bool val) { set_boolean(val); }

Value::Value(date val){ set_date(val);}

Value::Value(const char *s, int len /*= 0*/) { set_string(s, len); }

Value::Value(const Value &other)
{
  this->attr_type_ = other.attr_type_;
  this->length_    = other.length_;
  this->own_data_  = other.own_data_;
  this->is_null_   = other.is_null_;
  if (this->is_null_) {
    return;
  }
  switch (this->attr_type_) {
    case AttrType::CHARS: {
      set_string_from_other(other);
    } break;

    default: {
      this->value_ = other.value_;
    } break;
  }
}

Value::Value(Value &&other)
{
  this->attr_type_ = other.attr_type_;
  this->length_    = other.length_;
  this->own_data_  = other.own_data_;
  this->value_     = other.value_;
  this->is_null_   = other.is_null_;
  other.own_data_  = false;
  other.length_    = 0;
}

Value &Value::operator=(const Value &other)
{
  if (this == &other) {
    return *this;
  }
  reset();
  this->attr_type_ = other.attr_type_;
  this->length_    = other.length_;
  this->own_data_  = other.own_data_;
  this->is_null_   = other.is_null_;
  if (other.is_null_) {
    return *this;
  }
  switch (this->attr_type_) {
    case AttrType::CHARS: {
      set_string_from_other(other);
    } break;

    default: {
      this->value_ = other.value_;
    } break;
  }
  return *this;
}

Value &Value::operator=(Value &&other)
{
  if (this == &other) {
    return *this;
  }
  reset();
  this->attr_type_ = other.attr_type_;
  this->length_    = other.length_;
  this->own_data_  = other.own_data_;
  this->value_     = other.value_;
  this->is_null_   = other.is_null_;
  other.own_data_  = false;
  other.length_    = 0;
  return *this;
}

void Value::reset()
{
  switch (attr_type_) {
    case AttrType::CHARS:
      if (own_data_ && value_.pointer_value_ != nullptr) {
        delete[] value_.pointer_value_;
        value_.pointer_value_ = nullptr;
      }
      break;
    default: break;
  }

  attr_type_ = AttrType::UNDEFINED;
  is_null_   = false;
  length_    = 0;
  own_data_  = false;
}

void Value::set_data(char *data, int length, bool is_not_null)
{
  if (!is_not_null) {
    is_null_ = static_cast<bool>(data[length]);
    if(is_null_) {
      return;
    }
  }
  else {
    is_null_ = false;
  }
  switch (attr_type_) {
    case AttrType::CHARS: {
      set_string(data, length);
    } break;
    case AttrType::INTS: {
      value_.int_value_ = *(int *)data;
      length_           = length;
    } break;
    case AttrType::FLOATS: {
      value_.float_value_ = *(float *)data;
      length_             = length;
    } break;
    case AttrType::DATES: {
      value_.date_value_ = *(date *)data;
      length_ = length;
    } break;
    case AttrType::BOOLEANS: {
      value_.bool_value_ = *(int *)data != 0;
      length_            = length;
    } break;
    default: {
      LOG_WARN("unknown data type: %d", attr_type_);
    } break;
  }
}

void Value::set_int(int val)
{
  reset();
  attr_type_        = AttrType::INTS;
  value_.int_value_ = val;
  length_           = sizeof(val);
  is_null_          = false;
}

void Value::set_float(float val)
{
  reset();
  attr_type_          = AttrType::FLOATS;
  value_.float_value_ = val;
  length_             = sizeof(val);
  is_null_            = false;
}
void Value::set_date(date val)
{
  attr_type_ = AttrType::DATES;
  value_.date_value_ = val;
  length_ = sizeof(val);
  is_null_            = false;
}
void Value::set_boolean(bool val)
{
  reset();
  attr_type_         = AttrType::BOOLEANS;
  value_.bool_value_ = val;
  length_            = sizeof(val);
  is_null_            = false;
}

void Value::set_string(const char *s, int len /*= 0*/)
{
  reset();
  attr_type_ = AttrType::CHARS;
  if (s == nullptr) {
    value_.pointer_value_ = nullptr;
    is_null_              = true;
    length_               = 0;
  } else {
    own_data_ = true;
    if (len > 0) {
      len = strnlen(s, len);
    } else {
      len = strlen(s);
    }
    value_.pointer_value_ = new char[len + 1];
    length_               = len;
    memcpy(value_.pointer_value_, s, len);
    value_.pointer_value_[len] = '\0';
    is_null_                  = false;
  }
}

void Value::set_value(const Value &value)
{
  if(value.is_null_) {
    this->attr_type_ = value.attr_type_;
    this->is_null_ = true;
    return;
  }
  switch (value.attr_type_) {
    case AttrType::INTS: {
      set_int(value.get_int());
    } break;
    case AttrType::FLOATS: {
      set_float(value.get_float());
    } break;
    case AttrType::CHARS: {
      set_string(value.get_string().c_str());
    } break;
    case AttrType::DATES: {
      set_date(value.get_date());
    } break;
    case AttrType::BOOLEANS: {
      set_boolean(value.get_boolean());
    } break;
    default: {
      ASSERT(false, "got an invalid value type");
    } break;
  }
}

void Value::set_string_from_other(const Value &other)
{
  ASSERT(attr_type_ == AttrType::CHARS, "attr type is not CHARS");
  if (other.is_null_) {
    free(this->value_.pointer_value_);
    this->value_.pointer_value_ = nullptr;
    this->length_               = 0;
    this->is_null_              = true;
    return;
  }
  if (own_data_ && other.value_.pointer_value_ != nullptr && length_ != 0) {
    this->value_.pointer_value_ = new char[this->length_ + 1];
    memcpy(this->value_.pointer_value_, other.value_.pointer_value_, this->length_);
    this->value_.pointer_value_[this->length_] = '\0';
  }
}

const char *Value::data() const
{
  if (is_null_) {
    return nullptr;
  }
  switch (attr_type_) {
    case AttrType::CHARS: {
      return value_.pointer_value_;
    } break;
    default: {
      return (const char *)&value_;
    } break;
  }
}

string Value::to_string() const
{
  string res;
  if(this->attr_type_!=AttrType::DATES){
  if (is_null_) { 
    return "NULL";
  }
  RC     rc = DataType::type_instance(this->attr_type_)->to_string(*this, res);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to convert value to string. type=%s", attr_type_to_string(this->attr_type_));
    return "";
  }
  return res;
  }
  else{
    if (is_null_) { 
    return "NULL";
   }
    unsigned year = 0, month = 0, day = 0;
    stringstream os;
      date t = value_.date_value_;
      year = (t >> 16);
      month = (t >> 8) & 0xff;
      day = t & 0xff;
      os << std::setw(4) << std::setfill('0') << year << "-";
      os << std::setw(2) << std::setfill('0') << month << "-";
      os << std::setw(2) << std::setfill('0') << day;
      res = os.str();
      return res;
  }
}

int Value::compare(const Value &other) const { 
  if(this->attr_type_!=AttrType::DATES){
  return DataType::type_instance(this->attr_type_)->compare(*this, other); }
  else{
    return common::compare_date((void *)&this->value_.date_value_, (void *)&other.value_.date_value_);
  }
}

int Value::get_int() const
{
  if (is_null_) {
    LOG_WARN("can't get null value as int");
  }
  switch (attr_type_) {
    case AttrType::CHARS: {
      try {
        return (int)(stol(value_.pointer_value_));
      } catch (exception const &ex) {
        LOG_TRACE("failed to convert string to number. s=%s, ex=%s", value_.pointer_value_, ex.what());
        return 0;
      }
    }
    case AttrType::INTS: {
      return value_.int_value_;
    }
    case AttrType::FLOATS: {
      return (int)(value_.float_value_);
    }
    case AttrType::BOOLEANS: {
      return (int)(value_.bool_value_);
    }
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return 0;
    }
  }
  return 0;
}

float Value::get_float() const
{
  if (is_null_) {
    LOG_WARN("can't get null value as float");
  }
  switch (attr_type_) {
    case AttrType::CHARS: {
      try {
        return stof(value_.pointer_value_);
      } catch (exception const &ex) {
        LOG_TRACE("failed to convert string to float. s=%s, ex=%s", value_.pointer_value_, ex.what());
        return 0.0;
      }
    } break;
    case AttrType::INTS: {
      return float(value_.int_value_);
    } break;
    case AttrType::FLOATS: {
      return value_.float_value_;
    } break;
    case AttrType::BOOLEANS: {
      return float(value_.bool_value_);
    } break;
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return 0;
    }
  }
  return 0;
}

date Value::get_date() const
{ 
  if (is_null_) {
    LOG_WARN("can't get null value as date");
  }
  switch (attr_type_) {
    case AttrType::CHARS: {
      LOG_TRACE("failed to convert string to date.");
      return (date)0;
    } break;
    case AttrType::INTS: {
      LOG_TRACE("failed to convert int to date.");
      return (date)0;
    } break;
    case AttrType::FLOATS: {
      LOG_TRACE("failed to convert float to date.");
      return (date)0;
    } break;
    case AttrType::BOOLEANS: {
      LOG_TRACE("failed to convert bool to date.");
      return (date)0;
    } break;
    case AttrType::DATES: {
      return value_.date_value_;
    } break;
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return false;
    }
  }
}

string Value::get_string() const { 
  if (is_null_) {
    LOG_WARN("can't get null value as string");
  }  
  return this->to_string(); 
}

bool Value::get_boolean() const
{
  if (is_null_) {
    LOG_WARN("can't get null value as boolean");
  }
  switch (attr_type_) {
    case AttrType::CHARS: {
      try {
        float val = stof(value_.pointer_value_);
        if (val >= EPSILON || val <= -EPSILON) {
          return true;
        }

        int int_val = stol(value_.pointer_value_);
        if (int_val != 0) {
          return true;
        }

        return value_.pointer_value_ != nullptr;
      } catch (exception const &ex) {
        LOG_TRACE("failed to convert string to float or integer. s=%s, ex=%s", value_.pointer_value_, ex.what());
        return value_.pointer_value_ != nullptr;
      }
    } break;
    case AttrType::INTS: {
      return value_.int_value_ != 0;
    } break;
    case AttrType::FLOATS: {
      float val = value_.float_value_;
      return val >= EPSILON || val <= -EPSILON;
    } break;
    case AttrType::BOOLEANS: {
      return value_.bool_value_;
    } break;
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return false;
    }
  }
  return false;
}

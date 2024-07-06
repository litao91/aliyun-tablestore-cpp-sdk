#pragma once
#ifndef TABLESTORE_CORE_TYPES_HPP
#define TABLESTORE_CORE_TYPES_HPP
/*
BSD 3-Clause License

Copyright (c) 2017, Alibaba Cloud
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "tablestore/core/error.hpp"
#include "tablestore/util/logger.hpp"
#include "tablestore/util/mempiece.hpp"
#include "tablestore/util/prettyprint.hpp"
#include "tablestore/util/random.hpp"
#include "tablestore/util/result.hpp"
#include "tablestore/util/threading.hpp"
#include "tablestore/util/timestamp.hpp"
#include <boost/noncopyable.hpp>
#include <deque>
#include <memory>
#include <string>

namespace aliyun {
namespace tablestore {
namespace core {

enum Action {
  kApi_CreateTable,
  kApi_ListTable,
  kApi_DescribeTable,
  kApi_DeleteTable,
  kApi_UpdateTable,
  kApi_GetRow,
  kApi_PutRow,
  kApi_UpdateRow,
  kApi_DeleteRow,
  kApi_BatchGetRow,
  kApi_BatchWriteRow,
  kApi_GetRange,
  kApi_ComputeSplitsBySize,
};

void collectEnum(std::deque<Action> &);

/**
 * Types of primary key
 */
enum PrimaryKeyType {
  kPKT_Integer,
  kPKT_String,
  kPKT_Binary,
};

/**
 * For internal usage only. Do NOT use it.
 */
enum BloomFilterType {
  kBFT_None = 1,
  kBFT_Cell,
  kBFT_Row,
};

void collectEnum(std::deque<BloomFilterType> &);

enum TableStatus {
  kTS_Active = 1,
  kTS_Inactive,
  kTS_Loading,
  kTS_Unloading,
  kTS_Updating,
};

void collectEnum(std::deque<TableStatus> &xs);

enum CompareResult {
  kCR_Uncomparable,
  kCR_Equivalent,
  kCR_Smaller,
  kCR_Larger,
};

class RetryStrategy {
public:
  enum RetryCategory {
    UNRETRIABLE,
    RETRIABLE,
    DEPENDS,
  };

  static RetryCategory retriable(const OTSError &);
  static bool retriable(Action, const OTSError &);

  virtual ~RetryStrategy() {}

  virtual RetryStrategy *clone() const = 0;
  virtual int64_t retries() const throw() = 0;
  virtual bool shouldRetry(Action, const OTSError &) const = 0;
  virtual util::Duration nextPause() = 0;
};

template <class Elem> class IVector {
public:
  typedef Elem ElemType;

  virtual ~IVector() {}

  virtual void prettyPrint(std::string &out) const {
    if (size() == 0) {
      out.append("[]");
      return;
    }
    out.push_back('[');
    pp::prettyPrint(out, (*this)[0]);
    for (int64_t i = 1, sz = size(); i < sz; ++i) {
      out.push_back(',');
      pp::prettyPrint(out, (*this)[i]);
    }
    out.push_back(']');
  }

  virtual int64_t size() const = 0;
  virtual const Elem &operator[](int64_t idx) const = 0;
  virtual Elem &operator[](int64_t idx) = 0;
  virtual const Elem &back() const = 0;
  virtual Elem &back() = 0;
  virtual Elem &append() = 0;
  virtual void reset() = 0;
};

template <typename T>
bool operator==(const IVector<T> &a, const IVector<T> &b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (int64_t i = 0, sz = a.size(); i < sz; ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

template <typename T>
bool operator!=(const IVector<T> &a, const IVector<T> &b) {
  return !(a == b);
}

template <class Elem> class DequeBasedVector : public IVector<Elem> {
public:
  typedef typename IVector<Elem>::ElemType ElemType;

  explicit DequeBasedVector() = default;
  explicit DequeBasedVector(const DequeBasedVector &) = default;
  explicit DequeBasedVector(DequeBasedVector &&) = default;

  DequeBasedVector &operator=(DequeBasedVector &&a) = default;
  DequeBasedVector &operator=(const DequeBasedVector &a) = default;

  void prettyPrint(std::string &out) const { IVector<Elem>::prettyPrint(out); }

  int64_t size() const { return mElems.size(); }

  const Elem &operator[](int64_t idx) const { return mElems.at(idx); }

  Elem &operator[](int64_t idx) { return mElems.at(idx); }

  const Elem &back() const {
    OTS_ASSERT(size() > 0);
    return mElems.back();
  }

  Elem &back() {
    OTS_ASSERT(size() > 0);
    return mElems.back();
  }

  Elem &append() {
    mElems.push_back(Elem());
    return mElems.back();
  }

  void reset() { mElems.clear(); }

private:
  std::deque<Elem> mElems;
};

class Endpoint {
public:
  explicit Endpoint() = default;
  explicit Endpoint(const std::string &endpoint, const std::string &instance);
  Endpoint(const Endpoint &a) = default;
  Endpoint(Endpoint &&a) = default;
  Endpoint &operator=(Endpoint &&a) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &endpoint() const { return mEndpoint; }

  std::string &mutableEndpoint() { return mEndpoint; }

  const std::string &instanceName() const { return mInstanceName; }

  std::string &mutableInstanceName() { return mInstanceName; }

private:
  std::string mEndpoint;
  std::string mInstanceName;
};

class Credential {
public:
  explicit Credential() {}

  explicit Credential(const std::string &accessKeyId,
                      const std::string &accessKeySecret)
      : mAccessKeyId(accessKeyId), mAccessKeySecret(accessKeySecret) {}

  explicit Credential(const std::string &accessKeyId,
                      const std::string &accessKeySecret,
                      const std::string &securityToken)
      : mAccessKeyId(accessKeyId), mAccessKeySecret(accessKeySecret),
        mSecurityToken(securityToken) {}

  Credential(Credential &&) = default;

  Credential &operator=(Credential const &) = default;
  Credential &operator=(Credential &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &accessKeyId() const { return mAccessKeyId; }

  std::string &mutableAccessKeyId() { return mAccessKeyId; }

  const std::string &accessKeySecret() const { return mAccessKeySecret; }

  std::string &mutableAccessKeySecret() { return mAccessKeySecret; }

  const std::string &securityToken() const { return mSecurityToken; }

  std::string &mutableSecurityToken() { return mSecurityToken; }

private:
  std::string mAccessKeyId;
  std::string mAccessKeySecret;
  std::string mSecurityToken;
};

class Tracker {
public:
  explicit Tracker() : mTraceHash(0) {}

  explicit Tracker(const std::string &traceId)
      : mTraceId(traceId), mTraceHash(0) {
    calculateHash();
  }

  explicit Tracker(const Tracker &a) = default;
  explicit Tracker(Tracker &&a) = default;

  Tracker &operator=(const Tracker &a) = default;
  Tracker &operator=(Tracker &&a) = default;

  static Tracker create(util::Random &);

  std::optional<OTSError> validate() const;
  void prettyPrint(std::string &) const;

  const std::string &traceId() const { return mTraceId; }

  uint64_t traceHash() const { return mTraceHash; }

private:
  void calculateHash();

private:
  std::string mTraceId;
  uint64_t mTraceHash;
};

class ClientOptions {
public:
  explicit ClientOptions();
  explicit ClientOptions(ClientOptions &&) = default;

  ClientOptions &operator=(ClientOptions &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  int64_t maxConnections() const { return mMaxConnections; }

  int64_t &mutableMaxConnections() { return mMaxConnections; }

  const util::Duration &connectTimeout() const { return mConnectTimeout; }

  util::Duration &mutableConnectTimeout() { return mConnectTimeout; }

  const util::Duration &requestTimeout() const { return mRequestTimeout; }

  util::Duration &mutableRequestTimeout() { return mRequestTimeout; }

  void resetRetryStrategy(RetryStrategy *);
  RetryStrategy *releaseRetryStrategy();

  util::Logger &mutableLogger() { return *mLogger; }

  void resetLogger(util::Logger *);
  util::Logger *releaseLogger();

  const std::deque<std::shared_ptr<util::Actor>> &actors() const {
    return mActors;
  }

  std::deque<std::shared_ptr<util::Actor>> &mutableActors() { return mActors; }

private:
  int64_t mMaxConnections;
  util::Duration mConnectTimeout;
  util::Duration mRequestTimeout;
  std::unique_ptr<RetryStrategy> mRetryStrategy;
  std::unique_ptr<util::Logger> mLogger;
  std::deque<std::shared_ptr<util::Actor>> mActors;
};

/**
 * Schema of a single primary key
 */
class PrimaryKeyColumnSchema {
public:
  enum Option {
    AutoIncrement,
  };

  explicit PrimaryKeyColumnSchema() : mType(kPKT_Integer) {}

  explicit PrimaryKeyColumnSchema(const std::string &name, PrimaryKeyType type)
      : mName(name), mType(type) {}

  explicit PrimaryKeyColumnSchema(const std::string &name, PrimaryKeyType type,
                                  Option opt)
      : mName(name), mType(type), mOption(opt) {}

  explicit PrimaryKeyColumnSchema(PrimaryKeyColumnSchema &&a) = default;

  PrimaryKeyColumnSchema &operator=(PrimaryKeyColumnSchema &&a) = default;

  const std::string &name() const { return mName; }

  std::string &mutableName() { return mName; }

  PrimaryKeyType type() const { return mType; }

  PrimaryKeyType &mutableType() { return mType; }

  const std::optional<Option> &option() const { return mOption; }

  std::optional<Option> &mutableOption() { return mOption; }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

private:
  std::string mName;
  PrimaryKeyType mType;
  std::optional<Option> mOption;
};

class Schema : public IVector<PrimaryKeyColumnSchema> {
public:
  explicit Schema() {}

  explicit Schema(Schema &&a) = default;

  Schema &operator=(Schema &&) = default;

  int64_t size() const { return mColumns.size(); }

  const PrimaryKeyColumnSchema &operator[](int64_t idx) const {
    return mColumns[idx];
  }

  PrimaryKeyColumnSchema &operator[](int64_t idx) { return mColumns[idx]; }

  const PrimaryKeyColumnSchema &back() const { return mColumns.back(); }

  PrimaryKeyColumnSchema &back() { return mColumns.back(); }

  PrimaryKeyColumnSchema &append() { return mColumns.append(); }

  void reset() { return mColumns.reset(); }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;

private:
  DequeBasedVector<PrimaryKeyColumnSchema> mColumns;
};

class PrimaryKeyValue {
public:
  enum Category {
    kNone,
    kInfMin,
    kInfMax,
    kAutoIncr,
    kInteger,
    kString,
    kBinary,
  };

  static PrimaryKeyType toPrimaryKeyType(Category);

private:
  class InfMin {};
  class InfMax {};
  class AutoIncrement {};
  class Str {};
  class Bin {};

  explicit PrimaryKeyValue(int64_t);
  explicit PrimaryKeyValue(Str, const std::string &);
  explicit PrimaryKeyValue(Bin, const std::string &);
  explicit PrimaryKeyValue(InfMin);
  explicit PrimaryKeyValue(InfMax);
  explicit PrimaryKeyValue(AutoIncrement);

public:
  explicit PrimaryKeyValue();
  explicit PrimaryKeyValue(PrimaryKeyValue &&) = default;
  PrimaryKeyValue(const PrimaryKeyValue &) = default;
  PrimaryKeyValue &operator=(PrimaryKeyValue &&) = default;
  PrimaryKeyValue &operator=(const PrimaryKeyValue &) = default;

  Category category() const;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
  CompareResult compare(const PrimaryKeyValue &) const;

  bool operator==(const PrimaryKeyValue &a) const {
    return compare(a) == kCR_Equivalent;
  }

  bool operator!=(const PrimaryKeyValue &a) const { return !(*this == a); }

  bool isReal() const {
    switch (category()) {
    case kNone:
    case kInfMin:
    case kInfMax:
    case kAutoIncr:
      return false;
    case kInteger:
    case kString:
    case kBinary:
      return true;
    }
    return false;
  }

  bool isInfinity() const {
    switch (category()) {
    case kInfMin:
    case kInfMax:
      return true;
    case kNone:
    case kAutoIncr:
    case kInteger:
    case kString:
    case kBinary:
      return false;
    }
    return false;
  }

public:
  // for integers
  static PrimaryKeyValue toInteger(int64_t);
  int64_t integer() const;
  int64_t &mutableInteger();

public:
  // for string
  static PrimaryKeyValue toStr(const std::string &);
  const std::string &str() const;
  std::string &mutableStr();

public:
  // for blob
  static PrimaryKeyValue toBlob(const std::string &);
  const std::string &blob() const;
  std::string &mutableBlob();

public:
  // for +inf
  static PrimaryKeyValue toInfMax();
  bool isInfMax() const;
  void setInfMax();

public:
  // for -inf
  static PrimaryKeyValue toInfMin();
  bool isInfMin() const;
  void setInfMin();

public:
  // for placeholder for auto-increment
  static PrimaryKeyValue toAutoIncrement();
  bool isAutoIncrement() const;
  void setAutoIncrement();

private:
  Category mCategory;
  int64_t mIntValue;
  std::string mStrBlobValue;
};

/**
 * A single column of primary key
 */
class PrimaryKeyColumn {
public:
  explicit PrimaryKeyColumn();
  PrimaryKeyColumn(const std::string &, const PrimaryKeyValue &);
  PrimaryKeyColumn(const PrimaryKeyColumn &&a)
      : mName(std::move(a.mName)), mValue(std::move(a.mValue)) {}

  PrimaryKeyColumn &operator=(PrimaryKeyColumn &&) = default;
  PrimaryKeyColumn &operator=(const PrimaryKeyColumn &) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
  bool operator==(const PrimaryKeyColumn &) const;

  bool operator!=(const PrimaryKeyColumn &a) const { return !(*this == a); }

  const std::string &name() const { return mName; }

  std::string &mutableName() { return mName; }

  const PrimaryKeyValue &value() const { return mValue; }

  PrimaryKeyValue &mutableValue() { return mValue; }

private:
  std::string mName;
  PrimaryKeyValue mValue;
};

class PrimaryKey : public IVector<PrimaryKeyColumn> {
public:
  explicit PrimaryKey() = default;
  PrimaryKey(const PrimaryKey &) = default;
  explicit PrimaryKey(PrimaryKey &&a) = default;

  PrimaryKey &operator=(PrimaryKey &&a) = default;
  PrimaryKey &operator=(const PrimaryKey &a) = default;

  int64_t size() const { return mColumns.size(); }

  const PrimaryKeyColumn &operator[](int64_t idx) const {
    return mColumns[idx];
  }

  PrimaryKeyColumn &operator[](int64_t idx) { return mColumns[idx]; }

  const PrimaryKeyColumn &back() const { return mColumns.back(); }

  PrimaryKeyColumn &back() { return mColumns.back(); }

  PrimaryKeyColumn &append() { return mColumns.append(); }

  void reset() { return mColumns.reset(); }

  void prettyPrint(std::string &out) const;
  std::optional<OTSError> validate() const;
  CompareResult compare(const PrimaryKey &a) const;

private:
  DequeBasedVector<PrimaryKeyColumn> mColumns;
};

/**
 * Meta of a table.
 * Once the table is created, these configurations will never be modified.
 */
class TableMeta {
public:
  explicit TableMeta() = default;

  explicit TableMeta(const std::string &tableName) : mTableName(tableName) {}

  explicit TableMeta(TableMeta &&a) = default;

  TableMeta &operator=(TableMeta &&a) = default;

  const std::string &tableName() const { return mTableName; }

  std::string &mutableTableName() { return mTableName; }

  const Schema &schema() const { return mSchema; }

  Schema &mutableSchema() { return mSchema; }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

private:
  std::string mTableName;
  Schema mSchema;
};

class CapacityUnit {
public:
  explicit CapacityUnit() {}

  explicit CapacityUnit(int64_t readCU, int64_t writeCU)
      : mRead(readCU), mWrite(writeCU) {}

  explicit CapacityUnit(CapacityUnit &&a) = default;
  explicit CapacityUnit(const CapacityUnit &a) = default;

  CapacityUnit &operator=(CapacityUnit &&a) = default;

  const std::optional<int64_t> read() const { return mRead; }

  std::optional<int64_t> &mutableRead() { return mRead; }

  const std::optional<int64_t> &write() const { return mWrite; }

  std::optional<int64_t> &mutableWrite() { return mWrite; }

  std::optional<OTSError> validate() const;
  void prettyPrint(std::string &) const;
  void reset();

private:
  std::optional<int64_t> mRead;
  std::optional<int64_t> mWrite;
};

/**
 * Options of tables, which can be updated by UpdateTable
 */
class TableOptions {
public:
  explicit TableOptions() {}

  explicit TableOptions(TableOptions &&a) = default;

  TableOptions &operator=(TableOptions &&a);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::optional<util::Duration> &timeToLive() const {
    return mTimeToLive;
  }

  std::optional<util::Duration> &mutableTimeToLive() { return mTimeToLive; }

  const std::optional<int64_t> maxVersions() const { return mMaxVersions; }

  std::optional<int64_t> &mutableMaxVersions() { return mMaxVersions; }

  const std::optional<BloomFilterType> bloomFilterType() const {
    return mBloomFilterType;
  }

  std::optional<BloomFilterType> &mutableBloomFilterType() {
    return mBloomFilterType;
  }

  const std::optional<int64_t> blockSize() const { return mBlockSize; }

  std::optional<int64_t> &mutableBlockSize() { return mBlockSize; }

  const std::optional<util::Duration> &maxTimeDeviation() const {
    return mMaxTimeDeviation;
  }

  std::optional<util::Duration> &mutableMaxTimeDeviation() {
    return mMaxTimeDeviation;
  }

  const std::optional<CapacityUnit> &reservedThroughput() const {
    return mReservedThroughput;
  }

  std::optional<CapacityUnit> &mutableReservedThroughput() {
    return mReservedThroughput;
  }

private:
  std::optional<CapacityUnit> mReservedThroughput;
  std::optional<util::Duration> mTimeToLive;
  std::optional<int64_t> mMaxVersions;
  std::optional<BloomFilterType> mBloomFilterType;
  std::optional<int64_t> mBlockSize;
  std::optional<util::Duration> mMaxTimeDeviation;
};

class AttributeValue {
public:
  enum Category {
    kNone,
    kString,
    kInteger,
    kBinary,
    kBoolean,
    kFloatPoint,
  };

private:
  class Str {};
  class Blob {};

  explicit AttributeValue(int64_t);
  explicit AttributeValue(Str, const std::string &);
  explicit AttributeValue(Blob, const std::string &);
  explicit AttributeValue(bool);
  explicit AttributeValue(double);

public:
  explicit AttributeValue();
  explicit AttributeValue(AttributeValue &&a) = default;
  explicit AttributeValue(const AttributeValue &a) = default;
  AttributeValue &operator=(AttributeValue &&);

  Category category() const { return mCategory; }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
  CompareResult compare(const AttributeValue &) const;

  bool operator==(const AttributeValue &a) const {
    return compare(a) == kCR_Equivalent;
  }

  bool operator!=(const AttributeValue &a) const { return !(*this == a); }

public:
  // for strings
  static AttributeValue toStr(const std::string &);
  const std::string &str() const;
  std::string &mutableStr();

public:
  // for blob
  static AttributeValue toBlob(const std::string &);
  const std::string &blob() const;
  std::string &mutableBlob();

public:
  // for integers
  static AttributeValue toInteger(int64_t);
  int64_t integer() const;
  int64_t &mutableInteger();

public:
  // for floating point numbers
  static AttributeValue toFloatPoint(double);
  double floatPoint() const;
  double &mutableFloatPoint();

public:
  // for booleans
  static AttributeValue toBoolean(bool);
  bool boolean() const;
  bool &mutableBoolean();

private:
  Category mCategory;
  int64_t mIntValue;
  std::string mStrBlobValue;
  bool mBoolValue;
  double mFloatingValue;
};

class Attribute {
public:
  explicit Attribute() {}
  explicit Attribute(const std::string &, const AttributeValue &);
  explicit Attribute(const std::string &, const AttributeValue &,
                     util::UtcTime);

  explicit Attribute(Attribute &&a) = default;

  Attribute &operator=(Attribute &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
  bool operator==(const Attribute &) const;

  bool operator!=(const Attribute &a) const { return !(*this == a); }

  const std::string &name() const { return mName; }

  std::string &mutableName() { return mName; }

  const AttributeValue &value() const { return mValue; }

  AttributeValue &mutableValue() { return mValue; }

  const std::optional<util::UtcTime> &timestamp() const { return mTimestamp; }

  std::optional<util::UtcTime> &mutableTimestamp() { return mTimestamp; }

private:
  std::string mName;
  AttributeValue mValue;
  std::optional<util::UtcTime> mTimestamp;
};

class Row {
public:
  explicit Row() {}
  explicit Row(Row &&a);
  explicit Row(const Row &a);

  Row &operator=(Row &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
  bool operator==(const Row &) const;

  bool operator!=(const Row &a) const { return !(*this == a); }

  const PrimaryKey &primaryKey() const { return mPkey; }

  PrimaryKey &mutablePrimaryKey() { return mPkey; }

  const IVector<Attribute> &attributes() const { return mAttrs; }

  IVector<Attribute> &mutableAttributes() { return mAttrs; }

private:
  PrimaryKey mPkey;
  DequeBasedVector<Attribute> mAttrs;
};

/**
 * a range of UTC time.
 * Both (inclusive) start and (exclusive) end  must be integral multiple of
 * milliseconds.
 */
class TimeRange {
public:
  explicit TimeRange() {}
  explicit TimeRange(util::UtcTime start, util::UtcTime end)
      : mStart(start), mEnd(end) {}

  explicit TimeRange(TimeRange &&a) = default;
  explicit TimeRange(const TimeRange &a) = default;

  TimeRange &operator=(TimeRange &&a);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  util::UtcTime start() const { return mStart; }

  util::UtcTime &mutableStart() { return mStart; }

  util::UtcTime end() const { return mEnd; }

  util::UtcTime &mutableEnd() { return mEnd; }

private:
  util::UtcTime mStart;
  util::UtcTime mEnd;
};

class Split {
public:
  explicit Split() {}

  explicit Split(Split &&a) = default;

  Split &operator=(Split &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  /**
   * The (inclusive) lower bound of the split, as the same length as the primary
   * key columns of the table.
   */
  const std::shared_ptr<PrimaryKey> &lowerBound() const { return mLowerBound; }

  std::shared_ptr<PrimaryKey> &mutableLowerBound() { return mLowerBound; }

  /**
   * The (exclusive) upper bound of the split, as the same length as the primary
   * key columns of the table.
   */
  const std::shared_ptr<PrimaryKey> &upperBound() const { return mUpperBound; }

  std::shared_ptr<PrimaryKey> &mutableUpperBound() { return mUpperBound; }

  /**
   * A hint of the location where the split lies in.
   * If a location is not comfortable to be seen, it will keep empty.
   */
  const std::string location() const { return mLocation; }

  std::string &mutableLocation() { return mLocation; }

private:
  std::shared_ptr<PrimaryKey> mLowerBound;
  std::shared_ptr<PrimaryKey> mUpperBound;
  std::string mLocation;
};

// conditions

class ColumnCondition {
public:
  enum Type { kSingle, kComposite };

  virtual ~ColumnCondition() {}
  virtual Type type() const = 0;
  virtual void prettyPrint(std::string &) const = 0;
  virtual std::optional<OTSError> validate() const = 0;
  virtual void reset() = 0;
};

bool operator==(const ColumnCondition &, const ColumnCondition &);

inline bool operator!=(const ColumnCondition &a, const ColumnCondition &b) {
  return !(a == b);
}

class SingleColumnCondition : public ColumnCondition {
public:
  enum Relation {
    kEqual,
    kNotEqual,
    kLarger,
    kLargerEqual,
    kSmaller,
    kSmallerEqual,
  };

public:
  explicit SingleColumnCondition()
      : mRelation(kEqual), mPassIfMissing(false), mLatestVersionOnly(true) {}

  explicit SingleColumnCondition(const std::string &columnName, Relation rel,
                                 const AttributeValue &columnValue)
      : mColumnName(columnName), mRelation(rel), mColumnValue(columnValue),
        mPassIfMissing(false), mLatestVersionOnly(true) {}

  explicit SingleColumnCondition(SingleColumnCondition &&a) = default;

  SingleColumnCondition &operator=(SingleColumnCondition &&) = default;
  bool operator==(const SingleColumnCondition &) const;

  ColumnCondition::Type type() const { return ColumnCondition::kSingle; }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &columnName() const { return mColumnName; }

  std::string &mutableColumnName() { return mColumnName; }

  Relation relation() const { return mRelation; }

  Relation &mutableRelation() { return mRelation; }

  const AttributeValue &columnValue() const { return mColumnValue; }

  AttributeValue &mutableAttributeValue() { return mColumnValue; }

  bool passIfMissing() const { return mPassIfMissing; }

  bool &mutablePassIfMissing() { return mPassIfMissing; }

  bool latestVersionOnly() const { return mLatestVersionOnly; }

  bool &mutableLatestVersionOnly() { return mLatestVersionOnly; }

private:
  std::string mColumnName;
  Relation mRelation;
  AttributeValue mColumnValue;
  bool mPassIfMissing;
  bool mLatestVersionOnly;
};

class CompositeColumnCondition : public ColumnCondition {
public:
  enum Operator {
    kNot,
    kAnd,
    kOr,
  };

public:
  explicit CompositeColumnCondition() : mOperator(kAnd) {}

  explicit CompositeColumnCondition(CompositeColumnCondition &&a) = default;

  CompositeColumnCondition &operator=(CompositeColumnCondition &&) = default;
  bool operator==(const CompositeColumnCondition &) const;

  Type type() const { return kComposite; }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  Operator op() const { return mOperator; }

  Operator &mutableOp() { return mOperator; }

  const IVector<std::shared_ptr<ColumnCondition>> &children() const {
    return mChildren;
  }

  IVector<std::shared_ptr<ColumnCondition>> &mutableChildren() {
    return mChildren;
  }

private:
  Operator mOperator;
  DequeBasedVector<std::shared_ptr<ColumnCondition>> mChildren;
};

class Condition {
public:
  enum RowExistenceExpectation {
    kIgnore = 0,
    kExpectExist,
    kExpectNotExist,
  };

public:
  explicit Condition() : mRowCondition(kIgnore) {}

  explicit Condition(const Condition &a) = default;
  explicit Condition(Condition &&a) = default;

  Condition &operator=(Condition &&);
  Condition &operator=(const Condition &);
  bool operator==(const Condition &) const;
  bool operator!=(const Condition &b) const { return !(*this == b); }

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  RowExistenceExpectation rowCondition() const { return mRowCondition; }

  RowExistenceExpectation &mutableRowCondition() { return mRowCondition; }

  const std::shared_ptr<ColumnCondition> &columnCondition() const {
    return mColumnCondition;
  }

  std::shared_ptr<ColumnCondition> *mutableColumnCondition() {
    return &mColumnCondition;
  }

private:
  RowExistenceExpectation mRowCondition;
  std::shared_ptr<ColumnCondition> mColumnCondition;
};

// row changes

class RowChange {
public:
  enum ReturnType {
    kRT_None,
    kRT_PrimaryKey,
  };

protected:
  explicit RowChange() : mReturnType(kRT_None) {}
  explicit RowChange(const RowChange &) = default;
  explicit RowChange(RowChange &&) = default;
  RowChange &operator=(RowChange &&) = default;
  RowChange &operator=(const RowChange &) = default;

  virtual void prettyPrint(std::string &) const;
  bool operator==(const RowChange &) const;

public:
  virtual ~RowChange() {}
  virtual std::optional<OTSError> validate() const;
  virtual void reset();

  const std::string &table() const { return mTable; }

  std::string &mutableTable() { return mTable; }

  const PrimaryKey &primaryKey() const { return mPrimaryKey; }

  PrimaryKey &mutablePrimaryKey() { return mPrimaryKey; }

  const Condition &condition() const { return mCondition; }

  Condition &mutableCondition() { return mCondition; }

  ReturnType returnType() const { return mReturnType; }

  ReturnType &mutableReturnType() { return mReturnType; }

private:
  std::string mTable;
  PrimaryKey mPrimaryKey;
  Condition mCondition;
  ReturnType mReturnType;
};

class RowPutChange : public RowChange {
public:
  explicit RowPutChange() {}
  explicit RowPutChange(RowPutChange &&a) = default;
  explicit RowPutChange(const RowPutChange &a) = default;

  RowPutChange &operator=(RowPutChange &&) = default;
  RowPutChange &operator=(const RowPutChange &) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const IVector<Attribute> &attributes() const { return mAttrs; }

  IVector<Attribute> &mutableAttributes() { return mAttrs; }

private:
  DequeBasedVector<Attribute> mAttrs;
};

class RowUpdateChange : public RowChange {
public:
  class Update {
  public:
    enum Type {
      /**
       * Overwrites a cell with a specific timestamp.
       * If the cell does not previously exist, insert it.
       */
      kPut,
      /**
       * Deletes a single cell with a specific timestamp.
       */
      kDelete,
      /**
       * Deletes all cells of a column
       */
      kDeleteAll,
    };

  public:
    explicit Update() : mType(kPut) {}

    explicit Update(Update &&a) = default;

    Update &operator=(Update &&) = default;
    void prettyPrint(std::string &) const;
    std::optional<OTSError> validate() const;
    bool operator==(const Update &) const;
    bool operator!=(const Update &b) const { return !(*this == b); }

    Type type() const { return mType; }

    Type &mutableType() { return mType; }

    const std::string &attrName() const { return mAttrName; }

    std::string &mutableAttrName() { return mAttrName; }

    const std::optional<AttributeValue> &attrValue() const {
      return mAttrValue;
    }

    std::optional<AttributeValue> &mutableAttrValue() { return mAttrValue; }

    const std::optional<util::UtcTime> &timestamp() const { return mTimestamp; }

    std::optional<util::UtcTime> &mutableTimestamp() { return mTimestamp; }

  private:
    Type mType;
    std::string mAttrName;
    std::optional<AttributeValue> mAttrValue;
    std::optional<util::UtcTime> mTimestamp;
  };

public:
  explicit RowUpdateChange() {}

  explicit RowUpdateChange(RowUpdateChange &&a) = default;
  explicit RowUpdateChange(const RowUpdateChange &a) = default;

  RowUpdateChange &operator=(RowUpdateChange &&);
  RowUpdateChange &operator=(const RowUpdateChange &);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
  bool operator==(const RowUpdateChange &) const;
  bool operator!=(const RowUpdateChange &b) const { return !(*this == b); }

  const IVector<Update> &updates() const { return mUpdates; }

  IVector<Update> &mutableUpdates() { return mUpdates; }

private:
  DequeBasedVector<Update> mUpdates;
};

class RowDeleteChange : public RowChange {
public:
  explicit RowDeleteChange() = default;

  explicit RowDeleteChange(const RowDeleteChange &a) = default;
  explicit RowDeleteChange(RowDeleteChange &&a) = default;

  RowDeleteChange &operator=(RowDeleteChange &&a) = default;
  RowDeleteChange &operator=(const RowDeleteChange &a) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
};

/**
 * User data will pass from operations in requests to their results in
 * responses. It is an easy facility for users to identify operations and their
 * results.
 */
template <class T> class PairWithUserData {
public:
  typedef T ValueType;

  explicit PairWithUserData() : mUserData(NULL) {}

  explicit PairWithUserData(PairWithUserData<T> &&a) = default;

  PairWithUserData<T> &operator=(PairWithUserData<T> &&a) = default;

  void prettyPrint(std::string &out) const { pp::prettyPrint(out, get()); }

  const T &get() const { return mData; }

  T &mutableGet() { return mData; }

  const void *userData() const { return mUserData; }

  void const *&mutableUserData() { return mUserData; }

private:
  T mData;
  const void *mUserData;
};

// query criteria

class QueryCriterion {
protected:
  explicit QueryCriterion() = default;
  explicit QueryCriterion(QueryCriterion &&) = default;
  explicit QueryCriterion(const QueryCriterion &) = default;
  QueryCriterion &operator=(QueryCriterion &&) = default;

public:
  virtual ~QueryCriterion() {}
  virtual void prettyPrint(std::string &) const;
  virtual std::optional<OTSError> validate() const;
  virtual void reset();

  const std::string &table() const { return mTable; }

  std::string &mutableTable() { return mTable; }

  const IVector<std::string> &columnsToGet() const { return mColumnsToGet; }

  IVector<std::string> &mutableColumnsToGet() { return mColumnsToGet; }

  const std::optional<int64_t> &maxVersions() const { return mMaxVersions; }

  std::optional<int64_t> &mutableMaxVersions() { return mMaxVersions; }

  const std::optional<TimeRange> &timeRange() const { return mTimeRange; }

  std::optional<TimeRange> &mutableTimeRange() { return mTimeRange; }

  const std::optional<bool> &cacheBlocks() const { return mCacheBlocks; }

  std::optional<bool> &mutableCacheBlocks() { return mCacheBlocks; }

  const std::shared_ptr<ColumnCondition> &filter() const { return mFilter; }

  std::shared_ptr<ColumnCondition> &mutableFilter() { return mFilter; }

private:
  std::string mTable;
  DequeBasedVector<std::string> mColumnsToGet;
  std::optional<int64_t> mMaxVersions;
  std::optional<TimeRange> mTimeRange;
  std::optional<bool> mCacheBlocks;
  std::shared_ptr<ColumnCondition> mFilter;
};

class PointQueryCriterion : public QueryCriterion {
public:
  explicit PointQueryCriterion() {}

  explicit PointQueryCriterion(PointQueryCriterion &&a) = default;

  PointQueryCriterion &operator=(PointQueryCriterion &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const PrimaryKey &primaryKey() const { return mPrimaryKey; }

  PrimaryKey &mutablePrimaryKey() { return mPrimaryKey; }

private:
  PrimaryKey mPrimaryKey;
};

class RangeQueryCriterion : public QueryCriterion {
public:
  enum Direction { FORWARD, BACKWARD };

public:
  explicit RangeQueryCriterion() : mDirection(FORWARD) {}

  explicit RangeQueryCriterion(RangeQueryCriterion &&a) = default;

  RangeQueryCriterion &operator=(RangeQueryCriterion &&) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  Direction direction() const { return mDirection; }

  Direction &mutableDirection() { return mDirection; }

  const PrimaryKey &inclusiveStart() const { return mInclusiveStart; }

  PrimaryKey &mutableInclusiveStart() { return mInclusiveStart; }

  const PrimaryKey &exclusiveEnd() const { return mExclusiveEnd; }

  PrimaryKey &mutableExclusiveEnd() { return mExclusiveEnd; }

  const std::optional<int64_t> &limit() const { return mLimit; }

  std::optional<int64_t> &mutableLimit() { return mLimit; }

private:
  Direction mDirection;
  PrimaryKey mInclusiveStart;
  PrimaryKey mExclusiveEnd;
  std::optional<int64_t> mLimit;
};

class MultiPointQueryCriterion : public QueryCriterion {
public:
  typedef PairWithUserData<PrimaryKey> RowKey;

public:
  explicit MultiPointQueryCriterion() {}

  explicit MultiPointQueryCriterion(MultiPointQueryCriterion &&);
  MultiPointQueryCriterion &operator=(MultiPointQueryCriterion &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const IVector<RowKey> &rowKeys() const { return mRowKeys; }

  IVector<RowKey> &mutableRowKeys() { return mRowKeys; }

private:
  DequeBasedVector<RowKey> mRowKeys;
};

// requests and responses

class Response {
protected:
  Response() = default;
  Response(const Response &) = default;
  Response(Response &&) = default;
  Response &operator=(Response &&) = default;
  void prettyPrint(std::string &) const;
  void reset();

public:
  const std::string &requestId() const { return mRequestId; }

  std::string &mutableRequestId() { return mRequestId; }

  const std::string &traceId() const { return mTraceId; }

  std::string &mutableTraceId() { return mTraceId; }

private:
  std::string mRequestId;
  std::string mTraceId;
};

class CreateTableRequest {
public:
  explicit CreateTableRequest();
  explicit CreateTableRequest(CreateTableRequest &&) = default;
  CreateTableRequest &operator=(CreateTableRequest &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const TableMeta &meta() const { return mMeta; }

  TableMeta &mutableMeta() { return mMeta; }

  const TableOptions &options() const { return mOptions; }

  TableOptions &mutableOptions() { return mOptions; }

  /**
   * For now, each shard split point must contains exactly one primary
   * key column which conforms to the table schema.
   */
  const IVector<PrimaryKey> &shardSplitPoints() const {
    return mShardSplitPoints;
  }

  IVector<PrimaryKey> &mutableShardSplitPoints() { return mShardSplitPoints; }

private:
  TableMeta mMeta;
  TableOptions mOptions;
  DequeBasedVector<PrimaryKey> mShardSplitPoints;
};

class CreateTableResponse : public Response {
public:
  explicit CreateTableResponse() {}
  explicit CreateTableResponse(CreateTableResponse &&a) = default;

  CreateTableResponse &operator=(CreateTableResponse &&a) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
};

class ListTableRequest {
public:
  explicit ListTableRequest() {}
  explicit ListTableRequest(ListTableRequest &&) = default;

  ListTableRequest &operator=(ListTableRequest &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
};

class ListTableResponse : public Response {
public:
  explicit ListTableResponse() {}
  explicit ListTableResponse(ListTableResponse &&);
  ListTableResponse &operator=(ListTableResponse &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const IVector<std::string> &tables() const { return mTables; }

  IVector<std::string> &mutableTables() { return mTables; }

private:
  DequeBasedVector<std::string> mTables;
};

class DeleteTableRequest {
public:
  explicit DeleteTableRequest() {}
  explicit DeleteTableRequest(DeleteTableRequest &&);

  DeleteTableRequest &operator=(DeleteTableRequest &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &table() const { return mTable; }

  std::string &mutableTable() { return mTable; }

private:
  std::string mTable;
};

class DeleteTableResponse : public Response {
public:
  explicit DeleteTableResponse() {}
  explicit DeleteTableResponse(DeleteTableResponse &&);
  DeleteTableResponse &operator=(DeleteTableResponse &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
};

class DescribeTableRequest {
public:
  explicit DescribeTableRequest() {}
  explicit DescribeTableRequest(DescribeTableRequest &&);
  DescribeTableRequest &operator=(DescribeTableRequest &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &table() const { return mTable; }

  std::string &mutableTable() { return mTable; }

private:
  std::string mTable;
};

class DescribeTableResponse : public Response {
public:
  explicit DescribeTableResponse() : mStatus(kTS_Active) {}
  explicit DescribeTableResponse(DescribeTableResponse &&);
  DescribeTableResponse &operator=(DescribeTableResponse &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const TableMeta &meta() const { return mMeta; }

  TableMeta &mutableMeta() { return mMeta; }

  const TableOptions &options() const { return mOptions; }

  TableOptions &mutableOptions() { return mOptions; }

  TableStatus status() const { return mStatus; }

  TableStatus &mutableStatus() { return mStatus; }

  const IVector<PrimaryKey> &shardSplitPoints() const {
    return mShardSplitPoints;
  }

  IVector<PrimaryKey> &mutableShardSplitPoints() { return mShardSplitPoints; }

private:
  TableMeta mMeta;
  TableOptions mOptions;
  TableStatus mStatus;
  DequeBasedVector<PrimaryKey> mShardSplitPoints;
};

class UpdateTableRequest {
public:
  explicit UpdateTableRequest() {}
  explicit UpdateTableRequest(UpdateTableRequest &&);
  UpdateTableRequest &operator=(UpdateTableRequest &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &table() const { return mTable; }

  std::string &mutableTable() { return mTable; }

  const TableOptions &options() const { return mOptions; }

  TableOptions &mutableOptions() { return mOptions; }

private:
  std::string mTable;
  TableOptions mOptions;
};

class UpdateTableResponse : public Response {
public:
  explicit UpdateTableResponse() {}
  explicit UpdateTableResponse(UpdateTableResponse &&);
  UpdateTableResponse &operator=(UpdateTableResponse &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();
};

class ComputeSplitsBySizeRequest {
  static const int64_t kDefaultSplitSize = 5; // 500MB

public:
  explicit ComputeSplitsBySizeRequest() : mSplitSize(kDefaultSplitSize) {}

  explicit ComputeSplitsBySizeRequest(ComputeSplitsBySizeRequest &&);
  ComputeSplitsBySizeRequest &operator=(ComputeSplitsBySizeRequest &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const std::string &table() const { return mTable; }

  std::string &mutableTable() { return mTable; }

  int64_t splitSize() const { return mSplitSize; }

  int64_t &mutableSplitSize() { return mSplitSize; }

private:
  std::string mTable;
  int64_t mSplitSize;
};

class ComputeSplitsBySizeResponse : public Response {
public:
  explicit ComputeSplitsBySizeResponse() {}
  explicit ComputeSplitsBySizeResponse(ComputeSplitsBySizeResponse &&);
  ComputeSplitsBySizeResponse &operator=(ComputeSplitsBySizeResponse &&);

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const Schema &schema() const { return mSchema; }

  Schema &mutableSchema() { return mSchema; }

  const IVector<Split> &splits() const { return mSplits; }

  IVector<Split> &mutableSplits() { return mSplits; }

private:
  CapacityUnit mConsumedCapacity;
  Schema mSchema;
  DequeBasedVector<Split> mSplits;
};

class PutRowRequest {
public:
  explicit PutRowRequest() {}
  explicit PutRowRequest(PutRowRequest &&a) = default;

  PutRowRequest &operator=(PutRowRequest &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const RowPutChange &rowChange() const { return mRowChange; }

  RowPutChange &mutableRowChange() { return mRowChange; }

private:
  RowPutChange mRowChange;
};

class PutRowResponse : public Response {
public:
  explicit PutRowResponse() {}
  explicit PutRowResponse(PutRowResponse &&a) = default;

  PutRowResponse &operator=(PutRowResponse &&) = default;

  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const std::optional<Row> &row() const { return mRow; }

  std::optional<Row> &mutableRow() { return mRow; }

private:
  CapacityUnit mConsumedCapacity;
  std::optional<Row> mRow;
};

class GetRowRequest {
public:
  explicit GetRowRequest() {}

  explicit GetRowRequest(GetRowRequest &&a) = default;

  GetRowRequest &operator=(GetRowRequest &&) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const PointQueryCriterion &queryCriterion() const { return mQueryCriterion; }

  PointQueryCriterion &mutableQueryCriterion() { return mQueryCriterion; }

private:
  PointQueryCriterion mQueryCriterion;
};

class GetRowResponse : public Response {
public:
  explicit GetRowResponse() = default;

  explicit GetRowResponse(GetRowResponse &&a) = default;

  GetRowResponse &operator=(GetRowResponse &&a) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const std::optional<Row> &row() const { return mRow; }

  std::optional<Row> &mutableRow() { return mRow; }

private:
  CapacityUnit mConsumedCapacity;
  std::optional<Row> mRow;
};

class GetRangeRequest {
public:
  explicit GetRangeRequest() {}

  explicit GetRangeRequest(GetRangeRequest &&a) = default;

  GetRangeRequest &operator=(GetRangeRequest &&) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const RangeQueryCriterion &queryCriterion() const { return mQueryCriterion; }

  RangeQueryCriterion &mutableQueryCriterion() { return mQueryCriterion; }

private:
  RangeQueryCriterion mQueryCriterion;
};

class GetRangeResponse : public Response {
public:
  explicit GetRangeResponse() = default;

  explicit GetRangeResponse(GetRangeResponse &&a) = default;
  explicit GetRangeResponse(const GetRangeResponse &a) = default;

  GetRangeResponse &operator=(GetRangeResponse &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const IVector<Row> &rows() const { return mRows; }

  IVector<Row> &mutableRows() { return mRows; }

  const std::optional<PrimaryKey> &nextStart() const { return mNextStart; }

  std::optional<PrimaryKey> &mutableNextStart() { return mNextStart; }

private:
  CapacityUnit mConsumedCapacity;
  DequeBasedVector<Row> mRows;
  std::optional<PrimaryKey> mNextStart;
};

class UpdateRowRequest {
public:
  explicit UpdateRowRequest() {}

  explicit UpdateRowRequest(UpdateRowRequest &&a) = default;
  explicit UpdateRowRequest(const UpdateRowRequest &a) = default;

  UpdateRowRequest &operator=(UpdateRowRequest &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const RowUpdateChange &rowChange() const { return mRowChange; }

  RowUpdateChange &mutableRowChange() { return mRowChange; }

private:
  RowUpdateChange mRowChange;
};

class UpdateRowResponse : public Response {
public:
  explicit UpdateRowResponse() {}

  explicit UpdateRowResponse(UpdateRowResponse &&a) = default;
  explicit UpdateRowResponse(const UpdateRowResponse &a) = default;

  UpdateRowResponse &operator=(UpdateRowResponse &&) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const std::optional<Row> &row() const { return mRow; }

  std::optional<Row> &mutableRow() { return mRow; }

private:
  CapacityUnit mConsumedCapacity;
  std::optional<Row> mRow;
};

class DeleteRowRequest {
public:
  explicit DeleteRowRequest() = default;

  explicit DeleteRowRequest(DeleteRowRequest &&a) = default;
  explicit DeleteRowRequest(const DeleteRowRequest &a) = default;

  DeleteRowRequest &operator=(DeleteRowRequest &&a);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const RowDeleteChange &rowChange() const { return mRowChange; }

  RowDeleteChange &mutableRowChange() { return mRowChange; }

private:
  RowDeleteChange mRowChange;
};

class DeleteRowResponse : public Response {
public:
  explicit DeleteRowResponse() = default;

  explicit DeleteRowResponse(DeleteRowResponse &&a) = default;
  explicit DeleteRowResponse(const DeleteRowResponse &a) = default;

  DeleteRowResponse &operator=(DeleteRowResponse &&) = default;
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const std::optional<Row> &row() const { return mRow; }

  std::optional<Row> &mutableRow() { return mRow; }

private:
  CapacityUnit mConsumedCapacity;
  std::optional<Row> mRow;
};

class BatchGetRowRequest {
public:
  explicit BatchGetRowRequest() {}
  explicit BatchGetRowRequest(BatchGetRowRequest &&);
  BatchGetRowRequest &operator=(BatchGetRowRequest &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const IVector<MultiPointQueryCriterion> &criteria() const {
    return mCriteria;
  }

  IVector<MultiPointQueryCriterion> &mutableCriteria() { return mCriteria; }

private:
  DequeBasedVector<MultiPointQueryCriterion> mCriteria;
};

class BatchGetRowResponse : public Response {
public:
  typedef PairWithUserData<util::Result<std::optional<Row>, OTSError>> Result;

public:
  explicit BatchGetRowResponse() {}
  explicit BatchGetRowResponse(BatchGetRowResponse &&a) = default;
  explicit BatchGetRowResponse(const BatchGetRowResponse &a) = default;

  BatchGetRowResponse &operator=(BatchGetRowResponse &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const IVector<Result> &results() const { return mResults; }

  IVector<Result> &mutableResults() { return mResults; }

private:
  CapacityUnit mConsumedCapacity;
  DequeBasedVector<Result> mResults;
};

class BatchWriteRowRequest {
public:
  typedef PairWithUserData<RowPutChange> Put;
  typedef PairWithUserData<RowUpdateChange> Update;
  typedef PairWithUserData<RowDeleteChange> Delete;

public:
  explicit BatchWriteRowRequest() {}
  explicit BatchWriteRowRequest(BatchWriteRowRequest &&);
  BatchWriteRowRequest &operator=(BatchWriteRowRequest &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const IVector<Put> &puts() const { return mPuts; }

  IVector<Put> &mutablePuts() { return mPuts; }

  const IVector<Update> &updates() const { return mUpdates; }

  IVector<Update> &mutableUpdates() { return mUpdates; }

  const IVector<Delete> &deletes() const { return mDeletes; }

  IVector<Delete> &mutableDeletes() { return mDeletes; }

private:
  DequeBasedVector<Put> mPuts;
  DequeBasedVector<Update> mUpdates;
  DequeBasedVector<Delete> mDeletes;
};

class BatchWriteRowResponse : public Response {
public:
  typedef PairWithUserData<util::Result<std::optional<Row>, OTSError>> Result;

public:
  explicit BatchWriteRowResponse() {}
  explicit BatchWriteRowResponse(BatchWriteRowResponse &&);
  BatchWriteRowResponse &operator=(BatchWriteRowResponse &&);
  void prettyPrint(std::string &) const;
  std::optional<OTSError> validate() const;
  void reset();

  const CapacityUnit &consumedCapacity() const { return mConsumedCapacity; }

  CapacityUnit &mutableConsumedCapacity() { return mConsumedCapacity; }

  const IVector<Result> &putResults() const { return mPutResults; }

  IVector<Result> &mutablePutResults() { return mPutResults; }

  const IVector<Result> &updateResults() const { return mUpdateResults; }

  IVector<Result> &mutableUpdateResults() { return mUpdateResults; }

  const IVector<Result> &deleteResults() const { return mDeleteResults; }

  IVector<Result> &mutableDeleteResults() { return mDeleteResults; }

private:
  CapacityUnit mConsumedCapacity;
  DequeBasedVector<Result> mPutResults;
  DequeBasedVector<Result> mUpdateResults;
  DequeBasedVector<Result> mDeleteResults;
};

} // namespace core
} // namespace tablestore
} // namespace aliyun

namespace pp {
namespace impl {

template <> struct PrettyPrinter<aliyun::tablestore::core::Action, void> {
  void operator()(std::string &, aliyun::tablestore::core::Action) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::PrimaryKeyType, void> {
  void operator()(std::string &,
                  aliyun::tablestore::core::PrimaryKeyType) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::PrimaryKeyColumnSchema::Option,
                     void> {
  void
  operator()(std::string &,
             aliyun::tablestore::core::PrimaryKeyColumnSchema::Option) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::BloomFilterType, void> {
  void operator()(std::string &,
                  aliyun::tablestore::core::BloomFilterType) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::PrimaryKeyValue::Category,
                     void> {
  void operator()(std::string &,
                  aliyun::tablestore::core::PrimaryKeyValue::Category) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::CompareResult, void> {
  void operator()(std::string &, aliyun::tablestore::core::CompareResult) const;
};

template <> struct PrettyPrinter<aliyun::tablestore::core::TableStatus, void> {
  void operator()(std::string &, aliyun::tablestore::core::TableStatus) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::RowChange::ReturnType, void> {
  void operator()(std::string &,
                  aliyun::tablestore::core::RowChange::ReturnType) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::AttributeValue::Category, void> {
  void operator()(std::string &,
                  aliyun::tablestore::core::AttributeValue::Category) const;
};

template <>
struct PrettyPrinter<
    aliyun::tablestore::core::Condition::RowExistenceExpectation, void> {
  void operator()(
      std::string &,
      aliyun::tablestore::core::Condition::RowExistenceExpectation) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::SingleColumnCondition::Relation,
                     void> {
  void
  operator()(std::string &,
             aliyun::tablestore::core::SingleColumnCondition::Relation) const;
};

template <>
struct PrettyPrinter<
    aliyun::tablestore::core::CompositeColumnCondition::Operator, void> {
  void operator()(
      std::string &,
      aliyun::tablestore::core::CompositeColumnCondition::Operator) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::ColumnCondition::Type, void> {
  void operator()(std::string &,
                  aliyun::tablestore::core::ColumnCondition::Type) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::RangeQueryCriterion::Direction,
                     void> {
  void
  operator()(std::string &,
             aliyun::tablestore::core::RangeQueryCriterion::Direction) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::core::RowUpdateChange::Update::Type,
                     void> {
  void
  operator()(std::string &,
             aliyun::tablestore::core::RowUpdateChange::Update::Type) const;
};

template <>
struct PrettyPrinter<aliyun::tablestore::util::Result<
                         std::optional<aliyun::tablestore::core::Row>,
                         aliyun::tablestore::core::OTSError>,
                     void> {
  void operator()(std::string &,
                  const aliyun::tablestore::util::Result<
                      std::optional<aliyun::tablestore::core::Row>,
                      aliyun::tablestore::core::OTSError> &) const;
};

} // namespace impl
} // namespace pp
#endif

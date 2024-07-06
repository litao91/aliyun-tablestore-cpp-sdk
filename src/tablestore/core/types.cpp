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
#include "tablestore/core/types.hpp"
#include "tablestore/core/retry.hpp"
#include "tablestore/util/arithmetic.hpp"
#include "tablestore/util/assert.hpp"
#include "tablestore/util/logger.hpp"
#include "tablestore/util/network.hpp"
#include "tablestore/util/random.hpp"
#include "tablestore/util/security.hpp"
#include "tablestore/util/threading.hpp"
#include "tablestore/util/try.hpp"
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdint.h>

using namespace std;

using namespace aliyun::tablestore::util;
using namespace aliyun::tablestore::core;

namespace pp {
namespace impl {

void PrettyPrinter<Action, void>::operator()(string &out, Action act) const {
  switch (act) {
  case kApi_CreateTable:
    out.append("CreateTable");
    break;
  case kApi_ListTable:
    out.append("ListTable");
    break;
  case kApi_DescribeTable:
    out.append("DescribeTable");
    break;
  case kApi_DeleteTable:
    out.append("DeleteTable");
    break;
  case kApi_UpdateTable:
    out.append("UpdateTable");
    break;
  case kApi_GetRow:
    out.append("GetRow");
    break;
  case kApi_PutRow:
    out.append("PutRow");
    break;
  case kApi_UpdateRow:
    out.append("UpdateRow");
    break;
  case kApi_DeleteRow:
    out.append("DeleteRow");
    break;
  case kApi_BatchGetRow:
    out.append("BatchGetRow");
    break;
  case kApi_BatchWriteRow:
    out.append("BatchWriteRow");
    break;
  case kApi_GetRange:
    out.append("GetRange");
    break;
  case kApi_ComputeSplitsBySize:
    out.append("ComputeSplitPointsBySize");
    break;
  }
}

void PrettyPrinter<PrimaryKeyType, void>::operator()(string &out,
                                                     PrimaryKeyType pkt) const {
  switch (pkt) {
  case kPKT_Integer:
    out.append("kPKT_Integer");
    break;
  case kPKT_String:
    out.append("kPKT_String");
    break;
  case kPKT_Binary:
    out.append("kPKT_Binary");
    break;
  }
}

void PrettyPrinter<PrimaryKeyColumnSchema::Option, void>::operator()(
    string &out, PrimaryKeyColumnSchema::Option pko) const {
  switch (pko) {
  case PrimaryKeyColumnSchema::AutoIncrement:
    out.append("AutoIncrement");
    break;
  }
}

void PrettyPrinter<BloomFilterType, void>::operator()(
    string &out, BloomFilterType bft) const {
  switch (bft) {
  case kBFT_None:
    out.append("kBFT_None");
    break;
  case kBFT_Cell:
    out.append("kBFT_Cell");
    break;
  case kBFT_Row:
    out.append("kBFT_Row");
    break;
  }
}

void PrettyPrinter<PrimaryKeyValue::Category, void>::operator()(
    string &out, PrimaryKeyValue::Category cat) const {
  switch (cat) {
  case PrimaryKeyValue::kNone:
    out.append("None");
    break;
  case PrimaryKeyValue::kInfMin:
    out.append("-Inf");
    break;
  case PrimaryKeyValue::kInfMax:
    out.append("+Inf");
    break;
  case PrimaryKeyValue::kAutoIncr:
    out.append("AutoIncr");
    break;
  case PrimaryKeyValue::kInteger:
    out.append("Integer");
    break;
  case PrimaryKeyValue::kString:
    out.append("String");
    break;
  case PrimaryKeyValue::kBinary:
    out.append("Binary");
    break;
  }
}

void PrettyPrinter<CompareResult, void>::operator()(string &out,
                                                    CompareResult cr) const {
  switch (cr) {
  case kCR_Uncomparable:
    out.append("kCR_Uncomparable");
    break;
  case kCR_Equivalent:
    out.append("kCR_Equivalent");
    break;
  case kCR_Smaller:
    out.append("kCR_Smaller");
    break;
  case kCR_Larger:
    out.append("kCR_Larger");
    break;
  }
}

void PrettyPrinter<TableStatus, void>::operator()(string &out,
                                                  TableStatus ts) const {
  switch (ts) {
  case kTS_Active:
    out.append("kTS_Active");
    break;
  case kTS_Inactive:
    out.append("kTS_Inactive");
    break;
  case kTS_Loading:
    out.append("kTS_Loading");
    break;
  case kTS_Unloading:
    out.append("kTS_Unloading");
    break;
  case kTS_Updating:
    out.append("kTS_Updating");
    break;
  }
}

void PrettyPrinter<RowChange::ReturnType, void>::operator()(
    string &out, RowChange::ReturnType ts) const {
  switch (ts) {
  case RowChange::kRT_None:
    out.append("kRT_None");
    break;
  case RowChange::kRT_PrimaryKey:
    out.append("kRT_PrimaryKey");
    break;
  }
}

void PrettyPrinter<AttributeValue::Category, void>::operator()(
    string &out, AttributeValue::Category cat) const {
  switch (cat) {
  case AttributeValue::kNone:
    out.append("None");
    break;
  case AttributeValue::kString:
    out.append("String");
    break;
  case AttributeValue::kInteger:
    out.append("Integer");
    break;
  case AttributeValue::kBinary:
    out.append("Binary");
    break;
  case AttributeValue::kBoolean:
    out.append("Boolean");
    break;
  case AttributeValue::kFloatPoint:
    out.append("FloatPoint");
    break;
  }
}

void PrettyPrinter<Condition::RowExistenceExpectation, void>::operator()(
    string &out, Condition::RowExistenceExpectation exp) const {
  switch (exp) {
  case Condition::kIgnore:
    out.append("kIgnore");
    break;
  case Condition::kExpectExist:
    out.append("kExpectExist");
    break;
  case Condition::kExpectNotExist:
    out.append("kExpectNotExist");
    break;
  }
}

void PrettyPrinter<SingleColumnCondition::Relation, void>::operator()(
    string &out, SingleColumnCondition::Relation exp) const {
  switch (exp) {
  case SingleColumnCondition::kEqual:
    out.append("kEqual");
    break;
  case SingleColumnCondition::kNotEqual:
    out.append("kNotEqual");
    break;
  case SingleColumnCondition::kLarger:
    out.append("kLarger");
    break;
  case SingleColumnCondition::kLargerEqual:
    out.append("kLargerEqual");
    break;
  case SingleColumnCondition::kSmaller:
    out.append("kSmaller");
    break;
  case SingleColumnCondition::kSmallerEqual:
    out.append("kSmallerEqual");
    break;
  }
}

void PrettyPrinter<CompositeColumnCondition::Operator, void>::operator()(
    string &out, CompositeColumnCondition::Operator op) const {
  switch (op) {
  case CompositeColumnCondition::kNot:
    out.append("kNot");
    break;
  case CompositeColumnCondition::kAnd:
    out.append("kAnd");
    break;
  case CompositeColumnCondition::kOr:
    out.append("kOr");
    break;
  }
}

void PrettyPrinter<ColumnCondition::Type, void>::operator()(
    string &out, ColumnCondition::Type cct) const {
  switch (cct) {
  case ColumnCondition::kSingle:
    out.append("kSingle");
    break;
  case ColumnCondition::kComposite:
    out.append("kComposite");
    break;
  }
}

void PrettyPrinter<RangeQueryCriterion::Direction, void>::operator()(
    string &out, RangeQueryCriterion::Direction dir) const {
  switch (dir) {
  case RangeQueryCriterion::FORWARD:
    out.append("FORWARD");
    break;
  case RangeQueryCriterion::BACKWARD:
    out.append("BACKWARD");
    break;
  }
}

void PrettyPrinter<RowUpdateChange::Update::Type, void>::operator()(
    string &out, RowUpdateChange::Update::Type type) const {
  switch (type) {
  case RowUpdateChange::Update::kPut:
    out.append("kPut");
    break;
  case RowUpdateChange::Update::kDelete:
    out.append("kDelete");
    break;
  case RowUpdateChange::Update::kDeleteAll:
    out.append("kDeleteAll");
    break;
  }
}

void PrettyPrinter<Result<std::optional<Row>, OTSError>, void>::operator()(
    string &out, const Result<std::optional<Row>, OTSError> &res) const {
  if (res.ok()) {
    out.append("{\"Ok\":");
    if (!res.okValue()) {
      out.append("null}");
    } else {
      pp::prettyPrint(out, *res.okValue());
      out.push_back('}');
    }
  } else {
    out.append("{\"OTSError\":");
    pp::prettyPrint(out, res.errValue());
    out.push_back('}');
  }
}

} // namespace impl
} // namespace pp

namespace aliyun {
namespace tablestore {
namespace core {

void collectEnum(deque<Action> &xs) {
  xs.push_back(kApi_CreateTable);
  xs.push_back(kApi_ListTable);
  xs.push_back(kApi_DescribeTable);
  xs.push_back(kApi_DeleteTable);
  xs.push_back(kApi_UpdateTable);
  xs.push_back(kApi_GetRow);
  xs.push_back(kApi_PutRow);
  xs.push_back(kApi_UpdateRow);
  xs.push_back(kApi_DeleteRow);
  xs.push_back(kApi_BatchGetRow);
  xs.push_back(kApi_BatchWriteRow);
  xs.push_back(kApi_GetRange);
  xs.push_back(kApi_ComputeSplitsBySize);
}

void collectEnum(deque<BloomFilterType> &xs) {
  xs.push_back(kBFT_None);
  xs.push_back(kBFT_Cell);
  xs.push_back(kBFT_Row);
}

void collectEnum(deque<TableStatus> &xs) {
  xs.push_back(kTS_Active);
  xs.push_back(kTS_Inactive);
  xs.push_back(kTS_Loading);
  xs.push_back(kTS_Unloading);
  xs.push_back(kTS_Updating);
}

int64_t flagDefaultActors = 10;

Endpoint::Endpoint(const string &endpoint, const string &inst)
    : mEndpoint(endpoint), mInstanceName(inst) {}

void Endpoint::reset() {
  mEndpoint.clear();
  mInstanceName.clear();
}

void Endpoint::prettyPrint(string &out) const {
  out.append("{\"Endpoint\":");
  pp::prettyPrint(out, mEndpoint);
  out.append(",\"InstanceName\":");
  pp::prettyPrint(out, mInstanceName);
  out.push_back('}');
}

std::optional<OTSError> Endpoint::validate() const {
  if (mEndpoint.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Endpoint must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  if (!MemPiece::from(mEndpoint).startsWith(MemPiece::from("http://")) &&
      !MemPiece::from(mEndpoint).startsWith(MemPiece::from("https://"))) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Endpoint must starts with either "
                         "\"http://\" or \"https://\".";
    return std::optional<OTSError>(std::move(e));
  }
  if (mInstanceName.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Instance name must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void Credential::reset() {
  mAccessKeyId.clear();
  mAccessKeySecret.clear();
  mSecurityToken.clear();
}

void Credential::prettyPrint(string &out) const {
  out.append("{\"AccessKeyId\":");
  pp::prettyPrint(out, mAccessKeyId);
  out.append(",\"AccessKeySecret\":");
  pp::prettyPrint(out, mAccessKeySecret);
  if (!mSecurityToken.empty()) {
    out.append(",\"SecurityToken\":");
    pp::prettyPrint(out, mSecurityToken);
  }
  out.push_back('}');
}

namespace {

inline bool ContainsCrlf(const string &s) {
  return s.find('\n') != string::npos || s.find('\r') != string::npos;
}

} // namespace

std::optional<OTSError> Credential::validate() const {
  if (mAccessKeyId.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Access-key id must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  if (ContainsCrlf(mAccessKeyId)) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Access-key id must contain neither CR nor LF.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mAccessKeySecret.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Access-key secret must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  if (ContainsCrlf(mAccessKeySecret)) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Access-key secret must contain neither CR nor LF.";
    return std::optional<OTSError>(std::move(e));
  }
  if (ContainsCrlf(mSecurityToken)) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Security token must contain neither CR nor LF.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

namespace {

uint64_t getTrackerBase() {
  Adler32 adler;
  const string &hostname = getHostName();
  FOREACH_ITER(i, hostname) { adler.update(static_cast<uint8_t>(*i)); }
  uint32_t hd = adler.get();
  uint16_t fold = (hd >> 16) ^ hd;
  uint64_t res = fold;
  res <<= 48;
  return res;
}

} // namespace

Tracker Tracker::create(Random &rng) {
  static uint64_t sBase = getTrackerBase();
  uint64_t res = random::nextInt(rng, 0x1000000000000ul);
  res |= sBase;
  string s;
  base57encode(s, res);
  return Tracker(s);
}

void Tracker::calculateHash() {
  std::hash<string> hasher;
  mTraceHash = hasher(mTraceId);
}

std::optional<OTSError> Tracker::validate() const {
  return std::optional<OTSError>();
}

void Tracker::prettyPrint(string &out) const { pp::prettyPrint(out, mTraceId); }

ClientOptions::ClientOptions() { reset(); }

void ClientOptions::reset() {
  mMaxConnections = 5000;
  mConnectTimeout = Duration::fromSec(3);
  mRequestTimeout = Duration::fromSec(3);

  mRetryStrategy.reset(new DeadlineRetryStrategy(
      shared_ptr<Random>(random::newDefault()), Duration::fromSec(10)));

  mActors.clear();
  for (int64_t i = 0; i < 10; ++i) {
    mActors.push_back(shared_ptr<Actor>(new Actor()));
  }

  mLogger.reset(createLogger("/", Logger::kInfo));
}

void ClientOptions::prettyPrint(string &out) const {
  out.append("{\"MaxConnections\":");
  pp::prettyPrint(out, maxConnections());
  out.append(",\"ConnectTimeout\":");
  pp::prettyPrint(out, connectTimeout());
  out.append(",\"RequestTimeout\":");
  pp::prettyPrint(out, requestTimeout());
  out.append(",\"RetryStrategy\":");
  pp::prettyPrint(out, string(typeid(*mRetryStrategy).name()));
  out.append(",\"Actors\":");
  pp::prettyPrint(out, mActors.size());
  out.push_back('}');
}

std::optional<OTSError> ClientOptions::validate() const {
  if (maxConnections() <= 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "MaxConnections must be positive.";
    return std::optional<OTSError>(std::move(e));
  }
  if (connectTimeout() < Duration::fromMsec(1)) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "ConnectTimeout must be greater than 1 msec. "
                         "Recommends 2 secs.";
    return std::optional<OTSError>(std::move(e));
  }
  if (requestTimeout() < Duration::fromMsec(1)) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "RequestTimeout must be greater than 1 msec. "
                         "Recommends 10 secs.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mRetryStrategy.get() == NULL) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "RetryStrategy is required.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void ClientOptions::resetRetryStrategy(RetryStrategy *rs) {
  mRetryStrategy.reset(rs);
}

RetryStrategy *ClientOptions::releaseRetryStrategy() {
  return mRetryStrategy.release();
}

void ClientOptions::resetLogger(Logger *logger) { mLogger.reset(logger); }

Logger *ClientOptions::releaseLogger() { return mLogger.release(); }

void PrimaryKeyColumnSchema::reset() {
  mName.clear();
  mType = kPKT_Integer;
  mOption = std::optional<Option>();
}

std::optional<OTSError> PrimaryKeyColumnSchema::validate() const {
  if (name().empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "\"name\" is required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mOption) {
    if (*mOption == AutoIncrement && type() != kPKT_Integer) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      string msg("AutoIncrement can only be applied on kPKT_Integer, for "
                 "primary key \"");
      msg.append(name());
      msg.append("\".");
      e.mutableMessage() = msg;
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

void PrimaryKeyColumnSchema::prettyPrint(string &out) const {
  out.push_back('{');
  pp::prettyPrint(out, mName);
  out.push_back(':');
  pp::prettyPrint(out, mType);
  if (mOption) {
    out.push_back('+');
    pp::prettyPrint(out, *mOption);
  }
  out.push_back('}');
}

void Schema::prettyPrint(string &out) const { pp::prettyPrint(out, mColumns); }

std::optional<OTSError> Schema::validate() const {
  if (size() == 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table schema must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  for (int64_t i = 0, sz = size(); i < sz; ++i) {
    TRY((*this)[i].validate());
  }
  return std::optional<OTSError>();
}

PrimaryKeyType PrimaryKeyValue::toPrimaryKeyType(Category cat) {
  switch (cat) {
  case kInteger:
    return kPKT_Integer;
  case kString:
    return kPKT_String;
  case kBinary:
    return kPKT_Binary;
  case kNone:
  case kInfMin:
  case kInfMax:
  case kAutoIncr:
    OTS_ASSERT(false)(cat);
  }
  return kPKT_Integer;
}

PrimaryKeyValue::PrimaryKeyValue() : mCategory(kNone), mIntValue(0) {}

PrimaryKeyValue::Category PrimaryKeyValue::category() const {
  return mCategory;
}

void PrimaryKeyValue::prettyPrint(string &out) const {
  switch (category()) {
  case kNone:
    out.append("none");
    break;
  case kInfMin:
    out.append("-inf");
    break;
  case kInfMax:
    out.append("+inf");
    break;
  case kAutoIncr:
    out.append("auto-incr");
    break;
  case kInteger:
    pp::prettyPrint(out, integer());
    break;
  case kString:
    pp::prettyPrint(out, str());
    break;
  case kBinary:
    pp::prettyPrint(out, MemPiece::from(blob()));
    break;
  }
}

std::optional<OTSError> PrimaryKeyValue::validate() const {
  if (category() == kNone) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "value is required.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void PrimaryKeyValue::reset() {
  mCategory = kNone;
  mIntValue = 0;
  mStrBlobValue.clear();
}

CompareResult PrimaryKeyValue::compare(const PrimaryKeyValue &b) const {
  OTS_ASSERT(category() != kNone);
  OTS_ASSERT(b.category() != kNone);

  if (category() == kInfMin) {
    if (b.category() == kInfMin) {
      return kCR_Uncomparable;
    } else {
      return kCR_Smaller;
    }
  } else if (b.category() == kInfMin) {
    return kCR_Larger;
  }

  if (category() == kInfMax) {
    if (b.category() == kInfMax) {
      return kCR_Uncomparable;
    } else {
      return kCR_Larger;
    }
  } else if (b.category() == kInfMax) {
    return kCR_Smaller;
  }

  if (category() == kAutoIncr) {
    return kCR_Uncomparable;
  } else if (b.category() == kAutoIncr) {
    return kCR_Uncomparable;
  }

  if (category() != b.category()) {
    return kCR_Uncomparable;
  }

  switch (category()) {
  case kInteger:
    if (integer() < b.integer()) {
      return kCR_Smaller;
    } else if (integer() > b.integer()) {
      return kCR_Larger;
    } else {
      return kCR_Equivalent;
    }
  case kString: {
    int c = lexicographicOrder(MemPiece::from(str()), MemPiece::from(b.str()));
    if (c < 0) {
      return kCR_Smaller;
    } else if (c > 0) {
      return kCR_Larger;
    } else {
      return kCR_Equivalent;
    }
  }
  case kBinary: {
    int c =
        lexicographicOrder(MemPiece::from(blob()), MemPiece::from(b.blob()));
    if (c < 0) {
      return kCR_Smaller;
    } else if (c > 0) {
      return kCR_Larger;
    } else {
      return kCR_Equivalent;
    }
  }
  case kNone:
  case kInfMin:
  case kInfMax:
  case kAutoIncr:
    OTS_ASSERT(false)(category());
  }
  return kCR_Uncomparable;
}

PrimaryKeyValue::PrimaryKeyValue(int64_t x)
    : mCategory(kInteger), mIntValue(x) {}

PrimaryKeyValue PrimaryKeyValue::toInteger(int64_t x) {
  return PrimaryKeyValue(x);
}

int64_t PrimaryKeyValue::integer() const {
  OTS_ASSERT(category() == kInteger)(category());
  return mIntValue;
}

int64_t &PrimaryKeyValue::mutableInteger() {
  reset();
  mCategory = kInteger;
  return mIntValue;
}

PrimaryKeyValue::PrimaryKeyValue(Str, const string &s)
    : mCategory(kString), mIntValue(0), mStrBlobValue(s) {}

PrimaryKeyValue PrimaryKeyValue::toStr(const string &s) {
  return PrimaryKeyValue(Str(), s);
}

const string &PrimaryKeyValue::str() const {
  OTS_ASSERT(category() == kString)(category());
  return mStrBlobValue;
}

string &PrimaryKeyValue::mutableStr() {
  reset();
  mCategory = kString;
  return mStrBlobValue;
}

PrimaryKeyValue::PrimaryKeyValue(Bin, const string &s)
    : mCategory(kBinary), mIntValue(0), mStrBlobValue(s) {}

PrimaryKeyValue PrimaryKeyValue::toBlob(const string &b) {
  return PrimaryKeyValue(Bin(), b);
}

const string &PrimaryKeyValue::blob() const {
  OTS_ASSERT(category() == kBinary)(category());
  return mStrBlobValue;
}

string &PrimaryKeyValue::mutableBlob() {
  reset();
  mCategory = kBinary;
  return mStrBlobValue;
}

PrimaryKeyValue::PrimaryKeyValue(InfMax) : mCategory(kInfMax), mIntValue(0) {}

PrimaryKeyValue PrimaryKeyValue::toInfMax() {
  return PrimaryKeyValue(InfMax());
}

bool PrimaryKeyValue::isInfMax() const { return category() == kInfMax; }

void PrimaryKeyValue::setInfMax() {
  PrimaryKeyValue to = PrimaryKeyValue::toInfMax();
  *this = std::move(to);
}

PrimaryKeyValue::PrimaryKeyValue(InfMin) : mCategory(kInfMin), mIntValue(0) {}

PrimaryKeyValue PrimaryKeyValue::toInfMin() {
  return PrimaryKeyValue(InfMin());
}

bool PrimaryKeyValue::isInfMin() const { return category() == kInfMin; }

void PrimaryKeyValue::setInfMin() {
  PrimaryKeyValue to = PrimaryKeyValue::toInfMin();
  *this = std::move(to);
}

PrimaryKeyValue::PrimaryKeyValue(AutoIncrement)
    : mCategory(kAutoIncr), mIntValue(0) {}

PrimaryKeyValue PrimaryKeyValue::toAutoIncrement() {
  return PrimaryKeyValue(AutoIncrement());
}

bool PrimaryKeyValue::isAutoIncrement() const {
  return category() == kAutoIncr;
}

void PrimaryKeyValue::setAutoIncrement() {
  PrimaryKeyValue to = PrimaryKeyValue::toAutoIncrement();
  *this = std::move(to);
}

std::optional<OTSError> PrimaryKeyColumn::validate() const {
  if (mName.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "name of primary-key column is required.";
    return std::optional<OTSError>(std::move(e));
  }
  std::optional<OTSError> err = value().validate();
  if (err) {
    string msg("For primary-key column \"");
    msg.append(mName);
    msg.append("\", ");
    msg.append(err->message());
    err->mutableMessage() = msg;
    return err;
  }
  return std::optional<OTSError>();
}

PrimaryKeyColumn::PrimaryKeyColumn() {}

PrimaryKeyColumn::PrimaryKeyColumn(const string &name, const PrimaryKeyValue &v)
    : mName(name), mValue(v) {}

void PrimaryKeyColumn::reset() {
  mName.clear();
  mValue.reset();
}

void PrimaryKeyColumn::prettyPrint(string &out) const {
  pp::prettyPrint(out, mName);
  out.push_back(':');
  pp::prettyPrint(out, mValue);
}

bool PrimaryKeyColumn::operator==(const PrimaryKeyColumn &a) const {
  if (name() != a.name()) {
    return false;
  }
  if (value() != a.value()) {
    return false;
  }
  return true;
}

void PrimaryKey::prettyPrint(string &out) const {
  if (mColumns.size() == 0) {
    out.append("{}");
    return;
  }
  out.push_back('{');
  pp::prettyPrint(out, mColumns[0]);
  for (int64_t i = 1, sz = mColumns.size(); i < sz; ++i) {
    out.push_back(',');
    pp::prettyPrint(out, mColumns[i]);
  }
  out.push_back('}');
}

std::optional<OTSError> PrimaryKey::validate() const {
  if (size() == 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Primary key is required.";
    return std::optional<OTSError>(std::move(e));
  }
  for (int64_t i = 0, sz = size(); i < sz; ++i) {
    TRY((*this)[i].validate());
  }
  return std::optional<OTSError>();
}

CompareResult PrimaryKey::compare(const PrimaryKey &b) const {
  int64_t asz = size();
  int64_t bsz = b.size();
  if (asz != bsz) {
    return kCR_Uncomparable;
  }
  for (int64_t i = 0; i < asz; ++i) {
    CompareResult c = (*this)[i].value().compare(b[i].value());
    if (c != kCR_Equivalent) {
      return c;
    }
  }
  return kCR_Equivalent;
}

void TableMeta::reset() {
  mTableName.clear();
  mSchema.reset();
}

void TableMeta::prettyPrint(string &out) const {
  out.append("{\"TableName\":");
  pp::prettyPrint(out, mTableName);
  out.append(",\"Schema\":");
  pp::prettyPrint(out, mSchema);
  out.push_back('}');
}

std::optional<OTSError> TableMeta::validate() const {
  if (tableName().empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name is required.";
    return std::optional<OTSError>(std::move(e));
  }
  TRY(schema().validate());
  return std::optional<OTSError>();
}

void TableOptions::reset() {
  mReservedThroughput.reset();
  mTimeToLive.reset();
  mMaxVersions.reset();
  mBloomFilterType.reset();
  mBlockSize.reset();
  mMaxTimeDeviation.reset();
}

void TableOptions::prettyPrint(string &out) const {
  out.push_back('{');
  bool first = true;
  if (mReservedThroughput) {
    first = false;
    out.append("\"ReservedThroughput\":");
    pp::prettyPrint(out, *mReservedThroughput);
  }
  if (mTimeToLive) {
    if (first) {
      first = false;
    } else {
      out.push_back(',');
    }
    out.append("\"TimeToLive\":");
    pp::prettyPrint(out, mTimeToLive->toSec());
  }
  if (mMaxVersions) {
    if (first) {
      first = false;
    } else {
      out.push_back(',');
    }
    out.append("\"MaxVersions\":");
    pp::prettyPrint(out, *mMaxVersions);
  }
  if (mBloomFilterType) {
    if (first) {
      first = false;
    } else {
      out.push_back(',');
    }
    out.append("\"BloomFilterType\":");
    pp::prettyPrint(out, *mBloomFilterType);
  }
  if (mBlockSize) {
    if (first) {
      first = false;
    } else {
      out.push_back(',');
    }
    out.append("\"BlockSize\":");
    pp::prettyPrint(out, *mBlockSize);
  }
  if (mMaxTimeDeviation) {
    if (first) {
      first = false;
    } else {
      out.push_back(',');
    }
    out.append("\"MaxTimeDeviation\":");
    pp::prettyPrint(out, mMaxTimeDeviation->toSec());
  }
  out.push_back('}');
}

std::optional<OTSError> TableOptions::validate() const {
  if (reservedThroughput()) {
    TRY(reservedThroughput()->validate());
    if (!reservedThroughput()->read()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Read reserved throughput is required.";
      return std::optional<OTSError>(std::move(e));
    }
    if (!reservedThroughput()->write()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Write reserved throughput is required.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  if (mTimeToLive) {
    if (mTimeToLive->toUsec() % kUsecPerSec != 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "TimeToLive must be integral multiple of seconds.";
      return std::optional<OTSError>(std::move(e));
    }
    if (mTimeToLive->toUsec() <= 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "TimeToLive must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  if (mMaxVersions) {
    if (*mMaxVersions <= 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "MaxVersions must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  if (mBlockSize) {
    if (*mBlockSize <= 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "BlockSize must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  if (mMaxTimeDeviation) {
    if (mMaxTimeDeviation->toUsec() % kUsecPerSec != 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() =
          "MaxTimeDeviation must be integral multiple of seconds.";
      return std::optional<OTSError>(std::move(e));
    }
    if (mMaxTimeDeviation->toUsec() <= 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "MaxTimeDeviation must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

void CapacityUnit::reset() {
  mRead.reset();
  mWrite.reset();
}

std::optional<OTSError> CapacityUnit::validate() const {
  if (mRead) {
    if (*mRead < 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "read capacity unit must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  if (mWrite) {
    if (*mWrite < 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "write capacity unit must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

void CapacityUnit::prettyPrint(string &out) const {
  out.push_back('{');
  bool first = true;
  if (mRead) {
    first = false;
    out.append("\"Read\":");
    pp::prettyPrint(out, *mRead);
  }
  if (mWrite) {
    if (first) {
      first = false;
    } else {
      out.push_back(',');
    }
    out.append("\"Write\":");
    pp::prettyPrint(out, *mWrite);
  }
  out.push_back('}');
}

AttributeValue::AttributeValue()
    : mCategory(kNone), mIntValue(0), mBoolValue(false), mFloatingValue(0) {}

void AttributeValue::reset() {
  mCategory = kNone;
  mIntValue = 0;
  mStrBlobValue.clear();
  mBoolValue = false;
  mFloatingValue = 0;
}

void AttributeValue::prettyPrint(string &out) const {
  switch (category()) {
  case kNone:
    out.append("none");
    break;
  case kString:
    pp::prettyPrint(out, str());
    break;
  case kBinary:
    pp::prettyPrint(out, MemPiece::from(blob()));
    break;
  case kInteger:
    pp::prettyPrint(out, integer());
    break;
  case kBoolean:
    if (boolean()) {
      out.append("true");
    } else {
      out.append("false");
    }
    break;
  case kFloatPoint:
    pp::prettyPrint(out, floatPoint());
    break;
  }
}

std::optional<OTSError> AttributeValue::validate() const {
  if (category() == kNone) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "value is required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (category() == kFloatPoint) {
    double v = floatPoint();
    if (isinf(v)) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "value cannot be set to infinity.";
      return std::optional<OTSError>(std::move(e));
    }
    if (isnan(v)) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "value cannot be set to NaN.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

CompareResult AttributeValue::compare(const AttributeValue &b) const {
  if (category() == kNone) {
    if (b.category() == kNone) {
      return kCR_Equivalent;
    } else {
      return kCR_Uncomparable;
    }
  } else if (b.category() == kNone) {
    return kCR_Uncomparable;
  }

  if (category() != b.category()) {
    return kCR_Uncomparable;
  }

  switch (category()) {
  case kInteger:
    if (integer() < b.integer()) {
      return kCR_Smaller;
    } else if (integer() > b.integer()) {
      return kCR_Larger;
    } else {
      return kCR_Equivalent;
    }
  case kBoolean: {
    if (boolean() == b.boolean()) {
      return kCR_Equivalent;
    } else if (boolean()) {
      return kCR_Larger;
    } else {
      return kCR_Smaller;
    }
  }
  case kFloatPoint: {
    if (floatPoint() == b.floatPoint()) {
      return kCR_Equivalent;
    } else if (floatPoint() < b.floatPoint()) {
      return kCR_Smaller;
    } else {
      return kCR_Larger;
    }
  }
  case kString: {
    int c = lexicographicOrder(MemPiece::from(str()), MemPiece::from(b.str()));
    if (c < 0) {
      return kCR_Smaller;
    } else if (c > 0) {
      return kCR_Larger;
    } else {
      return kCR_Equivalent;
    }
  }
  case kBinary: {
    int c =
        lexicographicOrder(MemPiece::from(blob()), MemPiece::from(b.blob()));
    if (c < 0) {
      return kCR_Smaller;
    } else if (c > 0) {
      return kCR_Larger;
    } else {
      return kCR_Equivalent;
    }
  }
  case kNone:
    OTS_ASSERT(false)(category());
  }
  return kCR_Uncomparable;
}

AttributeValue::AttributeValue(Str, const string &strblob)
    : mCategory(kString), mIntValue(0), mStrBlobValue(strblob),
      mBoolValue(false), mFloatingValue(0) {}

AttributeValue AttributeValue::toStr(const string &a) {
  return AttributeValue(Str(), a);
}

const string &AttributeValue::str() const {
  OTS_ASSERT(category() == kString)(category());
  return mStrBlobValue;
}

string &AttributeValue::mutableStr() {
  AttributeValue empty;
  *this = std::move(empty);
  mCategory = kString;
  return mStrBlobValue;
}

AttributeValue::AttributeValue(Blob, const string &strblob)
    : mCategory(kBinary), mIntValue(0), mStrBlobValue(strblob),
      mBoolValue(false), mFloatingValue(0) {}

AttributeValue AttributeValue::toBlob(const string &a) {
  return AttributeValue(Blob(), a);
}

const string &AttributeValue::blob() const {
  OTS_ASSERT(category() == kBinary)(category());
  return mStrBlobValue;
}

string &AttributeValue::mutableBlob() {
  AttributeValue empty;
  *this = std::move(empty);
  mCategory = kBinary;
  return mStrBlobValue;
}

AttributeValue::AttributeValue(int64_t v)
    : mCategory(kInteger), mIntValue(v), mBoolValue(false), mFloatingValue(0) {}

AttributeValue AttributeValue::toInteger(int64_t v) {
  return AttributeValue(v);
}

int64_t AttributeValue::integer() const {
  OTS_ASSERT(category() == kInteger)(category());
  return mIntValue;
}

int64_t &AttributeValue::mutableInteger() {
  AttributeValue empty;
  *this = std::move(empty);
  mCategory = kInteger;
  return mIntValue;
}

AttributeValue::AttributeValue(double v)
    : mCategory(kFloatPoint), mIntValue(0), mBoolValue(false),
      mFloatingValue(v) {}

AttributeValue AttributeValue::toFloatPoint(double v) {
  return AttributeValue(v);
}

double AttributeValue::floatPoint() const {
  OTS_ASSERT(category() == kFloatPoint)(category());
  return mFloatingValue;
}

double &AttributeValue::mutableFloatPoint() {
  AttributeValue empty;
  *this = std::move(empty);
  mCategory = kFloatPoint;
  return mFloatingValue;
}

AttributeValue::AttributeValue(bool v)
    : mCategory(kBoolean), mIntValue(0), mBoolValue(v), mFloatingValue(0) {}

AttributeValue AttributeValue::toBoolean(bool v) { return AttributeValue(v); }

bool AttributeValue::boolean() const {
  OTS_ASSERT(category() == kBoolean)(category());
  return mBoolValue;
}

bool &AttributeValue::mutableBoolean() {
  AttributeValue empty;
  *this = std::move(empty);
  mCategory = kBoolean;
  return mBoolValue;
}

Attribute::Attribute(const string &name, const AttributeValue &val)
    : mName(name), mValue(val) {}

Attribute::Attribute(const string &name, const AttributeValue &val, UtcTime ts)
    : mName(name), mValue(val), mTimestamp(ts) {}

void Attribute::reset() {
  mName.clear();
  mValue.reset();
  mTimestamp.reset();
}

bool Attribute::operator==(const Attribute &a) const {
  if (name() != a.name()) {
    return false;
  }
  if (value() != a.value()) {
    return false;
  }
  if (timestamp() != a.timestamp()) {
    return false;
  }
  return true;
}

void Attribute::prettyPrint(string &out) const {
  out.append("{\"Name\":");
  pp::prettyPrint(out, mName);
  out.append(",\"Value\":");
  pp::prettyPrint(out, mValue);
  if (mTimestamp) {
    out.append(",\"Timestamp\":");
    pp::prettyPrint(out, *mTimestamp);
  }
  out.push_back('}');
}

std::optional<OTSError> Attribute::validate() const {
  if (name().empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Attribute name must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  {
    std::optional<OTSError> err = value().validate();
    if (err) {
      string msg("For column ");
      pp::prettyPrint(msg, name());
      msg.append(", ");
      msg.append(err->message());
      err->mutableMessage() = msg;
      return err;
    }
  }
  if (timestamp()) {
    if (timestamp()->toMsec() < 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Timestamp of column ";
      pp::prettyPrint(e.mutableMessage(), name());
      e.mutableMessage().append(" must be positive.");
      return std::optional<OTSError>(std::move(e));
    }
    if (timestamp()->toUsec() % kUsecPerMsec != 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Timestamp of column ";
      pp::prettyPrint(e.mutableMessage(), name());
      e.mutableMessage().append(" must be multiple of milliseconds.");
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

bool Row::operator==(const Row &a) const {
  if (mPkey != a.mPkey) {
    return false;
  }
  if (mAttrs != a.mAttrs) {
    return false;
  }
  return true;
}

void Row::reset() {
  mPkey.reset();
  mAttrs.reset();
}

void Row::prettyPrint(string &out) const {
  out.append("{\"PrimaryKey\":");
  pp::prettyPrint(out, mPkey);
  out.append(",\"Attributes\":");
  pp::prettyPrint(out, mAttrs);
  out.push_back('}');
}

std::optional<OTSError> Row::validate() const {
  TRY(mPkey.validate());
  for (int64_t i = 0, sz = mAttrs.size(); i < sz; ++i) {
    TRY(mAttrs[i].validate());
  }
  return std::optional<OTSError>();
}

void TimeRange::reset() {
  mStart = UtcTime();
  mEnd = UtcTime();
}

void TimeRange::prettyPrint(string &out) const {
  out.push_back('[');
  pp::prettyPrint(out, mStart);
  out.push_back(',');
  pp::prettyPrint(out, mEnd);
  out.push_back(']');
}

std::optional<OTSError> TimeRange::validate() const {
  if (mStart.toUsec() % kUsecPerMsec != 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Start of time ranges must be integral "
                         "multiple of milliseconds.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mEnd.toUsec() % kUsecPerMsec != 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "End of time ranges must be integral multiple "
                         "of milliseconds.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mStart > mEnd) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Start of time ranges must be in advance of "
                         "their ends.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void Split::reset() {
  mLowerBound.reset();
  mUpperBound.reset();
  mLocation.clear();
}

void Split::prettyPrint(string &out) const {
  out.append("{\"Location\":");
  pp::prettyPrint(out, mLocation);
  if (mLowerBound.get() != NULL) {
    out.append(",\"LowerBound\":");
    pp::prettyPrint(out, *mLowerBound);
  }
  if (mUpperBound.get() != NULL) {
    out.append(",\"UpperBound\":");
    pp::prettyPrint(out, *mUpperBound);
  }
  out.push_back('}');
}

std::optional<OTSError> Split::validate() const {
  if (mLowerBound.get() == NULL) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Lower bound of a split must be nonnull.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mUpperBound.get() == NULL) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Upper bound of a split must be nonnull.";
    return std::optional<OTSError>(std::move(e));
  }
  TRY(mLowerBound->validate());
  TRY(mUpperBound->validate());
  if (mLowerBound->size() != mUpperBound->size()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Lower bound of a split must be "
                         "of the same length of the upper bound of that split.";
    return std::optional<OTSError>(std::move(e));
  }
  for (int64_t i = 0, sz = mLowerBound->size(); i < sz; ++i) {
    const PrimaryKeyColumn &lower = (*mLowerBound)[i];
    const PrimaryKeyColumn &upper = (*mUpperBound)[i];
    if (lower.name() != upper.name()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Lower bound of a split must have the same names of "
                           "the upper bound of that split.";
      return std::optional<OTSError>(std::move(e));
    }
    if (lower.value().category() != upper.value().category()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Lower bound of a split must have the same types of "
                           "the upper bound of that split.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  {
    CompareResult c = mLowerBound->compare(*mUpperBound);
    if (c == kCR_Larger || c == kCR_Equivalent) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Lower bound of a split must be smaller than the "
                           "upper bound of that split.";
      return std::optional<OTSError>(std::move(e));
    } else if (c == kCR_Uncomparable) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() =
          "Lower bound of a split must be comparable with the upper bound.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

void Response::reset() {
  mRequestId.clear();
  mTraceId.clear();
}

void Response::prettyPrint(string &out) const {
  if (!mRequestId.empty()) {
    out.append(",\"RequestId\":");
    pp::prettyPrint(out, mRequestId);
  }
  if (!mTraceId.empty()) {
    out.append(",\"TraceId\":");
    pp::prettyPrint(out, mTraceId);
  }
}

CreateTableRequest::CreateTableRequest()
    : mMeta(), mOptions(), mShardSplitPoints() {
  CapacityUnit tmp(0, 0);
  mutableOptions().mutableReservedThroughput().emplace(std::move(tmp));
  mutableOptions().mutableMaxVersions().emplace(1);
}

void CreateTableRequest::reset() {
  mMeta.reset();
  mOptions.reset();
  mShardSplitPoints.reset();
}

void CreateTableRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"CreateTableRequest\",\"Meta\":");
  pp::prettyPrint(out, mMeta);
  out.append(",\"Options\":");
  pp::prettyPrint(out, mOptions);
  out.append(",\"ShardSplitPoints\":");
  pp::prettyPrint(out, mShardSplitPoints);
  out.push_back('}');
}

std::optional<OTSError> CreateTableRequest::validate() const {
  TRY(meta().validate());
  TRY(options().validate());
  if (!options().reservedThroughput()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Both read and write capacity units are required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (!options().reservedThroughput()->read()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Both read and write capacity units are required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (!options().reservedThroughput()->write()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Both read and write capacity units are required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (*options().reservedThroughput()->read() < 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Read capacity units must be positive.";
    return std::optional<OTSError>(std::move(e));
  }
  if (*options().reservedThroughput()->write() < 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Write capacity units must be positive.";
    return std::optional<OTSError>(std::move(e));
  }
  if (!options().maxVersions()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "MaxVersions is missing while creating table.";
    return std::optional<OTSError>(std::move(e));
  }
  for (int64_t i = 0, sz = mShardSplitPoints.size(); i < sz; ++i) {
    const PrimaryKey &pk = mShardSplitPoints[i];
    TRY(pk.validate());
  }
  for (int64_t i = 0, sz = mShardSplitPoints.size(); i < sz; ++i) {
    const PrimaryKey &pk = mShardSplitPoints[i];
    if (pk.size() != 1) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Length of shard split points must be exactly one.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  for (int64_t i = 0, sz = mShardSplitPoints.size(); i < sz; ++i) {
    const PrimaryKey &pk = mShardSplitPoints[i];
    OTS_ASSERT(pk.size() >= 1)(pk.size());
    const PrimaryKeyColumn &pkc = pk[0];
    if (!pkc.value().isReal()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Shard split points contains an unreal value type ";
      pp::prettyPrint(e.mutableMessage(), pkc.value().category());
      e.mutableMessage().push_back('.');
      return std::optional<OTSError>(std::move(e));
    }
    const Schema &schema = mMeta.schema();
    OTS_ASSERT(schema.size() >= 1)
    (schema.size());
    const PrimaryKeyColumnSchema &colSchema = schema[0];
    if (MemPiece::from(pkc.name()) != MemPiece::from(colSchema.name())) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Shard split points contains ";
      pp::prettyPrint(e.mutableMessage(), pkc.name());
      e.mutableMessage().append(
          ", which is different with that in the schema.");
      return std::optional<OTSError>(std::move(e));
    }
    if (PrimaryKeyValue::toPrimaryKeyType(pkc.value().category()) !=
        colSchema.type()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Type of primary-key column ";
      pp::prettyPrint(e.mutableMessage(), pkc.name());
      e.mutableMessage().append(" mismatches that in schema.");
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

void CreateTableResponse::reset() { Response::reset(); }

void CreateTableResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"CreateTableResponse\"");
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> CreateTableResponse::validate() const {
  return std::optional<OTSError>();
}

void ListTableRequest::reset() {}

void ListTableRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"ListTableRequest\"}");
}

std::optional<OTSError> ListTableRequest::validate() const {
  return std::optional<OTSError>();
}

void ListTableResponse::reset() {
  Response::reset();
  mTables.reset();
}

void ListTableResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"ListTableResponse\",\"Tables\":");
  pp::prettyPrint(out, mTables);
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> ListTableResponse::validate() const {
  return std::optional<OTSError>();
}

void DeleteTableRequest::reset() { mTable.clear(); }

void DeleteTableRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"DeleteTableRequest\",\"Table\":");
  pp::prettyPrint(out, mTable);
  out.push_back('}');
}

std::optional<OTSError> DeleteTableRequest::validate() const {
  if (table().empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void DeleteTableResponse::reset() { Response::reset(); }

void DeleteTableResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"DeleteTableResponse\"");
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> DeleteTableResponse::validate() const {
  return std::optional<OTSError>();
}

void DescribeTableRequest::reset() { mTable.clear(); }

void DescribeTableRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"DescribeTableRequest\",\"Table\":");
  pp::prettyPrint(out, table());
  out.push_back('}');
}

std::optional<OTSError> DescribeTableRequest::validate() const {
  if (table().empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void DescribeTableResponse::reset() {
  Response::reset();
  mMeta.reset();
  mOptions.reset();
  mStatus = kTS_Active;
  mShardSplitPoints.reset();
}

void DescribeTableResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"DescribeTableResponse\",\"TableMeta\":");
  pp::prettyPrint(out, mMeta);
  out.append(",\"TableOptions\":");
  pp::prettyPrint(out, mOptions);
  out.append(",\"TableStatus\":");
  pp::prettyPrint(out, mStatus);
  out.append(",\"ShardSplitPoints\":");
  pp::prettyPrint(out, mShardSplitPoints);
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> DescribeTableResponse::validate() const {
  TRY(mMeta.validate());
  TRY(mOptions.validate());
  for (int64_t i = 0, sz = mShardSplitPoints.size(); i < sz; ++i) {
    TRY(mShardSplitPoints[i].validate());
  }
  return std::optional<OTSError>();
}

void UpdateTableRequest::reset() {
  mTable.clear();
  mOptions.reset();
}

void UpdateTableRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"UpdateTableRequest\",\"TableName\":");
  pp::prettyPrint(out, mTable);
  out.append(",\"TableOptions\":");
  pp::prettyPrint(out, mOptions);
  out.push_back('}');
}

std::optional<OTSError> UpdateTableRequest::validate() const {
  if (mTable.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name is required.";
    return std::optional<OTSError>(std::move(e));
  }
  TRY(mOptions.validate());
  return std::optional<OTSError>();
}

void UpdateTableResponse::reset() { Response::reset(); }

void UpdateTableResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"UpdateTableResponse\"");
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> UpdateTableResponse::validate() const {
  return std::optional<OTSError>();
}

void ComputeSplitsBySizeRequest::reset() {
  mTable.clear();
  mSplitSize = kDefaultSplitSize;
}

void ComputeSplitsBySizeRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"ComputeSplitsBySizeRequest\",\"TableName\":");
  pp::prettyPrint(out, mTable);
  out.append(",\"SplitSize\":");
  pp::prettyPrint(out, mSplitSize);
  out.push_back('}');
}

std::optional<OTSError> ComputeSplitsBySizeRequest::validate() const {
  if (mTable.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name must be nonempty.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mSplitSize <= 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Split size must be positive.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

void ComputeSplitsBySizeResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mSchema.reset();
  mSplits.reset();
}

void ComputeSplitsBySizeResponse::prettyPrint(string &out) const {
  out.append(
      "{\"API\":\"ComputeSplitsBySizeResponse\",\"ConsumedCapacityUnit\":");
  pp::prettyPrint(out, mConsumedCapacity);
  out.append(",\"Schema\":");
  pp::prettyPrint(out, mSchema);
  out.append(",\"Splits\":");
  pp::prettyPrint(out, mSplits);
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> ComputeSplitsBySizeResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  TRY(mSchema.validate());
  for (int64_t i = 0, sz = mSplits.size(); i < sz; ++i) {
    TRY(mSplits[i].validate());
  }
  return std::optional<OTSError>();
}

bool Condition::operator==(const Condition &b) const {
  if (rowCondition() != b.rowCondition()) {
    return false;
  }
  if ((columnCondition().get() == NULL) !=
      (b.columnCondition().get() == NULL)) {
    return false;
  } else if (columnCondition().get() != NULL &&
             *columnCondition() != *b.columnCondition()) {
    return false;
  }
  return true;
}

void Condition::reset() {
  mRowCondition = kIgnore;
  mColumnCondition.reset();
}

void Condition::prettyPrint(string &out) const {
  out.append("{\"RowCondition\":");
  pp::prettyPrint(out, mRowCondition);
  if (mColumnCondition.get() != NULL) {
    out.append(",\"ColumnCondition\":");
    pp::prettyPrint(out, *mColumnCondition);
  }
  out.push_back('}');
}

std::optional<OTSError> Condition::validate() const {
  if (mColumnCondition.get() != NULL) {
    TRY(mColumnCondition->validate());
  }
  return std::optional<OTSError>();
}

bool operator==(const ColumnCondition &a, const ColumnCondition &b) {
  if (a.type() != b.type()) {
    return false;
  }
  switch (a.type()) {
  case ColumnCondition::kSingle:
    return static_cast<const SingleColumnCondition &>(a).operator==(
        static_cast<const SingleColumnCondition &>(b));
  case ColumnCondition::kComposite:
    return static_cast<const CompositeColumnCondition &>(a).operator==(
        static_cast<const CompositeColumnCondition &>(b));
  }
  OTS_ASSERT(false);
  return true;
}

bool SingleColumnCondition::operator==(const SingleColumnCondition &b) const {
  if (columnName() != b.columnName()) {
    return false;
  }
  if (relation() != b.relation()) {
    return false;
  }
  if (columnValue() != b.columnValue()) {
    return false;
  }
  if (passIfMissing() != b.passIfMissing()) {
    return false;
  }
  if (latestVersionOnly() != b.latestVersionOnly()) {
    return false;
  }
  return true;
}

void SingleColumnCondition::reset() {
  mColumnName.clear();
  mRelation = kEqual;
  mColumnValue.reset();
  mPassIfMissing = false;
  mLatestVersionOnly = true;
}

void SingleColumnCondition::prettyPrint(string &out) const {
  out.append("{\"Relation\":");
  pp::prettyPrint(out, mRelation);
  out.append(",\"ColumnName\":");
  pp::prettyPrint(out, mColumnName);
  out.append(",\"ColumnValue\":");
  pp::prettyPrint(out, mColumnValue);
  out.append(",\"PassIfMissing\":");
  pp::prettyPrint(out, mPassIfMissing);
  out.append(",\"LatestVersionOnly\":");
  pp::prettyPrint(out, mLatestVersionOnly);
  out.push_back('}');
}

std::optional<OTSError> SingleColumnCondition::validate() const {
  if (mColumnName.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Column name is required.";
    return std::optional<OTSError>(std::move(e));
  }
  TRY(mColumnValue.validate());
  if (mColumnValue.category() == AttributeValue::kNone) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Column value is required.";
    return std::optional<OTSError>(std::move(e));
  }
  return std::optional<OTSError>();
}

bool CompositeColumnCondition::operator==(
    const CompositeColumnCondition &b) const {
  if (op() != b.op()) {
    return false;
  }
  if (children().size() != b.children().size()) {
    return false;
  }
  for (int64_t i = 0, sz = children().size(); i < sz; ++i) {
    if (*children()[i] != *b.children()[i]) {
      return false;
    }
  }
  return true;
}

void CompositeColumnCondition::reset() {
  mOperator = kAnd;
  mChildren.reset();
}

void CompositeColumnCondition::prettyPrint(string &out) const {
  out.append("{\"Operator\":");
  pp::prettyPrint(out, mOperator);
  out.append(",\"Children\":");
  pp::prettyPrint(out, mChildren);
  out.push_back('}');
}

std::optional<OTSError> CompositeColumnCondition::validate() const {
  for (int64_t i = 0, sz = mChildren.size(); i < sz; ++i) {
    if (mChildren[i].get() == NULL) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() =
          "Children of a composite column condition must be nonnull.";
      return std::optional<OTSError>(std::move(e));
    }
    TRY(mChildren[i]->validate());
  }
  return std::optional<OTSError>();
}

void RowChange::reset() {
  mTable.clear();
  mPrimaryKey.reset();
  mCondition.reset();
  mReturnType = kRT_None;
}

bool RowChange::operator==(const RowChange &b) const {
  if (table() != b.table()) {
    return false;
  }
  if (primaryKey() != b.primaryKey()) {
    return false;
  }
  if (condition() != b.condition()) {
    return false;
  }
  if (returnType() != b.returnType()) {
    return false;
  }
  return true;
}

void RowChange::prettyPrint(string &out) const {
  out.append("\"TableName\":");
  pp::prettyPrint(out, mTable);
  out.append(",\"PrimaryKey\":");
  pp::prettyPrint(out, mPrimaryKey);
  out.append(",\"Condition\":");
  pp::prettyPrint(out, mCondition);
  out.append(",\"ReturnType\":");
  pp::prettyPrint(out, mReturnType);
}

std::optional<OTSError> RowChange::validate() const {
  if (mTable.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name is required.";
    return std::optional<OTSError>(std::move(e));
  }
  TRY(mPrimaryKey.validate());
  for (int64_t i = 0, sz = mPrimaryKey.size(); i < sz; ++i) {
    if (mPrimaryKey[i].value().isInfinity()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Infinity is not allowed in writing a row.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  TRY(mCondition.validate());
  return std::optional<OTSError>();
}

void RowPutChange::reset() {
  RowChange::reset();
  mAttrs.reset();
}

void RowPutChange::prettyPrint(string &out) const {
  out.append("{\"ChangeType\":\"RowPutChange\",");
  RowChange::prettyPrint(out);
  out.append(",\"Columns\":");
  pp::prettyPrint(out, mAttrs);
  out.push_back('}');
}

std::optional<OTSError> RowPutChange::validate() const {
  TRY(RowChange::validate());
  for (int64_t i = 0, sz = mAttrs.size(); i < sz; ++i) {
    TRY(mAttrs[i].validate());
  }
  return std::optional<OTSError>();
}

PutRowRequest &PutRowRequest::operator=(const MoveHolder<PutRowRequest> &a) {
  moveAssign(mRowChange, std::move(a->mRowChange));
  return *this;
}

void PutRowRequest::reset() { mRowChange.reset(); }

void PutRowRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"PutRowRequest\",\"RowChange\":");
  pp::prettyPrint(out, mRowChange);
  out.push_back('}');
}

std::optional<OTSError> PutRowRequest::validate() const {
  return mRowChange.validate();
}

PutRowResponse &PutRowResponse::operator=(const MoveHolder<PutRowResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mRow, std::move(a->mRow));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void PutRowResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mRow.reset();
}

void PutRowResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"PutRowResponse\",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  if (mRow) {
    out.append(",\"Row\":");
    pp::prettyPrint(out, *mRow);
  }
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> PutRowResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  if (mRow) {
    TRY(mRow->validate());
  }
  return std::optional<OTSError>();
}

QueryCriterion &QueryCriterion::operator=(const MoveHolder<QueryCriterion> &a) {
  moveAssign(mTable, std::move(a->mTable));
  moveAssign(mColumnsToGet, std::move(a->mColumnsToGet));
  moveAssign(mMaxVersions, std::move(a->mMaxVersions));
  moveAssign(mTimeRange, std::move(a->mTimeRange));
  moveAssign(mCacheBlocks, std::move(a->mCacheBlocks));
  moveAssign(mFilter, std::move(a->mFilter));
  return *this;
}

void QueryCriterion::reset() {
  mTable.clear();
  mColumnsToGet.reset();
  mMaxVersions.reset();
  mTimeRange.reset();
  mCacheBlocks.reset();
  mFilter.reset();
}

void QueryCriterion::prettyPrint(string &out) const {
  out.append("\"TableName\":");
  pp::prettyPrint(out, mTable);
  out.append(",\"ColumnsToGet\":");
  pp::prettyPrint(out, mColumnsToGet);
  if (mMaxVersions) {
    out.append(",\"MaxVersions\":");
    pp::prettyPrint(out, *mMaxVersions);
  }
  if (mTimeRange) {
    out.append(",\"TimeRange\":");
    pp::prettyPrint(out, *mTimeRange);
  }
  if (mCacheBlocks) {
    out.append(",\"CacheBlocks\":");
    pp::prettyPrint(out, *mCacheBlocks);
  }
  if (mFilter.get() != NULL) {
    out.append(",\"Filter\":");
    pp::prettyPrint(out, *mFilter);
  }
}

std::optional<OTSError> QueryCriterion::validate() const {
  if (mTable.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Table name is required.";
    return std::optional<OTSError>(std::move(e));
  }
  for (int64_t i = 0, sz = mColumnsToGet.size(); i < sz; ++i) {
    if (mColumnsToGet[i].empty()) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Columns in ColumnsToGet must be nonempty.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  if (!mMaxVersions && !mTimeRange) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Either MaxVersions or TimeRange is required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mMaxVersions && *mMaxVersions <= 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "MaxVersions must be positive.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mTimeRange) {
    TRY(mTimeRange->validate());
  }
  if (mFilter.get() != NULL) {
    TRY(mFilter->validate());
  }
  return std::optional<OTSError>();
}

PointQueryCriterion &
PointQueryCriterion::operator=(const MoveHolder<PointQueryCriterion> &a) {
  QueryCriterion::operator=(std::move(static_cast<QueryCriterion &>(*a)));
  moveAssign(mPrimaryKey, std::move(a->mPrimaryKey));
  return *this;
}

void PointQueryCriterion::reset() {
  QueryCriterion::reset();
  mPrimaryKey.reset();
}

void PointQueryCriterion::prettyPrint(string &out) const {
  out.push_back('{');
  QueryCriterion::prettyPrint(out);
  out.append(",\"PrimaryKey\":");
  pp::prettyPrint(out, mPrimaryKey);
  out.push_back('}');
}

std::optional<OTSError> PointQueryCriterion::validate() const {
  TRY(QueryCriterion::validate());
  TRY(mPrimaryKey.validate());
  return std::optional<OTSError>();
}

GetRowRequest &GetRowRequest::operator=(const MoveHolder<GetRowRequest> &a) {
  moveAssign(mQueryCriterion, std::move(a->mQueryCriterion));
  return *this;
}

void GetRowRequest::reset() { mQueryCriterion.reset(); }

void GetRowRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"GetRowRequest\",\"QueryCriterion\":");
  pp::prettyPrint(out, mQueryCriterion);
  out.push_back('}');
}

std::optional<OTSError> GetRowRequest::validate() const {
  return mQueryCriterion.validate();
}

GetRowResponse &GetRowResponse::operator=(const MoveHolder<GetRowResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mRow, std::move(a->mRow));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void GetRowResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mRow.reset();
}

void GetRowResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"GetRowResponse\",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  if (mRow) {
    out.append(",\"Row\":");
    pp::prettyPrint(out, *mRow);
  }
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> GetRowResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  if (mRow) {
    TRY(mRow->validate());
  }
  return std::optional<OTSError>();
}

RangeQueryCriterion &
RangeQueryCriterion::operator=(const MoveHolder<RangeQueryCriterion> &a) {
  moveAssign(mDirection, std::move(a->mDirection));
  moveAssign(mInclusiveStart, std::move(a->mInclusiveStart));
  moveAssign(mExclusiveEnd, std::move(a->mExclusiveEnd));
  moveAssign(mLimit, std::move(a->mLimit));
  QueryCriterion::operator=(std::move(static_cast<QueryCriterion &>(*a)));
  return *this;
}

void RangeQueryCriterion::reset() {
  QueryCriterion::reset();
  mDirection = FORWARD;
  mInclusiveStart.reset();
  mExclusiveEnd.reset();
  mLimit.reset();
}

void RangeQueryCriterion::prettyPrint(string &out) const {
  out.append("{\"Direction\":");
  pp::prettyPrint(out, mDirection);
  out.append(",\"Start\":");
  pp::prettyPrint(out, mInclusiveStart);
  out.append(",\"End\":");
  pp::prettyPrint(out, mExclusiveEnd);
  if (mLimit) {
    out.append(",\"Limit\":");
    pp::prettyPrint(out, *mLimit);
  }
  out.push_back('}');
}

std::optional<OTSError> RangeQueryCriterion::validate() const {
  TRY(QueryCriterion::validate());
  TRY(mInclusiveStart.validate());
  TRY(mExclusiveEnd.validate());
  if (mInclusiveStart.size() == 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Start primary key is required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mExclusiveEnd.size() == 0) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "End primary key is required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mInclusiveStart.size() != mExclusiveEnd.size()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() =
        "Start primary key must be of the same length of that of the end.";
    return std::optional<OTSError>(std::move(e));
  }
  {
    CompareResult r = mInclusiveStart.compare(mExclusiveEnd);
    if (mDirection == FORWARD) {
      if (r == kCR_Uncomparable || r == kCR_Larger) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Start primary key should be less than or equals "
                             "to the end in a forward range.";
        return std::optional<OTSError>(std::move(e));
      }
    } else {
      if (r == kCR_Uncomparable || r == kCR_Smaller) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Start primary key should be greater than or "
                             "equals to the end in a backward range.";
        return std::optional<OTSError>(std::move(e));
      }
    }
  }
  if (mLimit) {
    if (*mLimit <= 0) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Limit of GetRange must be positive.";
      return std::optional<OTSError>(std::move(e));
    }
  }
  return std::optional<OTSError>();
}

GetRangeRequest &
GetRangeRequest::operator=(const MoveHolder<GetRangeRequest> &a) {
  moveAssign(mQueryCriterion, std::move(a->mQueryCriterion));
  return *this;
}

void GetRangeRequest::reset() { mQueryCriterion.reset(); }

void GetRangeRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"GetRangeRequest\",\"QueryCriterion\":");
  pp::prettyPrint(out, mQueryCriterion);
  out.push_back('}');
}

std::optional<OTSError> GetRangeRequest::validate() const {
  TRY(mQueryCriterion.validate());
  return std::optional<OTSError>();
}

GetRangeResponse &
GetRangeResponse::operator=(const MoveHolder<GetRangeResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mRows, std::move(a->mRows));
  moveAssign(mNextStart, std::move(a->mNextStart));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void GetRangeResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mRows.reset();
  mNextStart.reset();
}

void GetRangeResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"GetRangeResponse\",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  out.append(",\"Rows\":");
  pp::prettyPrint(out, mRows);
  if (mNextStart) {
    out.append(",\"NextStart\":");
    pp::prettyPrint(out, *mNextStart);
  }
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> GetRangeResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  for (int64_t i = 0, sz = mRows.size(); i < sz; ++i) {
    TRY(mRows[i].validate());
  }
  if (mNextStart) {
    TRY(mNextStart->validate());
  }
  return std::optional<OTSError>();
}

RowUpdateChange::Update &RowUpdateChange::Update::operator=(
    const MoveHolder<RowUpdateChange::Update> &a) {
  moveAssign(mType, std::move(a->mType));
  moveAssign(mAttrName, std::move(a->mAttrName));
  moveAssign(mAttrValue, std::move(a->mAttrValue));
  moveAssign(mTimestamp, std::move(a->mTimestamp));
  return *this;
}

bool RowUpdateChange::Update::operator==(
    const RowUpdateChange::Update &b) const {
  if (type() != b.type()) {
    return false;
  }
  if (attrName() != b.attrName()) {
    return false;
  }
  if (attrValue() != b.attrValue()) {
    return false;
  }
  if (timestamp() != b.timestamp()) {
    return false;
  }
  return true;
}

void RowUpdateChange::reset() {
  RowChange::reset();
  mUpdates.reset();
}

bool RowUpdateChange::operator==(const RowUpdateChange &b) const {
  if (!RowChange::operator==(b)) {
    return false;
  }
  if (updates() != b.updates()) {
    return false;
  }
  return true;
}

void RowUpdateChange::Update::prettyPrint(string &out) const {
  out.append("{\"UpdateType\":");
  pp::prettyPrint(out, mType);
  out.append(",\"AttrName\":");
  pp::prettyPrint(out, mAttrName);
  if (mAttrValue) {
    out.append(",\"AttrValue\":");
    pp::prettyPrint(out, *mAttrValue);
  }
  if (mTimestamp) {
    out.append(",\"Timestamp\":");
    pp::prettyPrint(out, *mTimestamp);
  }
  out.push_back('}');
}

std::optional<OTSError> RowUpdateChange::Update::validate() const {
  if (mAttrName.empty()) {
    OTSError e(OTSError::kPredefined_OTSParameterInvalid);
    e.mutableMessage() = "Attribute name is required.";
    return std::optional<OTSError>(std::move(e));
  }
  if (mAttrValue) {
    TRY(mAttrValue->validate());
  }
  switch (mType) {
  case kPut:
    if (!mAttrValue) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() = "Attribute value is required for Put update.";
      return std::optional<OTSError>(std::move(e));
    }
    break;
  case kDelete: {
    if (mAttrValue) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() =
          "Attribute value should not be specified for Delete update.";
      return std::optional<OTSError>(std::move(e));
    }
    break;
  }
  case kDeleteAll: {
    if (mAttrValue) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() =
          "Attribute value should not be specified for Delete-All update.";
      return std::optional<OTSError>(std::move(e));
    }
    if (mTimestamp) {
      OTSError e(OTSError::kPredefined_OTSParameterInvalid);
      e.mutableMessage() =
          "Timestamp should not be specified for Delete-All update.";

      return std::optional<OTSError>(std::move(e));
    }
    break;
  }
  }
  return std::optional<OTSError>();
}

RowUpdateChange &
RowUpdateChange::operator=(const MoveHolder<RowUpdateChange> &a) {
  moveAssign(mUpdates, std::move(a->mUpdates));
  RowChange::move(*a);
  return *this;
}

void RowUpdateChange::prettyPrint(string &out) const {
  out.append("{\"ChangeType\":\"RowUpdateChange\",");
  RowChange::prettyPrint(out);
  out.append(",\"Update\":");
  pp::prettyPrint(out, mUpdates);
  out.push_back('}');
}

std::optional<OTSError> RowUpdateChange::validate() const {
  TRY(RowChange::validate());
  for (int64_t i = 0, sz = mUpdates.size(); i < sz; ++i) {
    TRY(mUpdates[i].validate());
  }
  return std::optional<OTSError>();
}

UpdateRowRequest &
UpdateRowRequest::operator=(const MoveHolder<UpdateRowRequest> &a) {
  moveAssign(mRowChange, std::move(a->mRowChange));
  return *this;
}

void UpdateRowRequest::reset() { mRowChange.reset(); }

void UpdateRowRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"UpdateRowRequest\",\"RowChange\":");
  pp::prettyPrint(out, mRowChange);
  out.push_back('}');
}

std::optional<OTSError> UpdateRowRequest::validate() const {
  TRY(mRowChange.validate());
  return std::optional<OTSError>();
}

UpdateRowResponse &
UpdateRowResponse::operator=(const MoveHolder<UpdateRowResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mRow, std::move(a->mRow));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void UpdateRowResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mRow.reset();
}

void UpdateRowResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"UpdateRowResponse\",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  if (mRow) {
    out.append(",\"Row\":");
    pp::prettyPrint(out, *mRow);
  }
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> UpdateRowResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  if (mRow) {
    TRY(mRow->validate());
  }
  return std::optional<OTSError>();
}

void RowDeleteChange::reset() { RowChange::reset(); }

void RowDeleteChange::prettyPrint(string &out) const {
  out.append("{\"ChangeType\":\"RowPutChange\",");
  RowChange::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> RowDeleteChange::validate() const {
  TRY(RowChange::validate());
  return std::optional<OTSError>();
}

DeleteRowRequest &
DeleteRowRequest::operator=(const MoveHolder<DeleteRowRequest> &a) {
  moveAssign(mRowChange, std::move(a->mRowChange));
  return *this;
}

void DeleteRowRequest::reset() { mRowChange.reset(); }

void DeleteRowRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"DeleteRowRequest\",\"RowChange\":");
  pp::prettyPrint(out, mRowChange);
  out.push_back('}');
}

std::optional<OTSError> DeleteRowRequest::validate() const {
  TRY(mRowChange.validate());
  return std::optional<OTSError>();
}

DeleteRowResponse &
DeleteRowResponse::operator=(const MoveHolder<DeleteRowResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mRow, std::move(a->mRow));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void DeleteRowResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mRow.reset();
}

void DeleteRowResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"DeleteRowResponse\"");
  Response::prettyPrint(out);
  out.append(",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  if (mRow) {
    out.append(",\"Row\":");
    pp::prettyPrint(out, *mRow);
  }
  out.push_back('}');
}

std::optional<OTSError> DeleteRowResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  if (mRow) {
    TRY(mRow->validate());
  }
  return std::optional<OTSError>();
}

MultiPointQueryCriterion::MultiPointQueryCriterion(
    const MoveHolder<MultiPointQueryCriterion> &a) {
  *this = a;
}

MultiPointQueryCriterion &MultiPointQueryCriterion::operator=(
    const MoveHolder<MultiPointQueryCriterion> &a) {
  QueryCriterion::operator=(std::move(static_cast<QueryCriterion &>(*a)));
  moveAssign(mRowKeys, std::move(a->mRowKeys));
  return *this;
}

void MultiPointQueryCriterion::reset() {
  QueryCriterion::reset();
  mRowKeys.reset();
}

void MultiPointQueryCriterion::prettyPrint(string &out) const {
  out.append("{");
  QueryCriterion::prettyPrint(out);
  out.append(",\"RowKeys\":");
  pp::prettyPrint(out, mRowKeys);
  out.push_back('}');
}

std::optional<OTSError> MultiPointQueryCriterion::validate() const {
  TRY(QueryCriterion::validate());
  for (int64_t i = 0, sz = mRowKeys.size(); i < sz; ++i) {
    TRY(mRowKeys[i].get().validate());
  }
  return std::optional<OTSError>();
}

BatchGetRowRequest::BatchGetRowRequest(
    const MoveHolder<BatchGetRowRequest> &a) {
  *this = a;
}

BatchGetRowRequest &
BatchGetRowRequest::operator=(const MoveHolder<BatchGetRowRequest> &a) {
  moveAssign(mCriteria, std::move(a->mCriteria));
  return *this;
}

void BatchGetRowRequest::reset() { mCriteria.reset(); }

void BatchGetRowRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"BatchGetRowRequest\",\"Criteria\":");
  pp::prettyPrint(out, mCriteria);
  out.push_back('}');
}

std::optional<OTSError> BatchGetRowRequest::validate() const {
  for (int64_t i = 0, sz = criteria().size(); i < sz; ++i) {
    TRY(criteria()[i].validate());
  }
  return std::optional<OTSError>();
}

BatchGetRowResponse &
BatchGetRowResponse::operator=(const MoveHolder<BatchGetRowResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mResults, std::move(a->mResults));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void BatchGetRowResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mResults.reset();
}

void BatchGetRowResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"BatchGetRowResponse\",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  out.append(",\"Results\":");
  pp::prettyPrint(out, mResults);
  Response::prettyPrint(out);
  out.append("}");
}

std::optional<OTSError> BatchGetRowResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  for (int64_t i = 0, sz = mResults.size(); i < sz; ++i) {
    const util::Result<std::optional<Row>, OTSError> &result =
        mResults[i].get();
    if (result.ok()) {
      if (result.okValue()) {
        TRY(result.okValue()->validate());
      }
    }
  }
  return std::optional<OTSError>();
}

BatchWriteRowRequest::BatchWriteRowRequest(
    const MoveHolder<BatchWriteRowRequest> &a) {
  *this = a;
}

void BatchWriteRowRequest::reset() {
  mPuts.reset();
  mUpdates.reset();
  mDeletes.reset();
}

BatchWriteRowRequest &
BatchWriteRowRequest::operator=(const MoveHolder<BatchWriteRowRequest> &a) {
  moveAssign(mPuts, std::move(a->mPuts));
  moveAssign(mUpdates, std::move(a->mUpdates));
  moveAssign(mDeletes, std::move(a->mDeletes));
  return *this;
}

void BatchWriteRowRequest::prettyPrint(string &out) const {
  out.append("{\"API\":\"BatchWriteRequest\",\"Puts\":");
  pp::prettyPrint(out, mPuts);
  out.append(",\"Updates\":");
  pp::prettyPrint(out, mUpdates);
  out.append(",\"Deletes\":");
  pp::prettyPrint(out, mDeletes);
  out.push_back('}');
}

std::optional<OTSError> BatchWriteRowRequest::validate() const {
  for (int64_t i = 0, sz = mPuts.size(); i < sz; ++i) {
    TRY(mPuts[i].get().validate());
  }
  for (int64_t i = 0, sz = mUpdates.size(); i < sz; ++i) {
    TRY(mUpdates[i].get().validate());
  }
  for (int64_t i = 0, sz = mDeletes.size(); i < sz; ++i) {
    TRY(mDeletes[i].get().validate());
  }
  return std::optional<OTSError>();
}

BatchWriteRowResponse::BatchWriteRowResponse(
    const MoveHolder<BatchWriteRowResponse> &a) {
  *this = a;
}

void BatchWriteRowResponse::reset() {
  Response::reset();
  mConsumedCapacity.reset();
  mPutResults.reset();
  mUpdateResults.reset();
  mDeleteResults.reset();
}

BatchWriteRowResponse &
BatchWriteRowResponse::operator=(const MoveHolder<BatchWriteRowResponse> &a) {
  moveAssign(mConsumedCapacity, std::move(a->mConsumedCapacity));
  moveAssign(mPutResults, std::move(a->mPutResults));
  moveAssign(mUpdateResults, std::move(a->mUpdateResults));
  moveAssign(mDeleteResults, std::move(a->mDeleteResults));
  Response::operator=(std::move(static_cast<Response &>(*a)));
  return *this;
}

void BatchWriteRowResponse::prettyPrint(string &out) const {
  out.append("{\"API\":\"BatchWriteResponse\",\"ConsumedCapacity\":");
  pp::prettyPrint(out, mConsumedCapacity);
  out.append(",\"PutResults\":");
  pp::prettyPrint(out, mPutResults);
  out.append(",\"UpdateResults\":");
  pp::prettyPrint(out, mUpdateResults);
  out.append(",\"DeleteResults\":");
  pp::prettyPrint(out, mDeleteResults);
  Response::prettyPrint(out);
  out.push_back('}');
}

std::optional<OTSError> BatchWriteRowResponse::validate() const {
  TRY(mConsumedCapacity.validate());
  for (int64_t i = 0, sz = putResults().size(); i < sz; ++i) {
    const util::Result<std::optional<Row>, OTSError> &res =
        putResults()[i].get();
    if (res.ok() && res.okValue()) {
      TRY(res.okValue()->validate());
    }
  }
  for (int64_t i = 0, sz = updateResults().size(); i < sz; ++i) {
    const util::Result<std::optional<Row>, OTSError> &res =
        updateResults()[i].get();
    if (res.ok() && res.okValue()) {
      TRY(res.okValue()->validate());
    }
  }
  for (int64_t i = 0, sz = deleteResults().size(); i < sz; ++i) {
    const util::Result<std::optional<Row>, OTSError> &res =
        deleteResults()[i].get();
    if (res.ok() && res.okValue()) {
      TRY(res.okValue()->validate());
    }
  }
  return std::optional<OTSError>();
}

} // namespace core
} // namespace tablestore
} // namespace aliyun

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
#include "common.hpp"
#include "tablestore/core/range_iterator.hpp"

#include "tablestore/util/assert.hpp"
#include "tablestore/util/logging.hpp"
#include "tablestore/util/move.hpp"
#include "testa/testa.hpp"
#include <boost/ref.hpp>
#include <functional>
#include <memory>
#include <string>
#include <deque>

using namespace std;

using namespace std::placeholders;
using namespace aliyun::tablestore::util;

namespace aliyun {
namespace tablestore {
namespace core {

typedef function<std::optional<OTSError>(GetRangeResponse&, const GetRangeRequest&)> Api;

namespace {

std::optional<OTSError> collect(deque<Row>& rows, RangeIterator& iter)
{
    for(;;) {
        std::optional<OTSError> err = iter.moveNext();
        if (err) {
            return err;
        }
        if (!iter.valid()) {
            break;
        }
        Row& r = iter.get();
        rows.push_back(Row());
        moveAssign(rows.back(), std::move(r));
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> getRange_empty(
    Logger& logger,
    GetRangeResponse& resp,
    const GetRangeRequest& req)
{
    GetRangeResponse emptyResp;
    moveAssign(resp, std::move(emptyResp));
    return std::optional<OTSError>();
}

} // namespace

void RangeIterator_empty(const string&)
{
    unique_ptr<Logger> logger(createLogger("/", Logger::kDebug));
    MockSyncClient client(*logger);
    client.mutableGetRange() =
        bind(getRange_empty,
            boost::ref(*logger),
            _1, _2);
    RangeQueryCriterion criterion;
    criterion.mutableInclusiveStart().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMin());
    criterion.mutableExclusiveEnd().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMax());

    RangeIterator rit(client, criterion);
    deque<Row> rows;
    std::optional<OTSError> err = collect(rows, rit);

    TESTA_ASSERT(!err)
        (*err)
        .issue();
    TESTA_ASSERT(pp::prettyPrint(rows) == "[]")
        (rows)
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(RangeIterator_empty);

namespace {

std::optional<OTSError> getRange_one(
    Logger& logger,
    GetRangeResponse& resp,
    const GetRangeRequest& req)
{
    {
        Row& r = resp.mutableRows().append();
        r.mutablePrimaryKey().append() =
            PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(0));
    }
    resp.mutableConsumedCapacity().mutableRead().reset(12);
    return std::optional<OTSError>();
}

} // namespace

void RangeIterator_one(const string&)
{
    unique_ptr<Logger> logger(createLogger("/", Logger::kDebug));
    MockSyncClient client(*logger);
    client.mutableGetRange() =
        bind(getRange_one,
            boost::ref(*logger),
            _1, _2);
    RangeQueryCriterion criterion;
    criterion.mutableInclusiveStart().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMin());
    criterion.mutableExclusiveEnd().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMax());

    RangeIterator rit(client, criterion);
    deque<Row> rows;
    std::optional<OTSError> err = collect(rows, rit);

    TESTA_ASSERT(!err)
        (*err)
        .issue();
    TESTA_ASSERT(pp::prettyPrint(rows) == "[{\"PrimaryKey\":{\"pk\":0},\"Attributes\":[]}]")
        (rows)
        .issue();
    CapacityUnit cu = rit.consumedCapacity();
    TESTA_ASSERT(cu.read())
        .issue();
    TESTA_ASSERT(*cu.read() == 12)
        .issue();
    TESTA_ASSERT(!cu.write())
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(RangeIterator_one);

namespace {

std::optional<OTSError> getRange_continuation_1(
    Logger& logger,
    GetRangeResponse& resp,
    const GetRangeRequest& req)
{
    OTS_LOG_DEBUG(logger);
    {
        Row& r = resp.mutableRows().append();
        r.mutablePrimaryKey().append() =
            PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(1));
    }
    resp.mutableConsumedCapacity().mutableRead().reset(2);
    return std::optional<OTSError>();
}

std::optional<OTSError> getRange_continuation_0(
    Logger& logger,
    Api& api,
    GetRangeResponse& resp,
    const GetRangeRequest& req)
{
    OTS_LOG_DEBUG(logger);
    std::optional<OTSError> err;
    {
        Row& r = resp.mutableRows().append();
        r.mutablePrimaryKey().append() =
            PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(0));
    }
    {
        PrimaryKey nextPk;
        nextPk.append() = PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(1));
        resp.mutableNextStart().emplace(std::move(nextPk));
    }
    resp.mutableConsumedCapacity().mutableRead().reset(1);
    api = bind(getRange_continuation_1,
        boost::ref(logger),
        _1, _2);
    return std::optional<OTSError>();
}

} // namespace

void RangeIterator_continuation(const string&)
{
    unique_ptr<Logger> logger(createLogger("/", Logger::kDebug));
    MockSyncClient client(*logger);
    Api& api = client.mutableGetRange();
    api = bind(getRange_continuation_0,
        boost::ref(*logger),
        boost::ref(api),
        _1, _2);
    RangeQueryCriterion criterion;
    criterion.mutableInclusiveStart().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMin());
    criterion.mutableExclusiveEnd().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMax());

    RangeIterator rit(client, criterion);
    deque<Row> rows;
    std::optional<OTSError> err = collect(rows, rit);

    TESTA_ASSERT(!err)
        (*err)
        .issue();
    TESTA_ASSERT(pp::prettyPrint(rows) == "["
        "{\"PrimaryKey\":{\"pk\":0},\"Attributes\":[]},"
        "{\"PrimaryKey\":{\"pk\":1},\"Attributes\":[]}"
        "]")
        (rows)
        .issue();
    CapacityUnit cu = rit.consumedCapacity();
    TESTA_ASSERT(cu.read())
        .issue();
    TESTA_ASSERT(*cu.read() == 3)
        .issue();
    TESTA_ASSERT(!cu.write())
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(RangeIterator_continuation);

namespace {

std::optional<OTSError> getRange_limit_1(
    Logger& logger,
    GetRangeResponse& resp,
    const GetRangeRequest& req)
{
    OTS_LOG_DEBUG(logger);
    std::optional<OTSError> err;
    OTS_ASSERT(req.queryCriterion().limit());
    OTS_LOG_DEBUG(logger)
        ("Limit", *(req.queryCriterion().limit()));
    for(int64_t i = 0, sz = *(req.queryCriterion().limit()); i < sz; ++i) {
        Row& r = resp.mutableRows().append();
        r.mutablePrimaryKey().append() =
            PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(i + 1));
    }
    {
        PrimaryKey nextPk;
        nextPk.append() =
            PrimaryKeyColumn(
                "pk",
                PrimaryKeyValue::toInteger(*(req.queryCriterion().limit()) + 1));
        resp.mutableNextStart().emplace(std::move(nextPk));
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> getRange_limit_0(
    Logger& logger,
    Api& api,
    GetRangeResponse& resp,
    const GetRangeRequest& req)
{
    OTS_LOG_DEBUG(logger);
    std::optional<OTSError> err;
    {
        Row& r = resp.mutableRows().append();
        r.mutablePrimaryKey().append() =
            PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(0));
    }
    {
        PrimaryKey nextPk;
        nextPk.append() = PrimaryKeyColumn("pk", PrimaryKeyValue::toInteger(1));
        resp.mutableNextStart().emplace(std::move(nextPk));
    }
    api = bind(getRange_limit_1,
        boost::ref(logger),
        _1, _2);
    return std::optional<OTSError>();
}

} // namespace

void RangeIterator_limit(const string&)
{
    unique_ptr<Logger> logger(createLogger("/", Logger::kDebug));
    MockSyncClient client(*logger);
    Api& api = client.mutableGetRange();
    api = bind(getRange_limit_0,
        boost::ref(*logger),
        boost::ref(api),
        _1, _2);
    RangeQueryCriterion criterion;
    criterion.mutableInclusiveStart().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMin());
    criterion.mutableExclusiveEnd().append() =
        PrimaryKeyColumn("pk", PrimaryKeyValue::toInfMax());
    criterion.mutableLimit().reset(2);

    RangeIterator rit(client, criterion);
    deque<Row> rows;
    std::optional<OTSError> err = collect(rows, rit);

    TESTA_ASSERT(!err)
        (*err)
        .issue();
    TESTA_ASSERT(pp::prettyPrint(rows) == "["
        "{\"PrimaryKey\":{\"pk\":0},\"Attributes\":[]},"
        "{\"PrimaryKey\":{\"pk\":1},\"Attributes\":[]}"
        "]")
        (rows)
        .issue();
    std::optional<PrimaryKey> nextPk = rit.nextStart();
    TESTA_ASSERT(nextPk)
        .issue();
    TESTA_ASSERT(pp::prettyPrint(*nextPk) == "{\"pk\":2}")
        (*nextPk)
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(RangeIterator_limit);

} // namespace core
} // namespace tablestore
} // namespace aliyun

#pragma once
#ifndef TABLESTORE_CORE_CLIENT_HPP
#define TABLESTORE_CORE_CLIENT_HPP
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
#include "tablestore/core/error.hpp"

#include "tablestore/util/logger.hpp"
#include <string>

namespace aliyun {
namespace tablestore {
namespace core {
class AsyncClient;

/**
 * An interface to synchronized client to aliyun TableStore.
 */
class SyncClient
{
public:
    virtual ~SyncClient() {}

    /**
     * Creates a synchronous client for production use.
     *
     * If no error is detected during creation, @p result will be set to
     * the client; otherwise, the error will be returned and
     * @p result keep untouched.
     */
    static std::optional<OTSError> create(
        SyncClient*& result,
        Endpoint&, Credential&, ClientOptions&);

    /**
     * Creates a synchronous client for production use from an asynchronous one.
     * Both share the same backbone implementation.
     */
    static SyncClient* create(AsyncClient&);

    // configurations

    virtual util::Logger& mutableLogger() =0;
    virtual const std::deque<std::shared_ptr<util::Actor> >& actors() const =0;
    virtual const RetryStrategy& retryStrategy() const =0;

    // table operations

    /**
     * Creates a table.
     */
    virtual std::optional<OTSError> createTable(
        CreateTableResponse&, const CreateTableRequest&) =0;

    /**
     * Deletes a table.
     */
    virtual std::optional<OTSError> deleteTable(
        DeleteTableResponse&, const DeleteTableRequest&) =0;

    /**
     * Lists all tables under this instance.
     */
    virtual std::optional<OTSError> listTable(
        ListTableResponse&, const ListTableRequest&) =0;

    /**
     * Fetches meta of a table.
     */
    virtual std::optional<OTSError> describeTable(
        DescribeTableResponse&, const DescribeTableRequest&) =0;

    /**
     * Updates mutable fields of meta of a table.
     */
    virtual std::optional<OTSError> updateTable(
        UpdateTableResponse&, const UpdateTableRequest&) =0;

    // point write

    /**
     * Puts a row.
     * When the row already exists, it will be overwritten if the row condition
     * in the request is ignore or expect-exist.
     */
    virtual std::optional<OTSError> putRow(PutRowResponse&, const PutRowRequest&) =0;

    /**
     * Updates a row.
     * It can be used either to modify an existent row or to insert a new row.
     */
    virtual std::optional<OTSError> updateRow(
        UpdateRowResponse&, const UpdateRowRequest&) =0;

    /**
     * Deletes a row.
     */
    virtual std::optional<OTSError> deleteRow(
        DeleteRowResponse&, const DeleteRowRequest&) =0;

    /**
     * Write a batch of rows.
     *
     * If there occurs a request-level error, returns the error;
     * If there are row-level errors, put them into their respective
     * results of rows.
     */
    virtual std::optional<OTSError> batchWriteRow(
        BatchWriteRowResponse&, const BatchWriteRowRequest&) =0;

    // // point query

    /**
     * Gets a row.
     * When the row inexists, it will respond a response with absent row field,
     * rather than an error.
     */
    virtual std::optional<OTSError> getRow(GetRowResponse&, const GetRowRequest&) =0;

    /**
     * Gets a batch of rows.
     *
     * If there occurs a request-level error, returns the error;
     * If there are row-level errors, put them into their respective
     * results of rows.
     */
    virtual std::optional<OTSError> batchGetRow(
        BatchGetRowResponse&, const BatchGetRowRequest&) =0;

    // range query

    /**
     * Fetches a range of rows in a single turnover.
     * Backend of TableStore will possibly respond prematurely.
     * When this happens, GetRangeResponse::nextStart will be set.
     *
     * Strongly recommend use RangeIterator instead to correctly
     * handle this premature nature.
     */
    virtual std::optional<OTSError> getRange(
        GetRangeResponse&, const GetRangeRequest&) =0;

    // Miscellaneous operations

    /**
     * Computes horizontal splits by a user-specific size of each split.
     * Ranges of these splits can be directly fed to getRange().
     *
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual std::optional<OTSError> computeSplitsBySize(
        ComputeSplitsBySizeResponse&, const ComputeSplitsBySizeRequest&) =0;
};

/**
 * An interface to asynchronized client to aliyun TableStore.
 */
class AsyncClient
{
public:
    virtual ~AsyncClient() {}

    /**
     * Creates an asynchronized client for production use.
     *
     * If no error is detected during creation, @p result will be set to
     * the client, which must be owned by the caller;
     * otherwise, the error will be returned and @p result keep untouched.
     */
    static std::optional<OTSError> create(
        AsyncClient*& result,
        Endpoint&, Credential&, ClientOptions&);

    /**
     * Creates an asynchronous client for production use from a synchronous one.
     * Both share the same backbone implementation.
     */
    static AsyncClient* create(SyncClient&);

    // configurations

    virtual util::Logger& mutableLogger() =0;
    virtual const std::deque<std::shared_ptr<util::Actor> >& actors() const =0;
    virtual const RetryStrategy& retryStrategy() const =0;

    // table operations

    /**
     * Creates a table.
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void createTable(
        CreateTableRequest&,
        const std::function<void(
            CreateTableRequest&, std::optional<OTSError>&, CreateTableResponse&)>&) =0;

    /**
     * Deletes a table.
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void deleteTable(
        DeleteTableRequest&,
        const std::function<void(
            DeleteTableRequest&, std::optional<OTSError>&, DeleteTableResponse&)>&) =0;

    /**
     * Lists all tables under this instance.
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void listTable(
        ListTableRequest&,
        const std::function<void(
            ListTableRequest&, std::optional<OTSError>&, ListTableResponse&)>&) =0;

    /**
     * Fetches meta of a table.
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void describeTable(
        DescribeTableRequest&,
        const std::function<void(
            DescribeTableRequest&, std::optional<OTSError>&, DescribeTableResponse&)>&) =0;

    /**
     * Updates mutable fields of meta of a table.
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void updateTable(
        UpdateTableRequest&,
        const std::function<void(
            UpdateTableRequest&, std::optional<OTSError>&, UpdateTableResponse&)>&) =0;

    // point write

    /**
     * Puts a row.
     * When the row already exists, it will be overwritten if the row condition
     * in the request is ignore or expect-exist.
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void putRow(
        PutRowRequest&,
        const std::function<void(
            PutRowRequest&, std::optional<OTSError>&, PutRowResponse&)>&) =0;

    /**
     * Updates a row.
     * It can be used either to modify an existent row or to insert a new row.
     *
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void updateRow(
        UpdateRowRequest&,
        const std::function<void(
            UpdateRowRequest&, std::optional<OTSError>&, UpdateRowResponse&)>&) =0;

    /**
     * Deletes a row.
     *
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void deleteRow(
        DeleteRowRequest&,
        const std::function<void(
            DeleteRowRequest&, std::optional<OTSError>&, DeleteRowResponse&)>&) =0;

    /**
     * Writes a batch of rows.
     *
     * Caveats:
     * - If there occurs a request-level error, returns the error;
     * - If there are row-level errors, put them into their respective
     *   results of rows.
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void batchWriteRow(
        BatchWriteRowRequest&,
        const std::function<void(
            BatchWriteRowRequest&, std::optional<OTSError>&, BatchWriteRowResponse&)>&) =0;

    // point query

    /**
     * Gets a row.
     * When the row inexists, it will respond a response with absent row field,
     * rather than an error.
     *
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void getRow(
        GetRowRequest&,
        const std::function<void(
            GetRowRequest&, std::optional<OTSError>&, GetRowResponse&)>&) =0;

    /**
     * Gets a batch of rows.
     * Inexistent rows will respond a response with absent row field,
     * rather than an error.
     *
     * Caveats:
     * - If there occurs a request-level error, returns the error;
     * - If there are row-level errors, put them into their respective
     *   results of rows.
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void batchGetRow(
        BatchGetRowRequest&,
        const std::function<void(
            BatchGetRowRequest&, std::optional<OTSError>&, BatchGetRowResponse&)>&) =0;

    // range query

    /**
     * Fetches a range of rows in a single turnover.
     * Backend of TableStore will possibly respond prematurely.
     * When this happens, GetRangeResponse::nextStart will be set.
     *
     * Strongly recommend use RangeIterator instead to correctly
     * handle this premature nature.
     *
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void getRange(
        GetRangeRequest&,
        const std::function<void(
            GetRangeRequest&, std::optional<OTSError>&, GetRangeResponse&)>&) =0;

    // Miscellaneous operations

    /**
     * Computes horizontal splits by a user-specific size of each split.
     * Ranges of these splits can be directly fed to getRange().
     *
     * Caveats:
     * - Content of the request will probably be changed.
     * - It is generally unwise to do blocking things in callback.
     */
    virtual void computeSplitsBySize(
        ComputeSplitsBySizeRequest&,
        const std::function<void(
            ComputeSplitsBySizeRequest&, std::optional<OTSError>&, ComputeSplitsBySizeResponse&)>&) =0;
};

} // namespace core
} // namespace tablestore
} // namespace aliyun
#endif

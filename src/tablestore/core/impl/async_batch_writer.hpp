#pragma once
#ifndef TABLESTORE_CORE_IMPL_BATCH_WRITER_HPP
#define TABLESTORE_CORE_IMPL_BATCH_WRITER_HPP
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
#include "tablestore/core/batch_writer.hpp"
#include "tablestore/util/threading.hpp"
#include "tablestore/util/random.hpp"

#include "tablestore/util/logger.hpp"
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <functional>
#include <deque>

namespace aliyun {
namespace tablestore {
namespace core {
namespace impl {

enum SingleRowType
{
    kPutRow,
    kUpdateRow,
    kDeleteRow,
};

template<typename T>
struct WriteTraits
{};

template<>
struct WriteTraits<BatchWriteRowRequest::Put>
{
    typedef BatchWriteRowRequest::Put TypeInBatchWriteRequest;
    typedef PutRowRequest SingleRowRequest;
    typedef PutRowResponse SingleRowResponse;
    typedef RowPutChange SingleRowChange;
    typedef std::function<
        void(SingleRowRequest&, std::optional<OTSError>&, SingleRowResponse&)> Callback;
    static const SingleRowType kType = kPutRow;
};
template<>
struct WriteTraits<PutRowRequest> : public WriteTraits<BatchWriteRowRequest::Put>
{};
template<>
struct WriteTraits<PutRowResponse> : public WriteTraits<BatchWriteRowRequest::Put>
{};
template<>
struct WriteTraits<BatchWriteRowRequest::Update>
{
    typedef BatchWriteRowRequest::Update TypeInBatchWriteRequest;
    typedef UpdateRowRequest SingleRowRequest;
    typedef UpdateRowResponse SingleRowResponse;
    typedef RowUpdateChange SingleRowChange;
    typedef std::function<
        void(SingleRowRequest&, std::optional<OTSError>&, SingleRowResponse&)> Callback;
    static const SingleRowType kType = kUpdateRow;
};
template<>
struct WriteTraits<UpdateRowRequest> : public WriteTraits<BatchWriteRowRequest::Update>
{};
template<>
struct WriteTraits<UpdateRowResponse> : public WriteTraits<BatchWriteRowRequest::Update>
{};
template<>
struct WriteTraits<BatchWriteRowRequest::Delete>
{
    typedef BatchWriteRowRequest::Delete TypeInBatchWriteRequest;
    typedef DeleteRowRequest SingleRowRequest;
    typedef DeleteRowResponse SingleRowResponse;
    typedef RowDeleteChange SingleRowChange;
    typedef std::function<
        void(SingleRowRequest&, std::optional<OTSError>&, SingleRowResponse&)> Callback;
    static const SingleRowType kType = kDeleteRow;
};
template<>
struct WriteTraits<DeleteRowRequest> : public WriteTraits<BatchWriteRowRequest::Delete>
{};
template<>
struct WriteTraits<DeleteRowResponse> : public WriteTraits<BatchWriteRowRequest::Delete>
{};

class AsyncBatchWriter : public core::AsyncBatchWriter, private boost::noncopyable
{
public:
    typedef std::function<void(
        PutRowRequest&, std::optional<OTSError>&, PutRowResponse&)> PutRowCallback;
    typedef std::function<void(
        UpdateRowRequest&, std::optional<OTSError>&, UpdateRowResponse&)> UpdateRowCallback;
    typedef std::function<void(
        DeleteRowRequest&, std::optional<OTSError>&, DeleteRowResponse&)> DeleteRowCallback;

    struct CallbackCarrier
    {
        std::deque<PutRowCallback> mPutCallbacks;
        std::deque<UpdateRowCallback> mUpdateCallbacks;
        std::deque<DeleteRowCallback> mDeleteCallbacks;
    };

    struct Item
    {
        enum ItemType
        {
            kInvalid,
            kPutRow,
            kUpdateRow,
            kDeleteRow,
        };

        ItemType mType;
        RowPutChange mRowPutChange;
        PutRowCallback mPutRowCallback;
        RowUpdateChange mRowUpdateChange;
        UpdateRowCallback mUpdateRowCallback;
        RowDeleteChange mRowDeleteChange;
        DeleteRowCallback mDeleteRowCallback;

        explicit Item()
          : mType(kInvalid)
        {}

        explicit Item(RowPutChange&, PutRowCallback&);
        explicit Item(RowUpdateChange&, UpdateRowCallback&);
        explicit Item(RowDeleteChange&, DeleteRowCallback&);
        
        explicit Item(const util::MoveHolder<Item>& a)
        {
            *this = a;
        }

        Item& operator=(const util::MoveHolder<Item>& a)
        {
            if (this == &*a) {
                return *this;
            }
            mType = a->mType;
            a->mType = kInvalid;
            util::moveAssign(mRowPutChange, std::move(a->mRowPutChange));
            util::moveAssign(mPutRowCallback, std::move(a->mPutRowCallback));
            util::moveAssign(mRowUpdateChange, std::move(a->mRowUpdateChange));
            util::moveAssign(mUpdateRowCallback, std::move(a->mUpdateRowCallback));
            util::moveAssign(mRowDeleteChange, std::move(a->mRowDeleteChange));
            util::moveAssign(mDeleteRowCallback, std::move(a->mDeleteRowCallback));
            return *this;
        }
    };

    class IContext
    {
    public:
        explicit IContext(SingleRowType tp)
          : mType(tp)
        {}

        virtual ~IContext()
        {}

        SingleRowType type() const;
        SingleRowType& mutableType();

    private:
        SingleRowType mType;
    };
    
    template<class Request>
    struct Context : public IContext
    {
        typedef typename WriteTraits<Request>::SingleRowResponse Response;
        typedef typename WriteTraits<Request>::Callback Callback;

        explicit Context();

        const Callback& callback() const;
        Callback& mutableCallback();
        const Request& request() const;
        Request& mutableRequest();
        const std::optional<OTSError>& error() const;
        std::optional<OTSError>& mutableError();
        const Response& response() const;
        Response& mutableResponse();
        
    private:
        Callback mCallback;
        Request mRequest;
        std::optional<OTSError> mError;
        Response mResponse;
    };
    
public:
    explicit AsyncBatchWriter(
        AsyncClient& client,
        const BatchWriterConfig& cfg);
    virtual ~AsyncBatchWriter();

    void putRow(
        PutRowRequest&,
        const std::function<void(
            PutRowRequest&, std::optional<OTSError>&, PutRowResponse&)>&);
    void updateRow(
        UpdateRowRequest&,
        const std::function<void(
            UpdateRowRequest&, std::optional<OTSError>&, UpdateRowResponse&)>&);
    void deleteRow(
        DeleteRowRequest&,
        const std::function<void(
            DeleteRowRequest&, std::optional<OTSError>&, DeleteRowResponse&)>&);

    // internal use only. tests require them to be exposed to public.
    void flush();
    std::tuple<util::Duration, int64_t> nextNapAndConcurrency(
        boost::atomic<bool>& backoff,
        int64_t maxConcurrency,
        util::Duration);

private:
    void callbackOnBatch(
        CallbackCarrier*, 
        BatchWriteRowRequest&,
        std::optional<OTSError>&,
        BatchWriteRowResponse&);
    void aggregator();
    void takeSomeNap(util::Duration);
    void batch(
        BatchWriteRowRequest&,
        CallbackCarrier&,
        int64_t& remains,
        int64_t& num);
    void send(int64_t concurrency);
    void waitAgain(CallbackCarrier&, BatchWriteRowRequest&);
    void prependWaitingList(std::deque<Item>&);
    void triggerCallback(const std::function<void()>& cb);

    template<class Request>
    void feedbackFromBatchReq(
        std::deque<Item>&,
        CallbackCarrier&,
        BatchWriteRowRequest&,
        BatchWriteRowResponse&);
    template<class Request>
    void feedbackAllError(
        CallbackCarrier&,
        BatchWriteRowRequest&,
        const OTSError&);
    template<class Request>
    void feedbackOkRequest(
        typename WriteTraits<Request>::SingleRowChange&,
        typename WriteTraits<Request>::Callback&,
        std::optional<Row>&,
        const std::string& requestId,
        const std::string& traceId);
    template<class Request>
    void feedbackErrRequest(
        std::deque<Item>&,
        typename WriteTraits<Request>::SingleRowChange&,
        typename WriteTraits<Request>::Callback&,
        OTSError&,
        const std::string& requestId,
        const std::string& traceId);

    template<class Request>
    void issue(
        Request&,
        const typename WriteTraits<Request>::Callback&);

public:
    static int64_t sConcurrencyIncStep;
    static int64_t sDefaultActors;

private:
    std::unique_ptr<util::Logger> mLogger;
    core::AsyncClient& mClient;

    int64_t mMaxConcurrency;
    int64_t mMaxBatchSize;
    util::Duration mRegularNap;
    util::Duration mMaxNap;
    util::Duration mNapShrinkStep;
    std::deque<std::shared_ptr<util::Actor> > mActors;

    util::Thread mAggregateThread;
    util::Semaphore mAggregateSem;
    boost::atomic<bool> mExit;
    boost::atomic<int64_t> mOngoingRequests;
    std::unique_ptr<util::Random> mRng;
    util::Mutex mMutex;
    std::deque<Item> mWaitingList;
    boost::atomic<bool> mShouldBackoff;
    boost::atomic<uint64_t> mActorSelector;
};

} // namespace impl
} // namespace core
} // namespace tablestore
} // namespace aliyun

#endif

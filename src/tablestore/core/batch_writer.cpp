#include "batch_writer.hpp"
#include "impl/async_batch_writer.hpp"
#include "tablestore/util/threading.hpp"
#include "tablestore/util/try.hpp"
#include <boost/ref.hpp>
#include <boost/noncopyable.hpp>
#include <functional>

using namespace std;

using namespace std::placeholders;
using namespace aliyun::tablestore::util;

namespace aliyun {
namespace tablestore {
namespace core {

BatchWriterConfig::BatchWriterConfig()
  : mMaxConcurrency(32),
    mMaxBatchSize(200),
    mRegularNap(util::Duration::fromMsec(10)),
    mMaxNap(util::Duration::fromSec(10)),
    mNapShrinkStep(util::Duration::fromMsec(157))
{
}

void BatchWriterConfig::prettyPrint(string& out) const
{
    pp::prettyPrint(out, "{\"MaxConcurrency\":");
    pp::prettyPrint(out, mMaxConcurrency);

    pp::prettyPrint(out, ",\"MaxBatchSize\":");
    pp::prettyPrint(out, mMaxBatchSize);

    pp::prettyPrint(out, ",\"RegularNap\":");
    pp::prettyPrint(out, mRegularNap);

    pp::prettyPrint(out, ",\"MaxNap\":");
    pp::prettyPrint(out, mMaxNap);

    pp::prettyPrint(out, ",\"NapShrinkStep\":");
    pp::prettyPrint(out, mNapShrinkStep);

    if (mActors) {
        pp::prettyPrint(out, ",\"Actors\":");
        pp::prettyPrint(out, mActors->size());
    }

    out.push_back('}');
}

std::optional<OTSError> BatchWriterConfig::validate() const
{
    if (mMaxConcurrency < 1) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Max concurrency must be positive.";
        return std::optional<OTSError>(util::move(e));
    }
    if (mMaxBatchSize < 1) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Max batch size must be positive.";
        return std::optional<OTSError>(util::move(e));
    }
    if (mRegularNap <= Duration::fromMsec(1)) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Regular nap must be greater than one msec.";
        return std::optional<OTSError>(util::move(e));
    }
    if (mMaxNap < mRegularNap * 2) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Max nap must be longer than twice regular period.";
        return std::optional<OTSError>(util::move(e));
    }
    if (mNapShrinkStep <= Duration::fromSec(0)) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Each step on shrinking nap must be positive.";
        return std::optional<OTSError>(util::move(e));
    }
    if (mActors && mActors->size() == 0) {
        OTSError e(OTSError::kPredefined_OTSParameterInvalid);
        e.mutableMessage() = "Number of invoking-callback threads must be positive.";
        return std::optional<OTSError>(util::move(e));
    }
    return std::optional<OTSError>();
}

namespace {
class SyncBatchWriterImpl : public SyncBatchWriter, private boost::noncopyable
{
public:
    explicit SyncBatchWriterImpl(AsyncBatchWriter* ac)
      : mAsyncWriter(ac)
    {}

    std::optional<OTSError> putRow(PutRowResponse&, const PutRowRequest&);
    std::optional<OTSError> updateRow(UpdateRowResponse&, const UpdateRowRequest&);
    std::optional<OTSError> deleteRow(DeleteRowResponse&, const DeleteRowRequest&);

private:
    unique_ptr<AsyncBatchWriter> mAsyncWriter;
};

template<typename Request, typename Response>
void callback(
    Semaphore& sem,
    std::optional<OTSError>& outErr,
    Response& outResp,
    Request& req,
    std::optional<OTSError>& inErr,
    Response& inResp)
{
    moveAssign(outErr, util::move(inErr));
    moveAssign(outResp, util::move(inResp));
    sem.post();
}

std::optional<OTSError> SyncBatchWriterImpl::putRow(
    PutRowResponse& resp,
    const PutRowRequest& reqIn)
{
    std::optional<OTSError> err;
    PutRowRequest req = reqIn;
    Semaphore sem(0);
    mAsyncWriter->putRow(
        req,
        bind(callback<PutRowRequest, PutRowResponse>,
            boost::ref(sem),
            boost::ref(err),
            boost::ref(resp),
            _1, _2, _3));
    sem.wait();
    return err;
}

std::optional<OTSError> SyncBatchWriterImpl::updateRow(
    UpdateRowResponse& resp,
    const UpdateRowRequest& reqIn)
{
    std::optional<OTSError> err;
    UpdateRowRequest req = reqIn;
    Semaphore sem(0);
    mAsyncWriter->updateRow(
        req,
        bind(callback<UpdateRowRequest, UpdateRowResponse>,
            boost::ref(sem),
            boost::ref(err),
            boost::ref(resp),
            _1, _2, _3));
    sem.wait();
    return err;
}

std::optional<OTSError> SyncBatchWriterImpl::deleteRow(
    DeleteRowResponse& resp,
    const DeleteRowRequest& reqIn)
{
    std::optional<OTSError> err;
    DeleteRowRequest req = reqIn;
    Semaphore sem(0);
    mAsyncWriter->deleteRow(
        req,
        bind(callback<DeleteRowRequest, DeleteRowResponse>,
            boost::ref(sem),
            boost::ref(err),
            boost::ref(resp),
            _1, _2, _3));
    sem.wait();
    return err;
}

} // namespace

std::optional<OTSError> SyncBatchWriter::create(
    SyncBatchWriter*& writer,
    AsyncClient& client,
    const BatchWriterConfig& cfg)
{
    AsyncBatchWriter* ac = NULL;
    TRY(AsyncBatchWriter::create(ac, client, cfg));
    writer = new SyncBatchWriterImpl(ac);
    return std::optional<OTSError>();
}

std::optional<OTSError> AsyncBatchWriter::create(
    AsyncBatchWriter*& writer,
    AsyncClient& client,
    const BatchWriterConfig& cfg)
{
    TRY(cfg.validate());
    writer = new impl::AsyncBatchWriter(client, cfg);
    return std::optional<OTSError>();
}

} // namespace core
} // namespace tablestore
} // namespace aliyun

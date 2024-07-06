#pragma once
#ifndef TABLESTORE_CORE_IMPL_ASYNC_CLIENT_HPP
#define TABLESTORE_CORE_IMPL_ASYNC_CLIENT_HPP
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
#include "tablestore/core/client.hpp"
#include <functional>
#include <memory>

namespace aliyun {
namespace tablestore {
namespace core {
namespace impl {
class SyncClient;
class AsyncClientBase;

class AsyncClient : public core::AsyncClient {
public:
  explicit AsyncClient(AsyncClientBase *);
  explicit AsyncClient(SyncClient &);

  util::Logger &mutableLogger();
  const std::deque<std::shared_ptr<util::Actor>> &actors() const;
  const RetryStrategy &retryStrategy() const;
  void createTable(
      CreateTableRequest &&,
      const std::function<void(CreateTableRequest &, std::optional<OTSError> &,
                               CreateTableResponse &)> &);
  void deleteTable(
      DeleteTableRequest &&,
      const std::function<void(DeleteTableRequest &, std::optional<OTSError> &,
                               DeleteTableResponse &)> &);
  void listTable(
      ListTableRequest &&,
      const std::function<void(ListTableRequest &, std::optional<OTSError> &,
                               ListTableResponse &)> &);
  void describeTable(DescribeTableRequest &&,
                     const std::function<void(DescribeTableRequest &,
                                              std::optional<OTSError> &,
                                              DescribeTableResponse &)> &);
  void updateTable(
      UpdateTableRequest &&,
      const std::function<void(UpdateTableRequest &, std::optional<OTSError> &,
                               UpdateTableResponse &)> &);
  void getRange(
      GetRangeRequest &&,
      const std::function<void(GetRangeRequest &, std::optional<OTSError> &,
                               GetRangeResponse &)> &);
  void
  putRow(PutRowRequest &&,
         const std::function<void(PutRowRequest &, std::optional<OTSError> &,
                                  PutRowResponse &)> &);
  void
  getRow(GetRowRequest &&,
         const std::function<void(GetRowRequest &, std::optional<OTSError> &,
                                  GetRowResponse &)> &);
  void updateRow(
      UpdateRowRequest &&,
      const std::function<void(UpdateRowRequest &, std::optional<OTSError> &,
                               UpdateRowResponse &)> &);
  void deleteRow(
      DeleteRowRequest &&,
      const std::function<void(DeleteRowRequest &, std::optional<OTSError> &,
                               DeleteRowResponse &)> &);
  void batchGetRow(
      BatchGetRowRequest &&,
      const std::function<void(BatchGetRowRequest &, std::optional<OTSError> &,
                               BatchGetRowResponse &)> &);
  void batchWriteRow(BatchWriteRowRequest &&,
                     const std::function<void(BatchWriteRowRequest &,
                                              std::optional<OTSError> &,
                                              BatchWriteRowResponse &)> &);
  void computeSplitsBySize(
      ComputeSplitsBySizeRequest &&,
      const std::function<void(ComputeSplitsBySizeRequest &,
                               std::optional<OTSError> &,
                               ComputeSplitsBySizeResponse &)> &);

private:
  std::shared_ptr<AsyncClientBase> mAsyncClient;

  friend class impl::SyncClient;
};

} // namespace impl
} // namespace core
} // namespace tablestore
} // namespace aliyun

#endif

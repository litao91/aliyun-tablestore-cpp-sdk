#pragma once
#ifndef TABLESTORE_CORE_IMPL_API_TRAITS_HPP
#define TABLESTORE_CORE_IMPL_API_TRAITS_HPP
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
#include <string>

namespace com {
namespace aliyun {
namespace tablestore {
namespace protocol {
class ListTableRequest;
class ListTableResponse;
class CreateTableRequest;
class CreateTableResponse;
class DeleteTableRequest;
class DeleteTableResponse;
class DescribeTableRequest;
class DescribeTableResponse;
class UpdateTableRequest;
class UpdateTableResponse;
class ComputeSplitPointsBySizeRequest;
class ComputeSplitPointsBySizeResponse;
class PutRowRequest;
class PutRowResponse;
class GetRowRequest;
class GetRowResponse;
class GetRangeRequest;
class GetRangeResponse;
class UpdateRowRequest;
class UpdateRowResponse;
class DeleteRowRequest;
class DeleteRowResponse;
class BatchGetRowRequest;
class BatchGetRowResponse;
class BatchWriteRowRequest;
class BatchWriteRowResponse;
} // namespace protocol
} // namespace tablestore
} // namespace aliyun
} // namespace com

namespace aliyun {
namespace tablestore {
namespace core {
namespace impl {

template <Action kAction> struct ApiTraits {};

template <> struct ApiTraits<kApi_ListTable> {
  using ApiRequest = aliyun::tablestore::core::ListTableRequest;
  using ApiResponse = aliyun::tablestore::core::ListTableResponse;
  using PbRequest = com::aliyun::tablestore::protocol::ListTableRequest;
  using PbResponse = com::aliyun::tablestore::protocol::ListTableResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_CreateTable> {
  using ApiRequest = aliyun::tablestore::core::CreateTableRequest;
  using ApiResponse = aliyun::tablestore::core::CreateTableResponse;
  using PbRequest = com::aliyun::tablestore::protocol::CreateTableRequest;
  using PbResponse = com::aliyun::tablestore::protocol::CreateTableResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_DeleteTable> {
  using ApiRequest = aliyun::tablestore::core::DeleteTableRequest;
  using ApiResponse = aliyun::tablestore::core::DeleteTableResponse;
  using PbRequest = com::aliyun::tablestore::protocol::DeleteTableRequest;
  using PbResponse = com::aliyun::tablestore::protocol::DeleteTableResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_DescribeTable> {
  using ApiRequest = aliyun::tablestore::core::DescribeTableRequest;
  using ApiResponse = aliyun::tablestore::core::DescribeTableResponse;
  using PbRequest = com::aliyun::tablestore::protocol::DescribeTableRequest;
  using PbResponse = com::aliyun::tablestore::protocol::DescribeTableResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_UpdateTable> {
  using ApiRequest = aliyun::tablestore::core::UpdateTableRequest;
  using ApiResponse = aliyun::tablestore::core::UpdateTableResponse;
  using PbRequest = com::aliyun::tablestore::protocol::UpdateTableRequest;
  using PbResponse = com::aliyun::tablestore::protocol::UpdateTableResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_ComputeSplitsBySize> {
  using ApiRequest = aliyun::tablestore::core::ComputeSplitsBySizeRequest;
  using ApiResponse = aliyun::tablestore::core::ComputeSplitsBySizeResponse;
  using PbRequest =
      com::aliyun::tablestore::protocol::ComputeSplitPointsBySizeRequest;
  using PbResponse =
      com::aliyun::tablestore::protocol::ComputeSplitPointsBySizeResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_PutRow> {
  using ApiRequest = aliyun::tablestore::core::PutRowRequest;
  using ApiResponse = aliyun::tablestore::core::PutRowResponse;
  using PbRequest = com::aliyun::tablestore::protocol::PutRowRequest;
  using PbResponse = com::aliyun::tablestore::protocol::PutRowResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_GetRow> {
  using ApiRequest = aliyun::tablestore::core::GetRowRequest;
  using ApiResponse = aliyun::tablestore::core::GetRowResponse;
  using PbRequest = com::aliyun::tablestore::protocol::GetRowRequest;
  using PbResponse = com::aliyun::tablestore::protocol::GetRowResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_GetRange> {
  using ApiRequest = aliyun::tablestore::core::GetRangeRequest;
  using ApiResponse = aliyun::tablestore::core::GetRangeResponse;
  using PbRequest = com::aliyun::tablestore::protocol::GetRangeRequest;
  using PbResponse = com::aliyun::tablestore::protocol::GetRangeResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_UpdateRow> {
  using ApiRequest = aliyun::tablestore::core::UpdateRowRequest;
  using ApiResponse = aliyun::tablestore::core::UpdateRowResponse;
  using PbRequest = com::aliyun::tablestore::protocol::UpdateRowRequest;
  using PbResponse = com::aliyun::tablestore::protocol::UpdateRowResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_DeleteRow> {
  using ApiRequest = aliyun::tablestore::core::DeleteRowRequest;
  using ApiResponse = aliyun::tablestore::core::DeleteRowResponse;
  using PbRequest = com::aliyun::tablestore::protocol::DeleteRowRequest;
  using PbResponse = com::aliyun::tablestore::protocol::DeleteRowResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_BatchGetRow> {
  using ApiRequest = aliyun::tablestore::core::BatchGetRowRequest;
  using ApiResponse = aliyun::tablestore::core::BatchGetRowResponse;
  using PbRequest = com::aliyun::tablestore::protocol::BatchGetRowRequest;
  using PbResponse = com::aliyun::tablestore::protocol::BatchGetRowResponse;
  static const std::string kPath;
};

template <> struct ApiTraits<kApi_BatchWriteRow> {
  using ApiRequest = aliyun::tablestore::core::BatchWriteRowRequest;
  using ApiResponse = aliyun::tablestore::core::BatchWriteRowResponse;
  using PbRequest = com::aliyun::tablestore::protocol::BatchWriteRowRequest;
  using PbResponse = com::aliyun::tablestore::protocol::BatchWriteRowResponse;
  static const std::string kPath;
};

} // namespace impl
} // namespace core
} // namespace tablestore
} // namespace aliyun

#endif

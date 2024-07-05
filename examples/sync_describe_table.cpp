#include "tablestore/core/client.hpp"
#include "tablestore/core/types.hpp"
#include "tablestore/util/logging.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
extern "C" {
#include <unistd.h>
}

using namespace std;
using namespace aliyun::tablestore;
using namespace aliyun::tablestore::util;
using namespace aliyun::tablestore::core;

SyncClient *initOtsClient() {
  Endpoint ep("YourEndpoint", "YourInstance");
  Credential cr("AccessKeyId", "AccessKeySecret");
  ClientOptions opts;
  SyncClient *pclient = NULL;
  {
    std::optional<OTSError> res = SyncClient::create(pclient, ep, cr, opts);
    assert(!res);
  }
  sleep(30); // wait a while for connection ready
  return pclient;
}

const char kTableName[] = "describe_table";

void createTable(SyncClient &client) {
  CreateTableRequest req;
  {
    // immutable configurations of the table
    TableMeta &meta = req.mutableMeta();
    meta.mutableTableName() = kTableName;
    {
      // with exactly one integer primary key column
      Schema &schema = meta.mutableSchema();
      PrimaryKeyColumnSchema &pkColSchema = schema.append();
      pkColSchema.mutableName() = "pkey";
      pkColSchema.mutableType() = kPKT_Integer;
    }
  }
  CreateTableResponse resp;
  std::optional<OTSError> res = client.createTable(resp, req);
  cout << "create table \"" << kTableName << "\" ";
  if (res) {
    cout << "error" << endl
         << "  error code: " << res->errorCode() << endl
         << "  message: " << res->message() << endl
         << "  HTTP status: " << res->httpStatus() << endl
         << "  request id: " << res->requestId() << endl
         << "  trace id: " << res->traceId() << endl;
  } else {
    cout << "OK" << endl
         << "  request id: " << resp.requestId() << endl
         << "  trace id: " << resp.traceId() << endl;
  }
}

void describeTable(SyncClient &client) {
  DescribeTableRequest req;
  req.mutableTable() = kTableName;
  DescribeTableResponse resp;
  std::optional<OTSError> res = client.describeTable(resp, req);
  cout << "describe table \"" << kTableName << "\" ";
  if (res) {
    cout << "error" << endl
         << "  error code: " << res->errorCode() << endl
         << "  message: " << res->message() << endl
         << "  HTTP status: " << res->httpStatus() << endl
         << "  request id: " << res->requestId() << endl
         << "  trace id: " << res->traceId() << endl;
  } else {
    cout << "OK" << endl << "  " << pp::prettyPrint(resp) << endl;
  }
}

void deleteTable(SyncClient &client) {
  DeleteTableRequest req;
  req.mutableTable() = kTableName;
  DeleteTableResponse resp;
  std::optional<OTSError> res = client.deleteTable(resp, req);
  cout << "delete table \"" << kTableName << "\" ";
  if (res) {
    cout << "error" << endl
         << "  error code: " << res->errorCode() << endl
         << "  message: " << res->message() << endl
         << "  HTTP status: " << res->httpStatus() << endl
         << "  request id: " << res->requestId() << endl
         << "  trace id: " << res->traceId() << endl;
  } else {
    cout << "OK" << endl
         << "  request id: " << resp.requestId() << endl
         << "  trace id: " << resp.traceId() << endl;
  }
}

int main() {
  std::unique_ptr<SyncClient> client(initOtsClient());
  createTable(*client);
  describeTable(*client);
  deleteTable(*client);
  return 0;
}

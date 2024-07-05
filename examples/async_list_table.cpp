#include "tablestore/core/types.hpp"
#include "tablestore/core/client.hpp"
#include "tablestore/util/logging.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
extern "C" {
#include <unistd.h>
}

using namespace std;
using namespace aliyun::tablestore::util;
using namespace aliyun::tablestore::core;

AsyncClient* initOtsClient()
{
    Endpoint ep("YourEndpoint", "YourInstance");
    Credential cr("AccessKeyId", "AccessKeySecret");
    ClientOptions opts;
    AsyncClient* pclient = NULL;
    {
        std::optional<OTSError> res = AsyncClient::create(pclient, ep, cr, opts);
        assert(!res);
    }
    sleep(30); // wait a while for connection ready
    return pclient;
}

void listTableCallback(
    ListTableRequest&,
    std::optional<OTSError>& err,
    ListTableResponse& resp)
{
    if (err) {
        cout << pp::prettyPrint(*err) << endl;
    } else {
        const IVector<string>& xs = resp.tables();
        for(int64_t i = 0; i < xs.size(); ++i) {
            cout << xs[i] << endl;
        }
    }
}

void listTable(AsyncClient& client)
{
    ListTableRequest req;
    client.listTable(req, listTableCallback);
}

int main() {
    unique_ptr<AsyncClient> client(initOtsClient());
    listTable(*client);
    return 0;
}


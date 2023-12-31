// this file is auto generated by program,do not edit
#include "RpcServiceClient.h"
#include "protocols.pb.h"
#include <functional>
using namespace rpc;
class NormalServiceClient : public RpcServiceClient {
public:
  NormalServiceClient(std::string saddr, short sport)
      : RpcServiceClient(saddr, sport) {
    addRpcMethod("NormalService.GetTime");
    Connect();
  }
  ~NormalServiceClient() {}
  bool GetTime(GetTimeReq &request, GetTimeResp &response) {
    rpcCall(request, response, 0);
  }
};

// this file is auto generated by program,do not edit
#include "NormalService.h"
NormalService::NormalService() {
  RegisterDescriptors(
      NormalService_method_names[0],
      std::make_shared<service_method_descriptor>(
          NormalService_method_names[0],
          new RpcMethodHandler<GetTimeReq, GetTimeResp>(
              std::bind(&NormalService::GetTime, this, _1, _2))));
}
NormalService::~NormalService() {}

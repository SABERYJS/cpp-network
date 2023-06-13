#ifndef MT_NORMAL_SERVICE_IMPL_H
#define MT_NORMAL_SERVICE_IMPL_H

#include "NormalService.h"
#include<ctime>
#include<spdlog/spdlog.h>

class NormalServiceImpl :public NormalService {
public:
    NormalServiceImpl(/* args */) {}
    ~NormalServiceImpl() {}
    virtual Status GetTime(GetTimeReq* request, GetTimeResp* response)override {
        spdlog::info("req ctime {}", request->ctime());
        response->set_time(time(nullptr));
        return Status::OK;
    }
};


#endif
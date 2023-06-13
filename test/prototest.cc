#include"protocompiled/protocols.pb.h"
#include<iostream>
#include"CommonUtils.h"
#include "DnsResolver.h"
#include<iostream>

using namespace std;

int main(int argc, char const* argv[]) {
    auto ret = DnsResolver::Resolve("www.baidu.com");
    for (auto ptr = ret.begin();ptr != ret.end();ptr++) {
        std::cout << *ptr << std::endl;
    }
    return 0;
}

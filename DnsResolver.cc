#include"DnsResolver.h"

vector<string> DnsResolver::Resolve(const char* hostname) {
    int sockfd;
    vector<string>result;
    struct addrinfo hints, * servinfo, * p;
    struct sockaddr_in* h;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    //The return value from inet_ntoa() points to a buffer which is overwritten on each call. 
    //This buffer is implemented as thread-specific data in multithreaded applications.
    //IPv4-mapped addresses are not recommended.
    if ((rv = getaddrinfo(hostname, "80", &hints, &servinfo)) != 0) {
        throw std::runtime_error("dns resolve error");
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        h = (struct sockaddr_in*)p->ai_addr;
        result.push_back(inet_ntoa(h->sin_addr));
    }

    freeaddrinfo(servinfo); // all done with this structure
    return result;
}
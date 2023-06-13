#ifndef MT_DNS_RESOLVER_H
#define MT_DNS_RESOLVER_H
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h>	//hostent
#include<arpa/inet.h>
#include<vector>
#include<string>
#include<stdexcept>

using namespace std;

//https://cppsecrets.com/users/18989711511997116104103495564103109971051084699111109/C00-Socket-Programming-getaddrinfo.php
class DnsResolver {
public:
    static vector<string> Resolve(const char* hostname);
};


#endif
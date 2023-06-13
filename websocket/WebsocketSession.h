#ifndef MT_WEBSOCKET_SESSION_H
#define MT_WEBSOCKET_SESSION_H

#include "TcpSession.h"
#include "HttpRequest.h"
#include "base64.h"
#include <openssl/sha.h>
#include "WebsocketException.h"
#include<cctype>
#include"DynamicBytes.h"
#include<algorithm>



static const char* HostHeader = "Host";
static const char* UpgradeHeader = "Upgrade";
static const char* ConnectionHeader = "Connection";
static const char* SecWebsocketKeyHeader = "Sec-WebSocket-Key";
static const char* OriginHeader = "Origin";
static const char* SecWebsocketVersionHeader = "Sec-WebSocket-Version";
static const char* SecWebsocketProtocolHeader = "Sec-WebSocket-Protocol";
static const char* SecWebsocketExtensionHeader = "Sec-WebSocket-Extensions";



#define MAX_FRAME_SIZE  65535 //2^16-1

//reference https://datatracker.ietf.org/doc/html/rfc6455
/**
 *  websocket request
 *  GET /chat HTTP/1.1
    Host: server.example.com
    Upgrade: websocket
    Connection: Upgrade
    Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
    Origin: http://example.com
    Sec-WebSocket-Protocol: chat, superchat
    Sec-WebSocket-Version: 13
 *
*/

/**
 *  HTTP/1.1 101 Switching Protocols
    Upgrade: websocket
    Connection: Upgrade
    Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
    Sec-WebSocket-Protocol: chat
 *
*/
class WebsocketSession :public TcpSession {
private:

    //reference https://datatracker.ietf.org/doc/html/rfc6455#section-5.2
    struct frame {
        unsigned int fin : 1;
        unsigned int rsv1 : 1;
        unsigned int rsv2 : 1;
        unsigned int rsv3 : 1;
        unsigned int opcode : 4;
        unsigned int mask : 1;
        unsigned int payload_len : 7;
        uint8_t mask_keys[4];
        int mask_index{ 0 };
    };

    enum parse_frame_state {
        ParseBase, //parse most left 2 bytes
        ParseExtPayloadLength,
        ParseMaskKey,
        ParsePayloadData,
        ParseDone
    };
    //3-7 are reserved for further non-control frames
    enum websocket_opcodes {
        OpCodeContinueFrame = 0,
        OpCodeTextFrame = 1,
        OpCodeBinaryFrame = 2,
        OpCodeReserved3 = 3,
        OpCodeReserved4 = 4,
        OpCodeReserved5 = 5,
        OpCodeReserved6 = 6,
        OpCodeReserved7 = 7,
        OpCodeConnectionClose = 8,
        OpCodePing = 9,
        OpCodePong = 10
    };
    HttpRequest request;
    bool handshaked{ false };
    typedef HttpRequest::Result  HResult;
    string sec_websocket_key{};
    frame current{ 0 };
    parse_frame_state state{ parse_frame_state::ParseBase };
    DynamicBytes payload;
    DynamicBytes temporary;

public:
    WebsocketSession(/* args */);
    ~WebsocketSession();
    void SendClientData(const char* data, size_t len, websocket_opcodes opcode = websocket_opcodes::OpCodeTextFrame);
    void SendClientData(string& s, websocket_opcodes opcode = websocket_opcodes::OpCodeTextFrame);
    void SendClientData(string s, websocket_opcodes opcode = websocket_opcodes::OpCodeTextFrame);
protected:
    virtual void ProcessRead(const char* ptr, int size) override;
    virtual bool match_uri(const string& s) {
        return s == "/chat";
    }
    //subclass should implement this
    virtual void DispatchMessage(const char* ptr, size_t len) {
        spdlog::info("client msg: {}", string(ptr, len));
        SendClientData(ptr, len);
    };
private:
    bool check_sec_websocket_key_invalid(const string& key);
    bool check_sec_websocket_version_invalid(const string& version);
    void do_handshake(const char* ptr, int size);
    void send_upgrade_response();
    void parse_message(const char* ptr, int size);
    int parse_base_header(const char* ptr, int size);
    int parse_ext_payload_length(const char* ptr, int size);
    int parse_mask_key(const char* ptr, int size);
    int parse_payload_data(const char* ptr, int size);
    void reset_parse_state();
};

#endif
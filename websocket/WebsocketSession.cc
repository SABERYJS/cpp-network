#include "WebsocketSession.h"

WebsocketSession::WebsocketSession() {}
WebsocketSession::~WebsocketSession() {}


void WebsocketSession::do_handshake(const char* ptr, int size) {
    HResult result = request.Parse(ptr, size);
    if (result == HResult::Failed) {
        handleError(0);
        return;
    }
    setConsumed(request.GetCounter());
    if (request.CheckDone()) {
        //parse request finished
        if (request.GetMethod() != HttpRequest::request_method::Get) {
            //The method of the request MUST be GET, and the HTTP version MUSTbe at least 1.1.
            handleError(0);
            return;
        }
        //check uri
        if (!match_uri(request.GetUri())) {
            handleError(0);
            return;
        }
        //spdlog::info("{}", request.GetHeaderValue(UpgradeHeader));
        if (!CommonUtils::CompareIgnoreCase(request.GetHeaderValue(UpgradeHeader), "websocket")) {
            //The request MUST contain an |Upgrade| header field whose value MUST include the "websocket" keyword.
            handleError(0);
            return;
        }
        if (!CommonUtils::CompareIgnoreCase(request.GetHeaderValue(ConnectionHeader), "Upgrade")) {
            //The request MUST contain a |Connection| header field whose value MUST include the "Upgrade" token.
            handleError(0);
            return;
        }
        if (!check_sec_websocket_key_invalid(request.GetHeaderValue(SecWebsocketKeyHeader))) {
            handleError(0);
            return;
        }
        if (!check_sec_websocket_version_invalid(request.GetHeaderValue(SecWebsocketVersionHeader))) {
            handleError(0);
            return;
        }
        sec_websocket_key = request.GetHeaderValue(SecWebsocketKeyHeader);
        send_upgrade_response();
        handshaked = true;
    }
}

//The server takes the value of the Sec-WebSocket-Key sent in the handshake request, 
//appends 258EAFA5-E914-47DA-95CA-C5AB0DC85B11, takes SHA-1 of the new value, and is then base64 encoded.
void WebsocketSession::send_upgrade_response() {
    string raw = fmt::format("{}258EAFA5-E914-47DA-95CA-C5AB0DC85B11", sec_websocket_key);
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(raw.c_str()), raw.size(), hash);
    string encoded = base64_encode(hash, SHA_DIGEST_LENGTH);
    spdlog::info("Sec-WebSocket-Accept:{}", encoded);
    //Error during WebSocket handshake: Response must not include 'Sec-WebSocket-Protocol' header if not present in request: chat
    WriteData(fmt::format("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept:{}\r\n\r\n", encoded));
}

void WebsocketSession::ProcessRead(const char* ptr, int size) {
    if (check_closed()) {
        return;
    }

    if (!handshaked) {
        do_handshake(ptr, size);
    }
    else {
        parse_message(ptr, size);
    }
}

bool WebsocketSession::check_sec_websocket_key_invalid(const string& key) {
    if (base64_decode(key).size() != 16) {
        return false;
    }
    else {
        return true;
    }
}

bool WebsocketSession::check_sec_websocket_version_invalid(const string& version) {
    if (version != "13") {
        return false;
    }
    else {
        return true;
    }
}

/**
 *       0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
 *
*/
void WebsocketSession::parse_message(const char* ptr, int size) {
    int bytes = 0;
    while ((size - bytes) > 0) {
        try {
            if (state == parse_frame_state::ParseBase) {
                bytes += parse_base_header(ptr + bytes, size - bytes);
            }
            if (state == parse_frame_state::ParseExtPayloadLength && (size - bytes) > 0) {
                bytes += parse_ext_payload_length(ptr + bytes, size - bytes);
            }

            if (state == parse_frame_state::ParseMaskKey && (size - bytes) > 0) {
                bytes += parse_mask_key(ptr + bytes, size - bytes);
            }

            if (state == parse_frame_state::ParsePayloadData && (size - bytes) > 0) {
                bytes += parse_payload_data(ptr + bytes, size - bytes);
            }

            if (state != parse_frame_state::ParseBase) {
                break;
            }
            if ((size - bytes) < 2) {
                break;
            }
            else {
                continue;
            }
        }
        catch (const WebsocketException& e) {
            spdlog::error("parse websocket failed,{}", e.what());
            handleError(0);
            return;
        }
    }
    setConsumed(bytes);
}

int WebsocketSession::parse_base_header(const char* ptr, int size) {
    if (size < 2) {
        //base length
        return 0;
    }
    current.fin = (static_cast<uint8_t>(ptr[0]) & 0x80) >> 7;//fin
    if (current.fin != 1) {
        throw WebsocketException("fragmented message not support");
    }
    current.rsv1 = (static_cast<uint8_t>(ptr[0]) & 0x40) >> 6;//rsv1
    current.rsv2 = (static_cast<uint8_t>(ptr[0]) & 0x20) >> 5;//rsv2
    current.rsv3 = (static_cast<uint8_t>(ptr[0]) & 0x10) >> 4;//rsv3
    current.opcode = static_cast<uint8_t>(ptr[0]) & 0x0f; //opcode
    if (current.opcode == websocket_opcodes::OpCodeBinaryFrame) {
        throw WebsocketException("binary frame not support");
    }
    current.mask = (static_cast<uint8_t>(ptr[1]) & 0x80) >> 7;//mask
    if (current.mask != 1) {
        //All frames sent from client to server have this bit set to 1.
        throw WebsocketException("mask invalid,message that send from client must be masked");
    }
    current.payload_len = static_cast<uint8_t>(ptr[1]) & 0x7f;//lower 7 bits
    if (current.payload_len <= 125) {
        //7 bits
        state = parse_frame_state::ParseMaskKey;
    }
    else if (current.payload_len == 126) {
        //next 16 bits
        state = parse_frame_state::ParseExtPayloadLength;
    }
    else {
        //next 64 bits
        //not support,too big
        throw WebsocketException("payload length too length");
    }
    return 2;
}

int WebsocketSession::parse_ext_payload_length(const char* ptr, int size) {
    if (state == parse_frame_state::ParseExtPayloadLength) {
        if (size < 2) {
            return 0;
        }
        current.payload_len = (static_cast<uint16_t>(ptr[0]) << 8 & 0xff00) | (static_cast<uint16_t>(ptr[1]) & 0x00ff);
        state = parse_frame_state::ParseMaskKey;
        return 2;
    }
    else {
        return 0;
    }
}

int WebsocketSession::parse_mask_key(const char* ptr, int size) {
    if (state == parse_frame_state::ParseMaskKey) {
        if (size < 4) {
            return 0;
        }
        current.mask_keys[0] = static_cast<uint8_t>(ptr[0]);
        current.mask_keys[1] = static_cast<uint8_t>(ptr[1]);
        current.mask_keys[2] = static_cast<uint8_t>(ptr[2]);
        current.mask_keys[3] = static_cast<uint8_t>(ptr[3]);
        state = parse_frame_state::ParsePayloadData;
        return 4;
    }
    else {
        return 0;
    }
}

int WebsocketSession::parse_payload_data(const char* ptr, int size) {
    if (state == parse_frame_state::ParsePayloadData) {
        if (size < 0) {
            return 0;
        }
        int i = 0;
        uint8_t mask_char;
        int require = current.payload_len - current.mask_index;
        payload.Prepare(require);
        char* dest = payload.CData();
        int csize = std::min(size, static_cast<int>(require));
        while (i < csize) {
            mask_char = current.mask_keys[current.mask_index % 4];
            *(dest + i) = mask_char ^ static_cast<uint8_t>(ptr[i]);
            current.mask_index++;
            ++i;
        }
        payload.Commit(csize);
        if (current.mask_index == current.payload_len) {
            DispatchMessage(payload.Data(), current.payload_len);
            payload.Consume(current.payload_len);
            reset_parse_state();
        }
        return csize;
    }
    else {
        return 0;
    }
}

void WebsocketSession::reset_parse_state() {
    current = {};
    state = parse_frame_state::ParseBase;
}


//mask set to 0
//no mask key
void WebsocketSession::SendClientData(const char* data, size_t len, websocket_opcodes opcode) {
    if (len > MAX_FRAME_SIZE) {
        throw WebsocketException("frame too large");
    }
    size_t require_len = 0;
    if (len <= 125) {
        require_len = 2 + len;
    }
    else {
        require_len = 4 + len;
    }
    bool use_temporary = true;
    char* destination = nullptr;
    if (require_len > temporary.CSize()) {
        use_temporary = false;
        destination = new char[require_len];
    }
    else {
        destination = temporary.CData();
    }
    destination[0] = (uint8_t)(0x80 | (opcode & 0x0f));//first byte
    if (len <= 125) {
        destination[1] = (uint8_t)(0x7F & len);
        copy(data, data + len, destination + 2);
    }
    else {
        destination[1] = (uint8_t)(0x7F & 126);
        destination[2] = (uint8_t)(0xff00 & len >> 8);
        destination[3] = (uint8_t)(0x00ff & len);
        copy(data, data + len, destination + 2);
    }
    WriteData(destination, require_len);
    if (!use_temporary) {
        delete[] destination;
    }
    else {
        temporary.Clear();
    }
}

void WebsocketSession::SendClientData(string& s, websocket_opcodes opcode) {
    SendClientData(s.c_str(), s.size(), opcode);
}

void WebsocketSession::SendClientData(string s, websocket_opcodes opcode) {
    SendClientData(s.c_str(), s.size(), opcode);
}
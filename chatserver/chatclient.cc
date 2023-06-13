
#include "BaseTcpClient.h"
#include<cstdlib>
#include "protocompiled/protocols.pb.h"
#include<condition_variable>
#include<thread>
#include "ProtobufAssembler.h"
#include "DynamicBytes.h"
#include<functional>
#include<algorithm>
#include<string>
#include<chrono>

const std::string letters("abcdefghijklmnopqrstuvwxyz");

std::condition_variable condv;
std::mutex mtx;

std::string rand_string(int len) {
    string s;
    s.resize(len);
    for (int i = 0;i < len;i++) {
        s[i] = letters[rand() % letters.size()];
    }
    return s;
}


int identity = 0;
int peer = 0;
int mode = 0;

ProtobufAssembler<rpc::Message> assembler;


void send_auth(BaseTcpClient& client) {
    rpc::Message msg;
    msg.set_mtype(rpc::MessageType::MessageTypeAuth);
    msg.mutable_rauth()->set_uid(identity);
    string password = "password";
    msg.mutable_rauth()->set_password(password);

    char data[100];
    std::cout << msg.ByteSize() << std::endl;
    CommonUtils::EncodeInt16(data, msg.ByteSize());
    msg.SerializeToArray(data + 2, 100);
    if (client.Send(data, 2 + msg.ByteSize()) < 0) {
        exit(1);
    }
}

void send_chat_msg(BaseTcpClient& client) {
    while (true) {
        std::string text;
        std::getline(std::cin, text);
        if (text.size() == 0) {
            continue;
        }

        rpc::Message msg;
        msg.set_mtype(rpc::MessageType::MessageTypeChatMsg);
        msg.mutable_smsg()->set_peer(peer);
        msg.mutable_smsg()->set_msgtype(rpc::ChatMsgType::ChatMsgTypeText);
        msg.mutable_smsg()->set_text(text);

        char data[100];
        std::cout << msg.ByteSize() << std::endl;
        CommonUtils::EncodeInt16(data, msg.ByteSize());
        msg.SerializeToArray(data + 2, 100);
        if (client.Send(data, 2 + msg.ByteSize()) < 0) {
            exit(1);
        }
    }
}


int main(int argc, char const* argv[]) {

    if (argc < 3) {
        std::cerr << "invalid arguments" << std::endl;
        exit(1);
    }

    identity = stoi(argv[1]);
    peer = stoi(argv[2]);
    if (argc >= 4) {
        mode = stoi(argv[3]);
    }

    std::cout << "current:" << identity << " talk to " << peer << std::endl;

    srand(static_cast<unsigned int>(time(nullptr)));
    BaseTcpClient client("127.0.0.1", 90);
    client.Connect();

    std::thread rthread([&client]() {
        DynamicBytes buffer;
        while (true) {
            int bytes;
            auto data = client.ReadSome(bytes, 256);
            if (bytes == 0) {
                std::cout << "server closed connection" << std::endl;
                exit(1);
            }
            else  if (bytes < 0) {
                std::cerr << "unknown error" << std::endl;
                exit(1);
            }
            else {
                buffer.Prepare(bytes);
                copy(data.get(), data.get() + bytes, buffer.CData());
                buffer.Commit(bytes);
                rpc::Message msg;
                int ret = 0;
                if ((ret = assembler.Decode(buffer, msg)) < 0) {
                    std::cerr << "decode error" << std::endl;
                    exit(1);
                }
                else if (ret == 0) {
                    continue;
                }
                else {
                    std::cout << "mtype:" << msg.mtype();
                    if (msg.mtype() == rpc::MessageType::MessageTypeChatMsg) {
                        std::cout << " chatmsg: " << msg.smsg().msgtype();
                        if (msg.smsg().msgtype() == rpc::ChatMsgType::ChatMsgTypeText) {
                            std::cout << " text:" << msg.smsg().text() << std::endl;
                        }
                    }
                }
            }
        }

        });
    rthread.detach();

    send_auth(client);
    if (mode == 0) {
        send_chat_msg(client);
    }
    else {
        std::thread send_thread([&client]() {
            while (true) {
                std::string text = std::move(rand_string(16));

                rpc::Message msg;
                msg.set_mtype(rpc::MessageType::MessageTypeChatMsg);
                msg.mutable_smsg()->set_peer(peer);
                msg.mutable_smsg()->set_msgtype(rpc::ChatMsgType::ChatMsgTypeText);
                msg.mutable_smsg()->set_text(text);

                char data[100];
                std::cout << msg.ByteSize() << std::endl;
                CommonUtils::EncodeInt16(data, msg.ByteSize());
                msg.SerializeToArray(data + 2, 100);
                if (client.Send(data, 2 + msg.ByteSize()) < 0) {
                    exit(1);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            });
        send_thread.detach();
    }


    std::unique_lock<std::mutex>lock(mtx);
    condv.wait(lock);

    return 0;
}

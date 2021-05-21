/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <p2p/base/basic_packet_socket_factory.h>
#include <p2p/base/port_interface.h>
#include <p2p/base/turn_server.h>
#include <rtc_base/async_udp_socket.h>
#include <rtc_base/ip_address.h>
#include <rtc_base/socket_address.h>
#include <rtc_base/socket_server.h>
#include <rtc_base/thread.h>

#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <string>
#include <utility>

namespace webrtc_examples {

std::map<std::string, std::string> ReadAuthFile(std::istream* s) {
    std::map<std::string, std::string> name_to_key;
    for (std::string line; std::getline(*s, line);) {
        const size_t sep = line.find('=');
        if (sep == std::string::npos) continue;
        char buf[32];
        size_t len = rtc::hex_decode(buf, sizeof(buf), line.data() + sep + 1,
                                     line.size() - sep - 1);
        if (len > 0) {
            name_to_key.emplace(line.substr(0, sep), std::string(buf, len));
        }
    }
    return name_to_key;
}

}  // namespace webrtc_examples

namespace {
const char kSoftware[] = "libjingle TurnServer";

class TurnFileAuth : public cricket::TurnAuthInterface {
public:
    explicit TurnFileAuth(std::map<std::string, std::string> name_to_key)
        : name_to_key_(std::move(name_to_key)) {}

    virtual bool GetKey(const std::string& username,
                        const std::string& realm,
                        std::string* key) {
        // File is stored as lines of <username>=<HA1>.
        // Generate HA1 via "echo -n "<username>:<realm>:<password>" | md5sum"
        auto it = name_to_key_.find(username);
        if (it == name_to_key_.end()) return false;
        *key = it->second;
        return true;
    }

private:
    const std::map<std::string, std::string> name_to_key_;
};

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "usage: turnserver int-addr ext-ip realm auth-file"
                  << std::endl;
        return 1;
    }

    rtc::SocketAddress int_addr;
    if (!int_addr.FromString(argv[1])) {
        std::cerr << "Unable to parse IP address: " << argv[1] << std::endl;
        return 1;
    }

    rtc::IPAddress ext_addr;
    if (!IPFromString(argv[2], &ext_addr)) {
        std::cerr << "Unable to parse IP address: " << argv[2] << std::endl;
        return 1;
    }

    rtc::Thread* main = rtc::Thread::Current();
    rtc::AsyncUDPSocket* int_socket =
            rtc::AsyncUDPSocket::Create(main->socketserver(), int_addr);
    if (!int_socket) {
        std::cerr << "Failed to create a UDP socket bound at"
                  << int_addr.ToString() << std::endl;
        return 1;
    }

    cricket::TurnServer server(main);
    std::fstream auth_file(argv[4], std::fstream::in);

    TurnFileAuth auth(auth_file.is_open()
                              ? webrtc_examples::ReadAuthFile(&auth_file)
                              : std::map<std::string, std::string>());
    server.set_realm(argv[3]);
    server.set_software(kSoftware);
    server.set_auth_hook(&auth);
    server.AddInternalSocket(int_socket, cricket::PROTO_UDP);
    server.SetExternalSocketFactory(new rtc::BasicPacketSocketFactory(),
                                    rtc::SocketAddress(ext_addr, 0));

    std::cout << "Listening internally at " << int_addr.ToString() << std::endl;

    main->Run();
    return 0;
}

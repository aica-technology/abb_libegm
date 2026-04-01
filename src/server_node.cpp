#include <iostream>
#include <boost/asio.hpp>
#include "egm.pb.h"

using boost::asio::ip::udp;

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 6511)); // Use your desired port

    std::array<char, 2048> recv_buf;
    udp::endpoint remote_endpoint;

    while (true) {
        boost::system::error_code error;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

        if (error && error != boost::asio::error::message_size) {
            std::cerr << "Receive failed: " << error.message() << std::endl;
            continue;
        }

        abb::egm::EgmRobot msg;
        if (msg.ParseFromArray(recv_buf.data(), static_cast<int>(len))) {
            std::cout << "Received EgmRobot message!" << std::endl;
            if (msg.has_header()) {
                std::cout << "Seqno: " << msg.header().seqno() << std::endl;
            }
            // Print joint data if available
            if (msg.has_feedback() && msg.feedback().has_joints()) {
                const auto& joints = msg.feedback().joints();
                std::cout << "Joints: ";
                for (int i = 0; i < joints.joints_size(); ++i) {
                    std::cout << joints.joints(i);
                    if (i < joints.joints_size() - 1) std::cout << ", ";
                }
                std::cout << std::endl;
            }

            // Print measured forces if available
            if (msg.has_measuredforce()) {
                const auto& force = msg.measuredforce();
                std::cout << "Forces: ";
                for (int i = 0; i < force.force_size(); ++i) {
                    std::cout << force.force(i);
                    if (i < force.force_size() - 1) std::cout << ", ";
                }
                std::cout << std::endl;
            }
        } else {
            std::cerr << "Failed to parse EgmRobot message." << std::endl;
        }
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
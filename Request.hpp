#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>

int sockfd;

constexpr int PORT = 8080;
constexpr int MAX_CLIENTS = 10;
constexpr size_t MAX_REQUEST_SIZE = 4096;

void initServer() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    listen(sockfd, MAX_CLIENTS);
}

struct Request {
    int client;

    static Request get() {
        return Request{.client=accept(sockfd, nullptr, nullptr)};
    }

    std::string readRequest() {
        char buf[MAX_REQUEST_SIZE];
        ssize_t readSize = read(client, buf, sizeof(buf) - 1);
        return std::string(buf, readSize);
    }

    void respond(const std::string& message) {
        write(client, message.c_str(), message.size());
    }

    ~Request() {close(client);}
};

void closeServer() {
    close(sockfd);
}

#endif
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <functional>

int sockfd;

constexpr int PORT = 8080;
constexpr int MAX_CLIENTS = 10;
constexpr size_t MAX_REQUEST_SIZE = 4096;

static inline void handleSocketFail() {
    std::cerr << "Fatal Error: Failed to register a socket, terminating program\n";
    exit(EXIT_FAILURE);
}
static inline void handleFailedSetSockOpt() {
    std::cerr << "Fatal Error: Failed to set sock opt, terminating program\n";
    exit(EXIT_FAILURE);
}
static inline void handleFailedBind() {
    std::cerr << "Fatal Error: Failed to bind, terminating program\n";
    exit(EXIT_FAILURE);
}
static inline void handleFailedListen() {
    std::cerr << "Fatal Error: listen() failed, terminating program\n";
    exit(EXIT_FAILURE);
}

using FailedSocketHandler = std::function<void()>;
using FailedSetSockOptHandler = std::function<void()>;
using FailedBindHandler = std::function<void()>;
using FailedListenHandler = std::function<void()>;

static inline FailedSocketHandler failedSocket = handleSocketFail;
static inline FailedSetSockOptHandler failedSetSockOpt = handleFailedSetSockOpt;
static inline FailedBindHandler failedBind = handleFailedBind;
static inline FailedListenHandler failedListen = handleFailedListen;

inline void initServer() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        failedSocket();
        return;
    }
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        failedSetSockOpt();
        close(sockfd);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        failedBind();
        close(sockfd);
        return;
    }

    if (listen(sockfd, MAX_CLIENTS) < 0) {
        failedListen();
        close(sockfd);
        return;
    }
}

inline void setFailedSocketCallback(FailedSocketHandler socketHandler) {
    failedSocket = socketHandler;
}

inline void setFailedSetSockOptCallback(FailedSocketHandler setSockOptHandler) {
    failedSetSockOpt = setSockOptHandler;
}

inline void setFailedBindCallback(FailedBindHandler bindHandler) {
    failedBind = bindHandler;
}

inline void setFailedListenHandler(FailedListenHandler listenHandler) {
    failedListen = listenHandler;
}

struct Request {
    int client;

    static Request get() {
        return Request{.client=accept(sockfd, nullptr, nullptr)};
    }

    std::string readRequest() {
        int requestSize;
        ioctl(client, FIONREAD, &requestSize);
        char* buffer = new char[requestSize];
        ssize_t readSize = read(client, buffer, requestSize - 1);
        std::string result(buffer, readSize);
        delete[] buffer;
        return result;
    }

    std::string readRequest(size_t charAmount) {
        char* buffer = new char[charAmount];
        ssize_t readSize = read(client, buffer, charAmount - 1);
        buffer[charAmount - 1] = 0;
        std::string result(buffer, readSize);
        delete[] buffer;
        return result;
    }

    void respond(const std::string& message) {
        write(client, message.c_str(), message.size());
    }

    ~Request() {close(client);}
};

inline void closeServer() {
    close(sockfd);
}

#endif
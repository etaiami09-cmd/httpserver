#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>

inline int sockfd;

static inline int PORT = 8080;
static inline int MAX_CLIENTS = 10;

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

inline void setPort(int port) {
    PORT = port;
}

inline void setMaxClients(int maxClients) {
    MAX_CLIENTS = maxClients;
}

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

inline void setFailedSetSockOptCallback(FailedSetSockOptHandler setSockOptHandler) {
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
    std::string waiting;

private:
    Request(int clientFd) : client(clientFd) {}
public:

    static std::vector<Request>& getPending() {
        static std::vector<Request> pending;
        return pending;
    }

    static Request get() {
        auto& pending = getPending();
        pending.erase(
            std::remove_if(pending.begin(), pending.end(), [](Request& request) {
                char buffer;
                return recv(request.client, &buffer, 1, MSG_PEEK | MSG_DONTWAIT) == 0;
            }),
        pending.end()
        );
        if (!pending.empty()) {
            auto request = std::move(pending[pending.size() - 1]);
            pending.pop_back();
            return request;
        }
        return Request{accept(sockfd, nullptr, nullptr)};
    }

    static void feedBackRequest(Request& request) {
        getPending().push_back(std::move(request));
    }

    Request(Request&& other) {
        client = other.client;
        waiting = std::move(other.waiting);
        other.client = -1;
    }

    Request& operator=(Request&& other) {
        if (this != &other) {
            close(client);
            client = other.client;
            waiting = std::move(other.waiting);
            other.client = -1;
        }
        return *this;
    }

    std::string readRequest() {
        int requestSize;
        ioctl(client, FIONREAD, &requestSize);
        char* buffer = new char[requestSize];
        ssize_t readSize = read(client, buffer, requestSize - 1);
        std::string result = waiting + std::string(buffer, readSize);
        delete[] buffer;
        waiting.clear();
        return result;
    }

    std::string readRequest(size_t charAmount) {
        if (waiting.size() >= charAmount) {
            std::string result{waiting.begin(), waiting.begin() + charAmount};
            waiting = waiting.substr(charAmount);
            return result;
        }
        char* buffer = new char[charAmount - waiting.size()];
        ssize_t readSize = read(client, buffer, charAmount - waiting.size());
        std::string result = waiting + std::string(buffer, readSize);
        waiting.clear();
        delete[] buffer;
        return result;
    }

    std::string readUntil(const std::string& delimeter) {
        auto waitingDelim = waiting.find(delimeter);
        if (waitingDelim != std::string::npos) {
            std::string result = waiting.substr(0, waitingDelim + delimeter.size());
            waiting = waiting.substr(waitingDelim + delimeter.size());
            return result;
        }
        std::string result = std::move(waiting);
        char buffer[1024];
        while (result.find(delimeter) == std::string::npos) {
            ssize_t readSize = read(client, buffer, sizeof(buffer));
            if (readSize <= 0) return result;
            result.append(buffer, readSize);
        }
        auto pos = result.find(delimeter);
        waiting = result.substr(pos + delimeter.size());
        return result.substr(0, pos + delimeter.size());
    }

    void unread(const std::string& requestRemainder) {
        waiting = requestRemainder + waiting;
    }

    void respond(const std::string& message) {
        std::ignore = write(client, message.c_str(), message.size());
    }

    ~Request() {close(client);}
};

inline void closeServer() {
    close(sockfd);
}

#endif
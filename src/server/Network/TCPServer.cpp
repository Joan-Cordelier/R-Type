/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPServer
*/

#include "TCPServer.hpp"
#include "../Logs/Logger.hpp"

TCPServer::TCPServer(ThreadedQueue<DecodedMessage>& queue) : AServer(queue)
{
    init(AServer::protocol::TCP, 4789);
}

TCPServer::~TCPServer()
{
    reset();
}

int TCPServer::run()
{
    _fds.push_back({_serverFd, POLLIN, 0});
    MessageFactory& factory = MessageFactory::getInstance();

    while (_running) {
        if (poll(_fds.data(), _fds.size(), 100) < 0) {
            if (errno == EINTR)
                continue;
            LOG_ERROR("TCP poll failed");
            return 84;
        }
        
        processOutgoingQueue();
        
        std::vector<int> toDisconnect;
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                 toDisconnect.push_back(_fds[i].fd);
                 continue;
            }
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverFd) {
                    int newFd = accept(_serverFd, nullptr, nullptr);
                    if (newFd >= 0) {
                        LOG_INFO("TCP client connected (fd: " + std::to_string(newFd) + ")");
                        _fds.push_back({newFd, POLLIN, 0});
                        {
                            std::lock_guard<std::mutex> lock(_clientsMutex);
                            _clientFds.push_back(newFd);
                        }
                        _buffers.emplace(newFd, LinearBuffer());
                    } else
                        LOG_ERROR("TCP accept failed");
                } else {
                    char buffer[4096];
                    int n = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
                    
                    if (n <= 0) {
                        if (n < 0)
                            LOG_WARN("TCP recv failed (fd: " + std::to_string(_fds[i].fd) + ")");
                        toDisconnect.push_back(_fds[i].fd);
                    } else {
                        LOG_DEBUG("TCP received " + std::to_string(n) + " bytes (fd: " + std::to_string(_fds[i].fd) + ")");
                        if (!_buffers[_fds[i].fd].write(buffer, n)) {
                            toDisconnect.push_back(_fds[i].fd);
                            continue;
                        }
                        while (true) {
                            DecodedMessage msg = factory.decodeFromBuffer(_buffers[_fds[i].fd]);
                            if (msg.opCode == INCOMPLETE)
                                break;
                            if (msg.opCode == PARSING_ERROR) {
                                LOG_WARN("TCP parsing error (fd: " + std::to_string(_fds[i].fd) + ")");
                                toDisconnect.push_back(_fds[i].fd);
                                break;
                            }
                            LOG_INFO("TCP message received: OpCode=" + std::to_string(msg.opCode) + " Len=" + std::to_string(msg.len));
                            _queue.push(msg.priority, msg);
                        }
                    }
                }
            }
        }

        handleDisconnections(toDisconnect);        
        for (int fd : toDisconnect) {
            _buffers.erase(fd);
        }
    }
    return 0;
}

void TCPServer::send(const MessageData& data, Priority priority)
{
    if (data.empty())
        return;
    _outgoingQueue.push(priority, {-1, data});
}

void TCPServer::send(int fd, const MessageData& data, Priority priority)
{
    if (data.empty() || fd < 0)
        return;
    _outgoingQueue.push(priority, {fd, data});
}

void TCPServer::processOutgoingQueue()
{
    for (Priority p : {Priority::CRITICAL, Priority::HIGH, Priority::MEDIUM, Priority::LOW}) {
        while (true) {
            auto msgOpt = _outgoingQueue.pop(p);
            if (!msgOpt.has_value())
                break;
            
            OutgoingMessage& msg = msgOpt.value();
            if (msg.targetFd == -1) {
                sendToAll(msg.data);
            } else {
                sendToFd(msg.targetFd, msg.data);
            }
        }
    }
}

int TCPServer::sendToFd(int fd, const MessageData& data)
{
    if (data.empty() || fd < 0)
        return -1;
    
    std::lock_guard<std::mutex> lock(_clientsMutex);
    
    auto it = std::find(_clientFds.begin(), _clientFds.end(), fd);
    if (it == _clientFds.end()) {
        LOG_WARN("TCP sendToFd: invalid fd " + std::to_string(fd));
        return -1;
    }
    
    ssize_t sent = ::send(fd, data.data(), data.size(), MSG_NOSIGNAL);
    if (sent < 0) {
        LOG_ERROR("TCP send failed (fd: " + std::to_string(fd) + ")");
        return -1;
    }
    LOG_DEBUG("TCP sent " + std::to_string(data.size()) + " bytes to fd " + std::to_string(fd));
    return 0;
}

int TCPServer::sendToAll(const MessageData& data)
{
    if (data.empty())
        return -1;
    
    int result = 0;
    std::lock_guard<std::mutex> lock(_clientsMutex);
    LOG_DEBUG("TCP broadcasting " + std::to_string(data.size()) + " bytes to " + std::to_string(_clientFds.size()) + " clients");
    for (int fd : _clientFds) {
        ssize_t sent = ::send(fd, data.data(), data.size(), MSG_NOSIGNAL);
        if (sent < 0) {
            LOG_ERROR("TCP send failed (fd: " + std::to_string(fd) + ")");
            result = -1;
        }
    }
    return result;
}

#pragma once

#include <queue>
#include <thread>
#include <future>
#include <memory>
#include <functional>

#include "asio.hpp"

#include "coro_actor_types.h"
#include "coro_actor_reply.h"

using coro_actor_reply_callback_t = std::function<void(coro_actor_reply& reply)>;

class coro_actor_client {
public:
    coro_actor_client();

    ~coro_actor_client();

    void stop();

    coro_actor_connect_status connect(const std::string& host, uint16_t port);

    coro_actor_reply send(uint8_t type, const std::string& service, const std::string& data);

    void async_send(uint8_t type, const std::string& service, const std::string& data, const coro_actor_reply_callback_t& reply_callback);

    void async_send(uint8_t type, const std::string& service, const std::string& data, uint32_t timeout, const coro_actor_reply_callback_t& reply_callback);

private:
    struct coro_actor_request_t {
        uint8_t type;
        std::string service;
        uint16_t service_length;
        std::string data;
        uint32_t data_length;
        uint32_t timeout;
        coro_actor_reply_callback_t cb;
        std::shared_ptr<std::promise<coro_actor_reply>> pm;
    };

    void handle_request();

    void handle_response();

    void handle_failed_requests();

private:
    asio::io_context io_context_;
    asio::ip::tcp::socket socket_;
    asio::steady_timer timer_;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
    
    std::thread work_thread_;

    bool request_in_progress_;
    std::queue<coro_actor_request_t> request_queue_;
    coro_actor_request_t current_req_;

    uint8_t reply_type_;
    uint32_t reply_data_length_;
    std::string reply_data_;
};
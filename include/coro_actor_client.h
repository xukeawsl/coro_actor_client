#pragma once

#include <queue>
#include <thread>
#include <future>
#include <memory>
#include <functional>

#include "asio.hpp"

#include "coro_actor_types.h"
#include "coro_actor_reply.h"

/**
 * coro_actor_client 调用请求响应的回调处理函数
 */
using coro_actor_reply_callback_t = std::function<void(coro_actor_reply& reply)>;

/**
 * coro_actor_client 是用于向 coro_actor 服务端发起请求的客户端对象
 */
class coro_actor_client {
public:
    /**
     * 构造函数, 会开启一个线程等待处理请求
     */
    coro_actor_client();

    /**
     * 析构函数, 会停止整个客户端的操作并等待线程结束
     */
    ~coro_actor_client();

    /**
     * 主动停止客户端的所有操作
     */
    void stop();

    /**
     * 同步连接 coro_actor 服务节点
     * 
     * @param host 主机地址, 可以是 ip 地址也可以是域名
     * @param port 主机端口
     * 
     * @return 连接状态
     */
    coro_actor_connect_status connect(const std::string& host, uint16_t port);

    /**
     * 同步发起服务调用
     * 
     * @param type 请求报文类型
     * @param service 请求的服务名
     * @param data 请求的报文内容
     * 
     * @return 响应报文
     */
    coro_actor_reply send(uint8_t type, const std::string& service, const std::string& data);

    /**
     * 异步发起服务调用
     * 
     * @param type 请求报文类型
     * @param service 请求的服务名
     * @param data 请求的报文内容
     * @param reply_callback 处理响应报文的回调函数
     */
    void async_send(uint8_t type, const std::string& service, const std::string& data, const coro_actor_reply_callback_t& reply_callback);

    /**
     * 异步发起服务调用, 指定一个超时等待时间
     * 
     * @param type 请求报文类型
     * @param service 请求的服务名
     * @param data 请求的报文内容
     * @param timeout 请求的超时时间
     * @param reply_callback 处理响应报文的回调函数
     */
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
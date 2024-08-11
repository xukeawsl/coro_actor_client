#include <iostream>

#include "coro_actor_client.h"

int main() {
    coro_actor_client client;

    auto status = client.connect("127.0.0.1", 7777);
    if (status == coro_actor_connect_status::ok) {
        std::cout << "successful connect" << std::endl;
    } else {
        std::cout << "failed to connect" << std::endl;
        return 1;
    }

    client.async_send(ACTOR_SERVICE, "svc_echo", "async_call", [](coro_actor_reply& reply) {
        if (reply.status() == coro_actor_reply::status::success) {
            std::cout << "reply: " << reply.data() << std::endl;
        }
    });

    client.async_send(ACTOR_SERVICE, "svc_echo", "test timeout", 500, [](coro_actor_reply& reply) {
        if (reply.status() == coro_actor_reply::status::success) {
            std::cout << "reply: " << reply.data() << std::endl;
        } else {
            std::cout << "failed to async_send" << std::endl;
        }
    });

    auto reply = client.send(ACTOR_SERVICE, "svc_echo", "hello world");
    if (reply.status() == coro_actor_reply::status::success) {
        std::cout << "reply: " << reply.data() << std::endl;
    } else {
        std::cout << "failed to send" << std::endl;
    }
    
    return 0;
}
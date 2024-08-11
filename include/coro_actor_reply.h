#pragma once

#include <string>

class coro_actor_reply {
public:

    enum class status {
        success,
        failed,
        moved,
        none
    };

public:

    coro_actor_reply() : status_(status::none) {}

    coro_actor_reply(coro_actor_reply::status status, uint8_t type, const std::string& data)
    : status_(status), type_(type), data_(data) {}

    void set_status(coro_actor_reply::status status) { status_ = status; }

    void set_type(uint8_t type) { type_ = type; }

    void set_data(const std::string& data) { data_ = data; }

    void set_data(std::string&& data) { data_ = std::move(data); }

    coro_actor_reply::status status() const { return status_; }

    uint8_t type() const { return type_; }

    std::string data() const { return data_; }

    std::string move() {
        status_ = status::moved;
        return std::move(data_);
    }

private:
    enum status status_;

    uint8_t type_;

    std::string data_;
};
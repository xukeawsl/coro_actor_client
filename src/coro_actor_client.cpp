#include "coro_actor_client.h"

coro_actor_client::coro_actor_client() :
    io_context_(1), socket_(io_context_), timer_(io_context_),
    work_guard_(asio::make_work_guard(io_context_)), request_in_progress_(false)
{
    timer_.expires_at(std::chrono::steady_clock::time_point::max());

    work_thread_ = std::thread([this] {
        this->io_context_.run();
    });
}

coro_actor_client::~coro_actor_client() {
    io_context_.stop();
    work_thread_.join();
}

void coro_actor_client::stop() {
    handle_failed_requests();
    asio::error_code ec;
    socket_.close(ec);
    timer_.cancel(ec);
    io_context_.stop();
}

coro_actor_connect_status coro_actor_client::connect(const std::string& host, uint16_t port) {
    if (socket_.is_open()) {
        return coro_actor_connect_status::already_connected;
    }

    asio::error_code ec;
    asio::ip::tcp::resolver resolver(io_context_);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port), ec);
    if (ec) {
        return coro_actor_connect_status::failed;
    }

    asio::connect(socket_, endpoints, ec);
    if (ec) {
        return coro_actor_connect_status::failed;
    }

    return coro_actor_connect_status::ok;
}

coro_actor_reply coro_actor_client::send(uint8_t type, const std::string& service, const std::string& data) {
    auto pm = std::make_shared<std::promise<coro_actor_reply>>();
    auto future = pm->get_future();

    asio::post(io_context_, [this, type, service, data, pm] {
        coro_actor_request_t req;
        req.type = type;
        req.service = service;
        req.data = data;
        req.timeout = UINT32_MAX;
        req.pm = pm;

        this->request_queue_.emplace(std::move(req));

        if (this->request_in_progress_ == false) {
            this->handle_request();
        }
    });

    auto reply = future.get();

    return reply;
}

void coro_actor_client::async_send(uint8_t type, const std::string& service, const std::string& data, const coro_actor_reply_callback_t& reply_callback) {
    async_send(type, service, data, UINT32_MAX, reply_callback);
}

void coro_actor_client::async_send(uint8_t type, const std::string& service, const std::string& data, uint32_t timeout, const coro_actor_reply_callback_t& reply_callback) {
    asio::post(io_context_, [this, type, service, data, timeout, reply_callback] {
        coro_actor_request_t req;
        req.type = type;
        req.service = service;
        req.data = data;
        req.timeout = timeout;
        req.cb = reply_callback;

        this->request_queue_.emplace(std::move(req));

        this->handle_request();
    });
}

void coro_actor_client::handle_request() {
    if (request_queue_.empty() || request_in_progress_ == true) {
        return;
    }

    request_in_progress_ = true;
    current_req_ = request_queue_.front();

    current_req_.service_length = asio::detail::socket_ops::host_to_network_short(current_req_.service.length());
    current_req_.data_length = asio::detail::socket_ops::host_to_network_long(current_req_.data.length());

    
    timer_.expires_after(std::chrono::milliseconds(current_req_.timeout));
    
    timer_.async_wait([this](asio::error_code ec) {
        if (!ec) {
            this->stop();
        }
    });

    std::array<asio::const_buffer, 5> write_buf = {
        { asio::buffer(&current_req_.service_length, 2), asio::buffer(current_req_.service.data(), current_req_.service.length()),
          asio::buffer(&current_req_.type, 1), asio::buffer(&current_req_.data_length, 4),
          asio::buffer(current_req_.data.data(), current_req_.data.length())}
    };

    asio::async_write(socket_, write_buf, [this](asio::error_code ec, std::size_t) {
        if (ec) {
            this->stop();
            return;
        }

        this->handle_response();
    });
}

void coro_actor_client::handle_response() {
    std::array<asio::mutable_buffer, 2> read_buf = {
        { asio::buffer(&reply_type_, 1), asio::buffer(&reply_data_length_, 4) }
    };

    asio::async_read(socket_, read_buf, [this](asio::error_code ec, std::size_t) {
        if (ec) {
            this->stop();
            return;
        }

        this->reply_data_length_ = asio::detail::socket_ops::network_to_host_long(this->reply_data_length_);
        this->reply_data_.resize(this->reply_data_length_);

        asio::async_read(this->socket_, asio::buffer(this->reply_data_.data(), this->reply_data_.length()),
            [this](asio::error_code ec, std::size_t) {
                if (ec) {
                    this->stop();
                    return;
                }

                this->request_queue_.pop();

                timer_.cancel_one(ec);

                coro_actor_reply reply(coro_actor_reply::status::success, this->reply_type_, this->reply_data_);
                if (current_req_.cb) {
                    current_req_.cb(reply);
                } else {
                    current_req_.pm->set_value(std::move(reply));
                }

                this->request_in_progress_ = false;

                if (!this->request_queue_.empty()) {
                    this->handle_request();
                }
            }
        );
    });
}

void coro_actor_client::handle_failed_requests() {
    while (!request_queue_.empty()) {
        auto req = request_queue_.front();
        request_queue_.pop();

        coro_actor_reply reply;
        reply.set_status(coro_actor_reply::status::failed);

        if (req.cb) {
            req.cb(reply);
        } else {
            req.pm->set_value(reply);
        }
    }
}
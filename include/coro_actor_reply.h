#pragma once

#include <string>

/**
 * @brief 对客户端调用请求响应的封装
 */
class coro_actor_reply {
public:
    /**
     * @brief 响应的状态
     */
    enum class status {
        success,    /**< 获取响应成功 */
        failed,     /**< 获取响应失败 */
        moved,      /**< 响应报文内容被转移 */
        none        /**< 响应默认状态 */
    };

public:
    /**
     * @brief 默认构造函数, 状态置为 none
     */
    coro_actor_reply() : status_(status::none) {}

    /**
     * @brief 默认析构函数
     */
    ~coro_actor_reply() = default;

    /**
     * @brief 指定内容构造函数
     * 
     * @param status 响应状态
     * @param type 响应报文类型
     * @param data 响应报文内容
     */
    coro_actor_reply(coro_actor_reply::status status, uint8_t type, const std::string& data)
    : status_(status), type_(type), data_(data) {}

    /**
     * @brief 设置响应状态
     * 
     * @param status 响应状态
     */
    void set_status(coro_actor_reply::status status) { status_ = status; }

    /**
     * @brief 设置响应报文类型
     * 
     * @param type 响应报文类型
     */
    void set_type(uint8_t type) { type_ = type; }

    /**
     * @brief 设置响应报文内容
     * 
     * @param data 响应报文内容
     */
    void set_data(const std::string& data) { data_ = data; }

    /**
     * @brief 通过移动的方式设置报文内容
     * 
     * @param data 响应报文内容
     */
    void set_data(std::string&& data) { data_ = std::move(data); }

    /**
     * @return 响应报文状态
     */
    coro_actor_reply::status status() const { return status_; }

    /**
     * @return 响应报文类型
     */
    uint8_t type() const { return type_; }

    /**
     * @return 响应报文内容的拷贝
     */
    std::string data() const { return data_; }

    /**
     * @return 移动响应报文内容
     */
    std::string move() {
        status_ = status::moved;
        return std::move(data_);
    }

private:
    enum status status_;

    uint8_t type_;

    std::string data_;
};
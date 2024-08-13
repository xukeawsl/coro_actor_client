#pragma once

/**
 * 默认的一个报文类型, 用户可以自行扩展
 */
#define ACTOR_SERVICE 0

/**
 * coro_actor_client 对象的连接状态
 */
enum class coro_actor_connect_status {
    already_connected,  /**< 已连接 */
    failed,             /**< 连接失败 */
    ok                  /**< 连接成功 */
};
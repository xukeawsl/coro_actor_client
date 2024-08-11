#pragma once

#define ACTOR_SERVICE 0

enum class coro_actor_connect_status {
    already_connected,
    failed,
    ok
};
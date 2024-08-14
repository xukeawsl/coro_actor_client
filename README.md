# coro_actor_client

提供客户端调用 `coro_actor` 框架服务的接口, 支持 Windows 和 Linux, 编译器只需要支持 C++11 即可

# 使用

## 安装

```bash
git clone https://github.com/xukeawsl/coro_actor_client.git
cd coro_actor_client
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cmake --install .
```

## 使用 CMake 作为第三方库引入

* 假如子模块存放在 `third-party` 目录下, 则在 `CMakeLists.txt` 添加即可

```cmake
add_subdirectory(third-party/coro_actor_client)
```

## 编译链接

* 以 `example` 中的代码为例

```cmake
add_executable(echo echo.cpp)

target_link_libraries(echo PUBLIC coro_actor_client)
```

## 编译测试代码

* 在 cmake 构建时设置 `ENABLE_TEST` 选项即可

```bash
cmake -DENABLE_TEST=On ..
```

## 文档

* 使用 Doxygen 生成, 可以在 `docs` 目录下的 `html` 目录查看接口文档
# README

uthread is a lightweight fiber/user threading librar. Some of the current supported features include...

- Cooperative scheduling with fast context switching (~30M thread context switches/second on a single core)
- Synchronization primitives like mutexes, condition variables, etc. for use between user threads
- Asynchronous IO notifications, courtesy of libevent

Some missing features which may come in the future...

- Guard pages for stack overflow protection

## Design

uthread is designed around an Executor which executes and switches among Threads. See [executor.hpp](include/uthread/executor.hpp) for more detailed info. The `Executor::sleep(...)` and `Executor::ready(...)` functions are especially important for being the basis of the provided synchronization primitives and IO notification system.

Once you have a high level understanding of the executor take a look at the [Mutex](src/mutex.cpp) as an example of using the `Executor::sleep(...)` and `Executor::ready(...)` functions. The [IO system](src/io.cpp) works in a similar fassion and relies on [libevent](http://libevent.org/) as a backend. You can read the [libevent online book](http://www.wangafu.net/~nickm/libevent-book) to learn more.

## Examples

- [tests/executor.cpp](tests/executor.cpp) for examples of spawning, executing, and switching between user threads
- [tests/mutex.cpp](tests/mutex.cpp) and [tests/condition_variable.cpp](tests/condition_variable.cpp) for examples of using the provided synchronization primitives
- [tests/io.cpp](tests/io.cpp) for an example of IO notifications

## Dependencies

- [CMake](https://cmake.org/)
- [glog](https://github.com/google/glog)
- [googletest](https://github.com/google/googletest)
- [benchmark](https://github.com/google/benchmark)
- [libevent](http://libevent.org/)

## Building

A [Vagrant](https://www.vagrantup.com/) VM is provided with a complete development environment. You should first bootup the VM from the [vagrant](vagrant) directory...

```
vagrant up && vagrant ssh
```

Once the VM you can perform a build...

```
cd /uthread                           && \
./cmake.sh -DCMAKE_BUILD_TYPE=Release && \
cd build                              && \
make -j8                              && \
make tests                            && \
make bench
```

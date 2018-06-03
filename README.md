# README

uthread is a lightweight fiber/user threading library. Some of the current supported features include...

- Cooperative scheduling with fast context switching (~30M thread context switches/second on a single core)
- Synchronization primitives like mutexes, condition variables, etc. for use between user threads
- Asynchronous IO notifications for timers, sockets, etc. courtesy of libevent

Some missing features which may come in the future...

- Guard pages for stack overflow protection

## Design

uthread is designed around an N:1 (user-thread:kernel-thread) mapping model. An executor is responsible for multiplexing a set of user threads on top of a kernel thread. See [executor.hpp](include/uthread/executor.hpp) for more detailed info. The `Executor::sleep(...)` and `Executor::ready(...)` functions are especially important for being the basis of the provided synchronization primitives and IO notification system.

Once you have a high level understanding of the executor take a look at the [Mutex](src/mutex.cpp) as an example of using the `Executor::sleep(...)` and `Executor::ready(...)` functions. The [IO system](src/io.cpp) works in a similar fassion and relies on [libevent](http://libevent.org/) as a backend. You can read the [libevent online book](http://www.wangafu.net/~nickm/libevent-book) to learn more.

## Examples

- [tests/executor.cpp](tests/executor.cpp) for examples of spawning, executing, and switching between user threads
- [tests/mutex.cpp](tests/mutex.cpp) and [tests/condition_variable.cpp](tests/condition_variable.cpp) for examples of using the provided synchronization primitives
- [tests/io.cpp](tests/io.cpp) for an example of IO notifications
- [examples/echo.cpp](examples/echo.cpp) for an example of a TCP echo server
- [examples/nc.cpp](examples/nc.cpp) for a bare-bones nc clone

You can run example `<X>` via `GLOG_logtostderr=1 ./<X>` after building.

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


## Dependencies

- [gflags](https://github.com/gflags/gflags)
- [glog](https://github.com/google/glog)
- [googletest](https://github.com/google/googletest)
- [benchmark](https://github.com/google/benchmark)
- [libevent](http://libevent.org/)

## Reading

The following are some good links regarding memory models, compiler/hardware instruction reordering, etc that are important to understand in concurrent environments. uthread multiplexes user threads on top of a single kernel thread so most of this is not immediately relevant but useful to understanding what kind of compiler optimizations might occur around context switches. Despite **thinking** I understand this stuff, I wouldn't be surprised if uthreads has some bugs with regards to these ideas :)

- [http://preshing.com/20120625/memory-ordering-at-compile-time/](http://preshing.com/20120625/memory-ordering-at-compile-time/)
- [http://preshing.com/20120515/memory-reordering-caught-in-the-act/](http://preshing.com/20120515/memory-reordering-caught-in-the-act/)
- [http://preshing.com/20120913/acquire-and-release-semantics/](http://preshing.com/20120913/acquire-and-release-semantics/)
- [https://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync](https://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync)
- [https://stackoverflow.com/questions/14182066/is-memory-reordering-visible-to-other-threads-on-a-uniprocessor](https://stackoverflow.com/questions/14182066/is-memory-reordering-visible-to-other-threads-on-a-uniprocessor)

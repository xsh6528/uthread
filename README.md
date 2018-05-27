# README

This is a simple C++ 11 starter project with unit tests, benchmarks, etc. using CMake for builds.

## Dependencies

- [CMake](https://cmake.org/)
- [glog](https://github.com/google/glog)
- [googletest](https://github.com/google/googletest)
- [benchmark](https://github.com/google/benchmark)

## Building

The basic steps for building the project look like this.

```
./cmake.sh
cd build
make -j8
make lint
make tests
make bench
```
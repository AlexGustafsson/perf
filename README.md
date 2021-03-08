<p align="center">
  <strong><a href="#quickstart">Quick Start</a> | <a href="#contribute">Contribute</a> </strong>
</p>

# perf
## A library for using the perf API on Linux

_Note: perf is under active development and may see breaking changes._

<a id="quickstart"></a>
## Quick Start

This project holds a library for the [Linux perf event APIs](https://perf.wiki.kernel.org/index.php/Main_Page). At its core lies an implementation of the `perf_event_open (2)` syscall, usable as is.

The library also contains several utilities necessary for stable usage of the APIs. The utilities provide error handling, support validation and more.

The focus of this library is highly accurate results without any overhead _during measuring_. That is, setting up measurements may be costly, but performing measurements requires zero overhead added by this library. This comes with a great benefit of achieving as accurate results as possible and being as flexible as possible, at the cost of UX.

To build the project, first install `libcap`:

* Ubuntu: `sudo apt install libcap-dev`
* RHEL: `sudo yum install libcap-devel`
* SUSE: `sudo zapper install libcap-devel`

Then, simply build the project.

```sh
make build
```

Examples are available in the `examples` directory, output to the `build/examples` directory.

```
./build/examples/full
```

## Table of contents

[Quickstart](#quickstart)<br/>
[Features](#features)<br />
[Documention](#documentation)<br />
[Contribute](#contribute)

## Features

* Easily built on modern Linux
* Zero-overhead measurements
* Tested on x86 Ubuntu LTS 20.04 (Linux 5.7 and 5.8)
* Tested on s390x RHEL 8.3 (Linux 4.18)
* Supports Linux 2.6.32 and newer
* Supports graceful handling of insufficient capabilities per monitored event (and `CAP_PERFMON` added in 5.9)

<a id="documentation"></a>
## Documentation

### Goals

There are many profiling tools available for Linux. Tools such as Valgrind (and Callgrind) are great and accurate ways of performing performance analysis of memory and the callstack. It does, however, produce simplified measurements for values such as CPU cycles and instructions since it doesn't measure native hardware. Other tools such as gprof, prof and gperftools are much closes to the hardware, but lack support for measuring kernel and idle events. Other ways of measuring instructions and cycles used in SUPERCOP, Papi etc. can be inaccurate, simply counting the overall cycles performed by a CPU core. This is where the Linux perf APIs shine. The APIs provide a comprehensive set of tools for measuring user and kernel events - be it via software defined breakpoints or hardware counters.

The `perf` CLI tool is great at providing a way of using these APIs, but it measures the execution of an entire program. The underlying system call, `perf_event_open (2)`, has no glibc wrapper and is therefore quite verbose - especially when performing a lot of measurements.

This library aims to fill this small niche gap where highly accurate measurements of specific code in a binary.

### Usage

To be continued as the API stabilizes.

<a id="contribute"></a>
## Contributing

Any contribution is welcome. If you're not able to code it yourself, perhaps someone else is - so post an issue if there's anything on your mind.

## Development

Make sure you meet the following prerequisites:
* `$CC` refers to `gcc` 8 or newer
* `libcap` is installed
* `clang-format` is installed (if you want to format code)
* `compiledb` is installed (if you want format code or analyze it with other llvm tools)

```shell
# Clone the repository
git clone https://github.com/AlexGustafsson/perf && cd perf

# Format the code
make format

# Build the library
make library

# Build examples
make examples
```

### Roadmap

* [ ] Add support for `mmap`ed events
* [x] Add support for monitoring groups
* [ ] Add further, real world examples

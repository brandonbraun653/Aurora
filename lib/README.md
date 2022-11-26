This directory contains a smattering of seemingly unrelated 3rd party libraries. The goal here is to
extend the functionality of Aurora by building on top of the open source community. Each library has
a specific purpose to the functionality of an embedded systems project. Hopefully by collecting these
libraries in one location will help make it easier to rapidly build complex systems.

CRCpp: https://github.com/d-bahr/CRCpp
  Software routines for calculating CRCs

ctre: https://github.com/hanickadot/compile-time-regular-expressions
  Compile Time Regular Expressions. Very handy for computing regex on strings in a memory efficient
  manner. This has a few compiler restrictions, so be sure to check out the wiki.

etl: https://github.com/ETLCPP/etl
  Embedded template library. Replaces a lot of the STL functionality with versions that focus on
  using statically allocated memory (no calls to new). This is very handy in embedded systems.

littlefs: https://github.com/littlefs-project/littlefs
  Embedded filesystem for NOR flash devices. Works rather well!

nanoprintf: https://github.com/charlesnicholson/nanoprintf
  A far more efficient (and restricted) printf implementation for embedded applications

nanopb: https://github.com/nanopb/nanopb
  ANSI C implementation of Protocol Buffers targeted for micro controllers

segger: https://www.segger.com/downloads/systemview/
  Target source code for SystemView instrumentation on a project. Handy when you need to trace what
  your system is doing in real time without interrupting the CPU's execution.

spiffs: https://github.com/pellepl/spiffs
  Old filesystem. Deprecated. Honestly needs removal.

type_safe: https://github.com/foonathan/type_safe
  C++ support for more strongly typed data, based on feature sets. Useful for preventing bugs due to
  unexpected or hidden type behavior.

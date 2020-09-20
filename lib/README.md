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

type_safe: https://github.com/foonathan/type_safe
  C++ support for more strongly typed data, based on feature sets. Useful for preventing bugs due to
  unexpected or hidden type behavior.

uLog: https://github.com/brandonbraun653/uLog
  A very lightweight logger for embedded systems. Very lightweight. It's very minimalistic.

yaffs: https://yaffs.net/get-yaffs
  This one is cool. It's "Yet Another Flash File System" but one that's rated for space travel. Several
  satellites and other star fairing vessels have this running on their systems. Should be good enough
  for the general purpose embedded world too.
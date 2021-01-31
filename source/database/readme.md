# Database
A very low level and stripped down data storage system for the purpose of embedded systems. The goal
is to provide read/write/verify capabilities for storing runtime registerable data with the intent
of it serving as "ground truth" for system level parameters.

Rather than have a single "extern" variable that is passed around the system via header file inclusion,
the database allows the user to look up the latest data via a key, performing a CRC check along the way
to ensure the data is still the same as when it was last written to.

The database is also highly flexible in that it can accept data of any type or size, pending the
underlying system having enough RAM to support it. Memory is managed in it's own statically allocated
heap, so it's safe from corrupting the system memory.

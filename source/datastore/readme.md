# Datastore
Aurora's datastore is a management layer on top of a database that adds useful functionality such as
data change event notifications, validity timeouts, and much more. It's goal is to provide a centralized
repository of system data that should be observed, validated, and acted upon in multiple product functionalities.

A great example of this is sensor data from an AHRS system in a quadrotor. Multiple threads may want to know
the latest data, perhaps for control system purposes or maybe logging. The control system depends on the data
being current and valid, while the logger simply wants to know what it should write to memory. In both cases,
the data must be sourced from a single location in a thread safe manner, validated, and then notify those who
care about it's changed state.

The datastore manager seeks to simply the effort required to implement this level of communication and validation
of datasets across an entire system.
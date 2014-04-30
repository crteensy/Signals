Signals - forwarding branch
===========================

This branch intended for work on the forwarding of Signal, Connection and Delegate function call operator arguments.
Stack overflow:

- http://codereview.stackexchange.com/q/47098/37034
- http://stackoverflow.com/q/23323547/692359
- http://codereview.stackexchange.com/q/48447/37034

Signal and Connection classes for use in embedded systems.

These classes provide a simple implementation of the Observer Pattern, i.e. it is possible to have an object emit a signals that is observed by other objects. The objects are usually not aware of each other, so this is quite useful in distributed systems.

Originally intended for Teensy 3.0 and 3.1 platforms, see http://pjrc.com/teensy/index.html
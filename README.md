Signals
=======

Signal and Connection classes for use in embedded systems.

These classes provide a simple implementation of the observer pattern, i.e. it is possible to have an object emit a signal that is observed by other objects. The objects don't need to be aware of each other: you can, for example, put a signal into a GPS library and create a connection that sends position information to a user interface.

Originally intended for Teensy 3.x and 3.1 platforms, see http://pjrc.com/teensy/index.html, but these classes are pretty much platform independent.

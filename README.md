# jasio

jasio is a minimalist asynchronous-io library with no dependencies. 
It gives the neccessary building blocks to build a single-threaded edge-triggered
concurrent server, though with few additions it can be made to be used in a 
multi-threaded environment.


## Documentation

There are two major types is jasio, `struct jasio_continuation` and `struct jasio`.
`struct jasio` allows you to register a callback to a file descriptor whenever
it is ready and nonblocking for the given event type you specify. The callback
function must have the function signature 
```void (*)(int fd, void *data, enum jasio_events events)```,
where `fd` is the ready file descriptor, `data` is the user defined hook 
that allows you to put your data in the function, and `events` are the 
events that are ready and non-blocking on the file descriptor. `struct jasio_coninuation`
is the means by which you can add the callback, for example:
```c
void foo(struct jasio *jasio, int fd) {
        struct jasio_continuation continuation;
        continuation.func = &my_callback;
        continuation.data = my_data;
        jasio_add(&jasio, fd, JASIO_IN, continuation);
        // jasio will call mycallback(fd, my_data, events) when fd is nonblocking,
        // and events are whatever events are non-blocking at the time
}
```
### Note:
#### 1) events may contain JASIO_ERR, in which case the specific error can be found in errno.
#### 2) jasio does not take ownership of data given to it, users must free their data after use.

look in headers for more documentation.
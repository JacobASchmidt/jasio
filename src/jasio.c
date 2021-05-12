
/*
MIT License

Copyright (c) 2021 JacobASchmidt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "jasio.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>

void jasio_create(struct jasio *jasio, int cap)
{
	jasio_fdmap_create(&jasio->fdmap, cap);
	jasio->poll_fd = epoll_create1(0);
}

int jasio_add(struct jasio *jasio, int fd, enum jasio_events events,
	      struct jasio_continuation continuation)
{
	struct epoll_event e;
	e.data.fd = fd;
	e.events = (int)events;
	jasio_fdmap_add(&jasio->fdmap, fd, continuation);

	return epoll_ctl(jasio->poll_fd, EPOLL_CTL_ADD, fd, &e);
}

int jasio_modify_events(struct jasio *jasio, int fd, enum jasio_events events)
{
	struct epoll_event e;
	e.data.fd = fd;
	e.events = (int)events;
	return epoll_ctl(jasio->poll_fd, EPOLL_CTL_MOD, fd, &e);
}
void jasio_modify_continuation(struct jasio *jasio, int fd,
			       struct jasio_continuation continuation)
{
	jasio_fdmap_set(&jasio->fdmap, fd, continuation);
}

int jasio_remove(struct jasio *jasio, int fd)
{
	return epoll_ctl(jasio->poll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void jasio_run(struct jasio *jasio, int timeout)
{
	struct epoll_event *events = NULL;
	int cap = 0;

	struct jasio_continuation continuation;
	for (;;) {
		if (jasio->fdmap.cap > cap) {
			cap = jasio->fdmap.cap;
			events = realloc(events,
					 cap * sizeof(struct epoll_event));
		}
		int n = epoll_wait(jasio->poll_fd, events, cap, timeout);
		if (n < 0) {
			assert(errno == EAGAIN || errno == EINTR);
			continue;
		}
		for (int i = 0; i < n; ++i) {
			int fd = events[i].data.fd;
			continuation = jasio_fdmap_get(&jasio->fdmap, fd);
			continuation.func(fd, continuation.data,
					  (enum jasio_events)events[i].events);
		}
	}
}

#ifndef _CHAOS_LINUX_H_
#define _CHAOS_LINUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/chaos"
int fd;

void PlatformStart() {
	mknod(FIFO_NAME, S_IFIFO | 0666, 0);
	fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
}

bool PlatformReadBytes(size_t num, std::vector<uint8_t>* buf) {
	size_t actual;
	if ((actual = read(fd, buf->data(), num)) != -1) {
		return actual == num; 
	}
	return false;
}

void PlatformStop() {
    close(fd);
}

#endif
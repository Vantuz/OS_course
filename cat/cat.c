#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 228

int print_stdout(int fd);

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		return print_stdout(STDIN_FILENO);
	} else {
		for(int i = 1; i < argc; i++) {
			int fd = open(argv[i], O_RDONLY);
			if (fd < 0) {
				fprintf(stderr, "No such file: %s\n", argv[i]);
			} else {		
				int tmp = print_stdout(fd);
				if (tmp != 0) {
					return tmp;
				}
			}
		}	
	}
	return 0;
}

int print_stdout(int fd) {
	char buf[BUF_SIZE];
	int count_read = 228;
	while(count_read > 0) {
		int tmp = read(fd, buf, BUF_SIZE);;
		while(tmp < 0 && errno == EINTR) {
			tmp = read(fd, buf, BUF_SIZE);
		}
		if (tmp < 0) {
			return -1;
		}
		count_read = tmp;
		int count_write = 0;
		while(count_write < count_read) {
			tmp = write(STDOUT_FILENO, buf + count_write, count_read - count_write);
			while(tmp < 0 && errno == EINTR) {
				tmp = write(STDOUT_FILENO, buf + count_write, count_read - count_write);
			}
			if (tmp < 0) {
				return -1;
			}
			count_write += tmp;
		}
	}
	return 0;
}

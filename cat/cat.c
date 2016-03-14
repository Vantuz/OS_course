#include <unistd.h>

#define BUF_SIZE 228

int main() {
	char buf[BUF_SIZE];
	int count_read = 228;
	while(count_read != 0) {
		count_read = read(STDIN_FILENO, buf, BUF_SIZE);
		int count_write = 0;
		while(count_write < count_read) {
			count_write += write(STDOUT_FILENO, buf + count_write, count_read - count_write);
		}
	}
	return 0;
}
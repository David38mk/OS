// drivers.md §7 — user-space client: write a string to /dev/primer1, read it back until 0
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	const char *msg = argc > 1 ? argv[1] : "zdravo";
	char buf[16];                      /* small on purpose: forces several reads */
	ssize_t n;
	int fd = open("/dev/primer1", O_RDWR);

	if (fd < 0) {
		perror("open /dev/primer1");
		return 1;
	}
	n = write(fd, msg, strlen(msg));
	printf("write returned %zd\n", n);
	close(fd);                         /* reopen: offp-style drivers restart at 0 */
	fd = open("/dev/primer1", O_RDONLY);
	if (fd < 0) {
		perror("reopen");
		return 1;
	}
	while ((n = read(fd, buf, sizeof buf - 1)) > 0) {
		buf[n] = '\0';
		printf("read %zd: \"%s\"\n", n, buf);
	}
	printf("read returned %zd (0 = end of file)\n", n);
	close(fd);
	return 0;
}

// SPDX-License-Identifier: MIT
/* Minimal current-boot UI automation for liuqin's 1800x2880 GNOME session. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define LIUQIN_X_MAX 1799
#define LIUQIN_Y_MAX 2879

static void sleep_ms(unsigned int ms)
{
	struct timespec ts = { .tv_sec = ms / 1000,
			       .tv_nsec = (long)(ms % 1000) * 1000000L };

	while (nanosleep(&ts, &ts) && errno == EINTR)
		;
}

static int emit(int fd, unsigned short type, unsigned short code, int value)
{
	struct input_event event = { .type = type, .code = code, .value = value };

	return write(fd, &event, sizeof(event)) == sizeof(event) ? 0 : -1;
}

static int setup_abs(int fd, unsigned short code, int maximum)
{
	struct uinput_abs_setup setup = {
		.code = code,
		.absinfo = { .minimum = 0, .maximum = maximum },
	};

	return ioctl(fd, UI_ABS_SETUP, &setup);
}

static int create_device(int keycode)
{
	struct uinput_setup setup = {
		.id = { .bustype = BUS_VIRTUAL, .vendor = 0x1d6b,
			.product = 0x6c71, .version = 1 },
	};
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);

	if (fd < 0)
		return -1;
	strncpy(setup.name, "liuqin automation touchscreen", UINPUT_MAX_NAME_SIZE - 1);
	if (ioctl(fd, UI_SET_EVBIT, EV_SYN) || ioctl(fd, UI_SET_EVBIT, EV_KEY) ||
	    ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH) ||
	    (keycode >= 0 && ioctl(fd, UI_SET_KEYBIT, keycode)) ||
	    ioctl(fd, UI_SET_EVBIT, EV_ABS) ||
	    setup_abs(fd, ABS_X, LIUQIN_X_MAX) || setup_abs(fd, ABS_Y, LIUQIN_Y_MAX) ||
	    setup_abs(fd, ABS_MT_SLOT, 0) || setup_abs(fd, ABS_MT_TRACKING_ID, 65535) ||
	    setup_abs(fd, ABS_MT_POSITION_X, LIUQIN_X_MAX) ||
	    setup_abs(fd, ABS_MT_POSITION_Y, LIUQIN_Y_MAX) ||
	    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT) || ioctl(fd, UI_DEV_SETUP, &setup) ||
	    ioctl(fd, UI_DEV_CREATE)) {
		close(fd);
		return -1;
	}
	sleep_ms(250);
	return fd;
}

static int point(int fd, int tracking, int x, int y, int down)
{
	return emit(fd, EV_ABS, ABS_MT_SLOT, 0) ||
		emit(fd, EV_ABS, ABS_MT_TRACKING_ID, tracking) ||
		(tracking >= 0 && (emit(fd, EV_ABS, ABS_MT_POSITION_X, x) ||
				   emit(fd, EV_ABS, ABS_MT_POSITION_Y, y) ||
				   emit(fd, EV_ABS, ABS_X, x) || emit(fd, EV_ABS, ABS_Y, y))) ||
		emit(fd, EV_KEY, BTN_TOUCH, down) || emit(fd, EV_SYN, SYN_REPORT, 0);
}

static int number(const char *text, int maximum)
{
	char *end;
	long value = strtol(text, &end, 10);

	if (!*text || *end || value < 0 || value > maximum)
		return -1;
	return (int)value;
}

int main(int argc, char **argv)
{
	int fd, rc = 1;

	if (argc == 4 && !strcmp(argv[1], "tap")) {
		int x = number(argv[2], LIUQIN_X_MAX), y = number(argv[3], LIUQIN_Y_MAX);
		if (x < 0 || y < 0 || (fd = create_device(-1)) < 0)
			goto out;
		rc = point(fd, 1, x, y, 1) || (sleep_ms(80), point(fd, -1, x, y, 0));
	} else if (argc == 7 && !strcmp(argv[1], "swipe")) {
		int x1 = number(argv[2], LIUQIN_X_MAX), y1 = number(argv[3], LIUQIN_Y_MAX);
		int x2 = number(argv[4], LIUQIN_X_MAX), y2 = number(argv[5], LIUQIN_Y_MAX);
		int duration = number(argv[6], 10000), step;
		if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || duration < 50 ||
		    (fd = create_device(-1)) < 0 || point(fd, 1, x1, y1, 1))
			goto out;
		for (step = 1; step <= 20; step++) {
			sleep_ms((unsigned int)duration / 20);
			if (point(fd, 1, x1 + (x2 - x1) * step / 20,
				  y1 + (y2 - y1) * step / 20, 1))
				goto destroy;
		}
		rc = point(fd, -1, x2, y2, 0);
	} else if (argc == 3 && !strcmp(argv[1], "key")) {
		int key = number(argv[2], KEY_MAX);
		if (key < 0 || (fd = create_device(key)) < 0)
			goto out;
		rc = emit(fd, EV_KEY, key, 1) || emit(fd, EV_SYN, SYN_REPORT, 0) ||
			(sleep_ms(60), emit(fd, EV_KEY, key, 0)) || emit(fd, EV_SYN, SYN_REPORT, 0);
	} else {
		fprintf(stderr, "usage: %s tap X Y | swipe X1 Y1 X2 Y2 MS | key CODE\n", argv[0]);
		return 2;
	}
destroy:
	sleep_ms(100);
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);
out:
	if (rc)
		perror("liuqin-uinput-automation");
	return rc;
}

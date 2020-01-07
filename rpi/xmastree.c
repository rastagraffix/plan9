#include <u.h>
#include <libc.h>

/*
 * This turns a Modmypi Christmas Tree into a proper blinky-lights
 * tree when plugged into a Pi running Plan 9.  Reference source
 * is at /n/9pio/contrib/lyndon/...
 *
 * One supplier for this peripheral is https://thepihut.com/ (search for
 * the Christmas Tree Programmable Kit).
 *
 * <lyndon@orthanc.ca>   Public Domain
 *
 */

int gpio = -1;
char buf[40];

/* gpio pin numbers the LEDs map to */
int star = 12;
int leds[] = {11, 16, 17, 18, 19, 20, 21, 22, 23, 24};

int status[nelem(leds)];	/* on/off status */

void
gpiowrite (void)
{
	long len = (long) strlen(buf);
	if (write(gpio, buf, len) != len) {
		fprint(2, "gpiowrite: %r\n");
		exits("gpiowrite");
	}
}

void
on (int pin)
{
	snprint(buf, sizeof(buf), "set %d 1", pin);
	gpiowrite();
}

void
off (int pin)
{
	snprint(buf, sizeof(buf), "set %d 0", pin);
	gpiowrite();
}

static int
shutdown (void *, char *)
{
	int i;
	
	if (gpio >= 0) {	
		for (i = 0; i < nelem(leds); i++) {
			off(leds[i]);
		}
		off(star);
	}
	exits(nil);
	
	return 0;
}

void
main ()
{
	int i;
	
	atnotify(shutdown, 1);	
	gpio = open("/dev/gpio", OWRITE);
	if (gpio < 0) {
		fprint(2, "/dev/gpio: %r\n");
		exits("gpio open");
	}
	
	snprint(buf, sizeof(buf), "function %d out", star);
	gpiowrite();
	for (i = 0; i < nelem(leds); i++) {
		snprint(buf, sizeof(buf), "function %d out", leds[i]);
		gpiowrite();
	}	
	on(star);
	for (i = 0; i < nelem(leds); i++) {
		status[i] = 0;
		off(leds[i]);
	}
	srand(time(0));

	/*
	 * Every 400 ms there's a 10% chance we'll flip the state of each LED.
	 */
	while (sleep(400L) == 0) {
		for (i = 0; i < nelem(leds); i++) {
			if (nrand(100) < 10) {
				switch (status[i]) {
				  case 0:	status[i] = 1;
				  			on(leds[i]);
				  			break;
				  default:	status[i] = 0;
				  			off(leds[i]);
				}
			}
		}
	}
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

void handler(int sig)
{
  printf("\nexiting...(%d)n", sig);
  exit(0);
}

void perror_exit(char *error)
{
  perror(error);
  handler(9);
}

int main (int argc, char *argv[])
{
  struct input_event ev[64];
  int fd, rd, value, digit, size = sizeof(struct input_event);
  char name[256] = "Unknown";
  char *device = NULL;

  //Setup check
  if (argv[1] == NULL){
      printf("Please specify (on the command line) the path to the dev event interface devicen");
      exit (0);
    }

  if ((getuid()) != 0)
    printf("You are not root! This may not work...n");

  if (argc > 1)
    device = argv[1];

  //Open Device
  if ((fd = open(device, O_RDONLY)) == -1) {
    printf("%s is not a vaild device.n", device);
    perror_exit("open()");
  }

  //Print Device Name
  ioctl(fd, EVIOCGNAME(sizeof (name)), name);
  printf("Reading From : %s (%s)\n", device, name);

  while (1){
      if ((rd = read(fd, ev, size * 64)) < size)
          perror_exit("read()");

      value = ev[0].value;

      if (value != ' ' && (ev[1].value >= 0 || ev[1].value <= 2) && ev[1].type == EV_KEY){ // Only read the key press event
	switch (ev[1].code) {
            case KEY_0: digit = '0'; break;
            case KEY_1: digit = '1'; break;
            case KEY_2: digit = '2'; break;
            case KEY_3: digit = '3'; break;
            case KEY_4: digit = '4'; break;
            case KEY_5: digit = '5'; break;
            case KEY_6: digit = '6'; break;
            case KEY_7: digit = '7'; break;
            case KEY_8: digit = '8'; break;
            case KEY_9: digit = '9'; break;
            default:    digit = '\0';
            }
	   printf("Key pressed Code[%c]\n", (digit));
      }
      //if (value != ' ' && ev[1].value == 0 && ev[1].type == EV_KEY){ // Only read the key release event
	   //printf("Key released Code[%d]\n", (ev[1].code));
      //}
  }

  return 0;
}

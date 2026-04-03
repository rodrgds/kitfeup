#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define LED_PIN "509"
#define GPIO_DIR "/sys/class/gpio/gpio" LED_PIN

void write_to_file(const char *path, const char *value)
{
  int fd = open(path, O_WRONLY);
  if (fd < 0)
  {
    perror("Error opening file");
    return;
  }
  write(fd, value, strlen(value));
  close(fd);
}

int main()
{
  struct stat st = {0};

  printf("Starting KitFEUP LED Blink on PIN %s...\n", LED_PIN);

  // 1. Check if the GPIO directory already exists (like 'if test -d' in shell)
  if (stat(GPIO_DIR, &st) == -1)
  {
    printf("Exporting PIN %s...\n", LED_PIN);
    write_to_file("/sys/class/gpio/export", LED_PIN);

    // Give the Linux kernel a fraction of a second to create the virtual folders
    usleep(100000);
  }
  else
  {
    printf("PIN %s already exported.\n", LED_PIN);
  }

  // Prepare the exact file paths
  char direction_path[128];
  char value_path[128];
  snprintf(direction_path, sizeof(direction_path), "%s/direction", GPIO_DIR);
  snprintf(value_path, sizeof(value_path), "%s/value", GPIO_DIR);

  // 2. Set direction to "out" (out as in output)
  write_to_file(direction_path, "out");

  // 3. Infinite blink loop
  printf("Blinking... Press Ctrl+C to stop.\n");
  while (1)
  {
    write_to_file(value_path, "0");
    usleep(500000); // 0.5 seconds

    write_to_file(value_path, "1");
    usleep(500000);
  }

  return 0;
}

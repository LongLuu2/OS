#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_Keyboard _IOR(0, 7, char)

int main() {
    char key;
    int fd = open("/proc/keyboard_driver", O_RDONLY);

    if (fd < 0) {
        perror("Failed to open /proc/keyboard_driver");
        return -1;
    }

    printf("Waiting for a key press...\n");

    // while (1) {
    //     ioctl(fd, IOCTL_Keyboard, &key);

    //     // Only print if the keypress is not '\0'
    //     if (key != '\0') {
    //         printf("Key pressed: %c\n", key);
    //     }

    //     // Sleep for a short time to avoid excessive CPU usage
    //     usleep(100000);  // Sleep for 100ms
    // }
    ioctl(fd, IOCTL_Keyboard, &key);
    printf("Key pressed?: %c\n", key);

    close(fd);
    return 0;
}

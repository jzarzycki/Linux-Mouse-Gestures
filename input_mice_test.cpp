#include <iostream> // cout
#include <unistd.h> // read
#include <fcntl.h>  // open

int main() {
    int fd;
    char data[3];
    const char* pFile = "/dev/input/mice";

    fd = open(pFile, O_RDONLY);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", pFile);
        return -1;
    }

    int left, middle, right;
    double x, y;
    while(true) {
        read(fd, &data, sizeof(data));
        left = data[0] & 0x1;
        right = data[0] & 0x2;
        middle = data[0] & 0x4;

        x = data[1];
        y = data[2];

        std::cout << "x: " << x  << " y: " << y << std::endl << "left: " << left << " right: " << right << " middle: " << middle << std::endl;
    }

    close(fd);

    return 0;
}

#include "utils.h"
#include "includes.h"

ssize_t ckb_generate_random(size_t size, unsigned char* data) {
    int rand = open("/dev/random", O_RDONLY);
    if (rand < 0) {
        ckb_err("Error loading /dev/random");
        close(rand);
        return 0;
    }

    ssize_t result = read(rand, data, sizeof(unsigned char) * size);

    if (result < 0)
        ckb_err("Error generating random data");

    close(rand);
    return result;
}

#include "my_library.c"
#include <stdio.h>

#define error(...) (fprintf(stderr, __VA_ARGS__))

int main( int argc, char* argv[] ){
    FILE *file1, *file2;

    file1 = fopen(argv[1], "rb");
    file2 = fopen(argv[2], "rb");

    struct BMP_FILE bmp_info1, bmp_info2;
    int return_error = get_bmp_info(file1, &bmp_info1);
    if (return_error){
        error("can not read the first file\n");
        return -1;
    }
    return_error = get_bmp_info(file2, &bmp_info2);
    if (return_error){
        error("can not read the second file\n");
        return -1;
    }

    return_error = compare_info(&bmp_info1, &bmp_info2);
    if (return_error) return -1;

    if (!bmp_info1.number_of_colors) bmp_info1.number_of_colors = 256;
    unsigned int palette_size = bmp_info1.number_of_colors;
    return_error = compare_pixels(file1, file2, &bmp_info1, &palette_size);
    if (return_error) return -1;
    return 0;
}

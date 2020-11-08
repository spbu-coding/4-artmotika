#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define WRONG_FORMAT -1
#define STRUCTURE_ERROR -2
#define BMP_VERSION 40
#define THE_FIRST_100_PIXELS 100

struct BMP_FILE {
    unsigned short type;
    unsigned int size;
    int width;
    int height;
    unsigned int pixel_array_offset;
    unsigned short bits_per_pixel;
    unsigned int image_size;
    unsigned int horizontal_resolution;
    unsigned int vertical_resolution;
    unsigned int number_of_colors;
    unsigned int number_of_important_colors;
};

int read_uint(unsigned int* x, FILE* file) {
    unsigned char input[4];
    if(fread(input, 4, 1, file) != 1) {
        if(feof(file) != 0) {
            fprintf(stderr, "the file has unexpectedly ended\n");
        } else {
            fprintf(stderr, "can not read file\n");
        }
    return 0;
    }
    *x = (input[3] << 24 | input[2] << 16 | input[1] << 8 | input[0]);
    return 1;
}

int read_ushort(unsigned short* x, FILE* file) {
    unsigned char input[2];
    if(fread(input, 2, 1, file) != 1) {
        if(feof(file) != 0) {
            fprintf(stderr, "the file has unexpectedly ended\n");
        } else {
            fprintf(stderr, "can not read file\n");
        }
        return 0;
    }
    *x = (input[1] << 8 | input[0]);
    return 1;
}

int read_int(int* x, FILE* file) {
    char input[4];
    if(fread(input, 4, 1, file) != 1) {
        if(feof(file) != 0) {
            fprintf(stderr, "the file has unexpectedly ended\n");
        } else {
            fprintf(stderr, "can not read file\n");
        }
        return 0;
    }
    *x = (input[3] << 24 | input[2] << 16 | input[1] << 8 | input[0]);
    return 1;
}

int read_char(unsigned char* x, FILE* file) {
    unsigned char input[1];
    if(fread(input, 1, 1, file) != 1) {
        if(feof(file) != 0) {
            fprintf(stderr, "the file has unexpectedly ended\n");
        } else {
            fprintf(stderr, "can not read file\n");
        }
        return 0;
    }
    *x = input[0];
    return 1;
}

int write_uint(unsigned int x, FILE* file) {
    unsigned char output[4];
    output[3] = (unsigned char)((x & 0xff000000) >> 24);
    output[2] = (unsigned char)((x & 0x00ff0000) >> 16);
    output[1] = (unsigned char)((x & 0x0000ff00) >> 8);
    output[0] = (unsigned char)((x & 0x000000ff) >> 0);
    return (file && fwrite(output, 4, 1, file) == 1);
}

int write_int(int x, FILE* file) {
    char output[4];
    output[3] = (char)((x & 0xff000000) >> 24);
    output[2] = (char)((x & 0x00ff0000) >> 16);
    output[1] = (char)((x & 0x0000ff00) >> 8);
    output[0] = (char)((x & 0x000000ff) >> 0);
    return (file && fwrite(output, 4, 1, file) == 1);
}

int write_ushort(unsigned short x, FILE* file) {
    char output[2];
    output[1] = (unsigned char)((x & 0xff00) >> 8);
    output[0] = (unsigned char)((x & 0x00ff) >> 0);
    return (file && fwrite(output, 2, 1, file) == 1);
}

int write_byte(unsigned char byte, FILE* file) {
    char output[1];
    output[0] = (unsigned char)byte;
    return (file && fwrite(output, 1, 1, file) == 1);
}

bool check_type(unsigned short type) {
    char types[6][2] = {"BM", "BA", "CI", "CP", "IC", "PT"};
    for(unsigned int i = 0; i < 6; i++) {
        if(memcmp(&type, types, 2) == 0) return true;
    }
    return false;
}

int get_bmp_info(FILE* file, struct BMP_FILE *bmp_info) {
    bool is_error = false;

    unsigned int bmp_size;
    fseek(file, 0, SEEK_END);
    bmp_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if(read_ushort(&bmp_info->type, file) == 0) is_error = true;
    if(!check_type(bmp_info->type)) {
        fprintf(stderr, "the file expansion should be .bmp\n");
        return WRONG_FORMAT;
    }

    if(read_uint(&bmp_info->size, file) == 0) is_error = true;
    bmp_info->size = bmp_size;

    unsigned int reserved;
    if(read_uint(&reserved, file) == 0) is_error = true;
    if(reserved != 0) {
        fprintf(stderr, "the incorrect file structure\n");
        return STRUCTURE_ERROR;
    }

    if(read_uint(&bmp_info->pixel_array_offset, file) == 0) is_error = true;

    unsigned bmp_version;
    if(read_uint(&bmp_version, file) == 0) is_error = true;
    if(bmp_version != BMP_VERSION) {
        fprintf(stderr, "the version of bmp file should be 3\n");
        return WRONG_FORMAT;
    }

    if(read_int(&bmp_info->width, file) == 0) is_error = true;
    if(read_int(&bmp_info->height, file) == 0) is_error = true;

    unsigned short number_of_color_planes;
    if(read_ushort(&number_of_color_planes, file) == 0) is_error = true;
    if(number_of_color_planes != 1) {
        fprintf(stderr, "the incorrect file structure\n");
        return STRUCTURE_ERROR;
    }

    if(read_ushort(&bmp_info->bits_per_pixel, file) == 0) is_error = true;
    if(bmp_info->bits_per_pixel != 8 && bmp_info->bits_per_pixel != 24) {
        fprintf(stderr, "the file should have 8 or 24 bits per pixel\n");
        return WRONG_FORMAT;
    }

    unsigned int compression_method;
    if(read_uint(&compression_method, file) == 0) is_error = true;
    if(compression_method != 0) {
        fprintf(stderr, "it doesn't support compression\n");
        return WRONG_FORMAT;
    }

    if(read_uint(&bmp_info->image_size, file) == 0) is_error = true;
    if(bmp_info->image_size != 0) {
        unsigned int calculated_size = bmp_info->size - bmp_info->pixel_array_offset;
        if(calculated_size != bmp_info->image_size) {
            fprintf(stderr, "the size differents\n");
            return STRUCTURE_ERROR;
        }
    }

    if(read_uint(&bmp_info->horizontal_resolution, file) == 0) is_error = true;
    if(read_uint(&bmp_info->vertical_resolution, file) == 0) is_error = true;
    if(read_uint(&bmp_info->number_of_colors, file) == 0) is_error = true;
    if(read_uint(&bmp_info->number_of_important_colors, file) == 0) is_error = true;

    if (is_error){
        fprintf(stderr, "can not read bmp structure\n");
        return STRUCTURE_ERROR;
    }
    return 0;
}

int convert_bmp(FILE *input_file, FILE *output_file, struct BMP_FILE *bmp_info){
    unsigned int PALETTE_BEGINNING = 54, PALETTE_END = bmp_info->pixel_array_offset;
    unsigned char byte = 0;

    while(ftell(input_file) < bmp_info->size){
        if (read_char(&byte, input_file) == 0){
            fprintf(stderr, "can not read file\n");
            return STRUCTURE_ERROR;
        }
        if (bmp_info->bits_per_pixel == 8 && ftell(input_file)>PALETTE_BEGINNING &&ftell(input_file)<PALETTE_END && (ftell(input_file)-PALETTE_BEGINNING)%4 != 0){
            byte = ~byte;
        }else if (bmp_info->bits_per_pixel == 24 && ftell(input_file)>bmp_info->pixel_array_offset){
            byte = ~byte;
        }
        write_byte(byte, output_file);
    }
    return 0;
}

int compare_info(struct BMP_FILE *info1, struct BMP_FILE *info2){
    if (abs(info1->width) != abs(info2->width)){
        fprintf(stderr, "different widths");
        return -1;
    }
    if (info1->height != info2->height) {
        fprintf(stderr, "different heights");
        return -1;
    }
    if (info1->bits_per_pixel != info2->bits_per_pixel){
        fprintf(stderr, "different bits per pixel");
        return -1;
    }
    if ((info1->number_of_colors != info2->number_of_colors) && info1->bits_per_pixel == 8){
        fprintf(stderr, "different number colors in palette");
        return -1;
    }
    return 0;
}



int compare_pixels(FILE *file1, FILE *file2, struct BMP_FILE *bmp_info, unsigned int *palette_size){
    unsigned int count_of_pixels = 0;
    if (bmp_info->width < 0) bmp_info->width = abs(bmp_info->width);
    if (bmp_info->height < 0) bmp_info->height = abs(bmp_info->height);

    if (bmp_info->bits_per_pixel == 8){
        unsigned int palette1[256];
        if (fread(palette1, sizeof(unsigned int), *palette_size, file1) != *palette_size){
            fprintf(stderr, "unreadable palette of picture 1\n");
            return -1;
        }
        unsigned int palette2[256];
        if (fread(palette2, sizeof(unsigned int), *palette_size, file2) != *palette_size){
            fprintf(stderr, "unreadable palette of picture 2\n");
            return -1;
        }

        unsigned char pixel1 = 0, pixel2 = 0;
        for (int i = 0; i < bmp_info->height; i++){
            for(int j = 0; j < bmp_info->width; j++){
                if (fread(&pixel1, 1, 1, file1) != 1){
                    fprintf(stderr, "can not read the pixel of picture 1\n");
                    return -1;
                }
                if (fread(&pixel2, 1, 1, file2) != 1){
                    fprintf(stderr, "can not read the pixel of picture 2\n");
                    return -1;
                }
                if (palette1[pixel1] != palette2[pixel2]){
                    fprintf(stderr,"%d %d\n", j, i);
                    count_of_pixels++;
                }
                if (count_of_pixels >= THE_FIRST_100_PIXELS) break;
            }
            if (count_of_pixels >= THE_FIRST_100_PIXELS) break;
        }
    }else{
        char pixel1[3], pixel2[3];
        for (int i = 0; i < bmp_info->height; i++) {
            for (int j = 0; j < bmp_info->width; j++) {
                if (fread(pixel1, 1, 3, file1) != 3){
                    fprintf(stderr, "can not read the pixel of picture 1\n");
                    return -1;
                }
                if (fread(pixel2, 1, 3, file2) != 3){
                    fprintf(stderr, "can not read the pixel of picture 2\n");
                    return -1;
                }

                int rgbp1 = (0x00 << 24 | pixel1[2] << 16 | pixel1[1] << 8 | pixel1[0]);
                int rgbp2 = (0x00 << 24 | pixel2[2] << 16 | pixel2[1] << 8 | pixel2[0]);

                if (rgbp1 != rgbp2){
                    fprintf(stderr,"%d %d\n", j, i);
                    count_of_pixels++;
                }
                if (count_of_pixels >= THE_FIRST_100_PIXELS) break;
            }
            if (count_of_pixels >= THE_FIRST_100_PIXELS) break;
        }
    }
    return 0;
}

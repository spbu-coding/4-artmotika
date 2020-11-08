#include "qdbmp.h"
#include "my_library.c"
#include <stdio.h>
#include <string.h>

#define error(...) (fprintf(stderr, __VA_ARGS__))

BMP*	bmp;

int read_arguments( int argc, char* argv[], int* is_mine ){
    if ( argc != 4 ){
        error("Incorrect arguments");
        return -1;
    }

    if (strcmp(argv[1], "--mine") == 0){
        * is_mine = 1;
    } else if (strcmp(argv[1], "--theirs") == 0){
        * is_mine = 0;
    }else{
        error("Incorrect arguments");
        return -1;
    }

    for (int j = 2; j < 4; ++j){
        int index_expansion = 0;
        for (int i = 0; i < strlen(argv[j]); ++i){
            if (argv[j][i] == '.') index_expansion = i + 1;
        }
        if (index_expansion != 0) {
            if (argv[j][index_expansion] != 'b'){
                error("Incorrect expansion");
                return -1;
            }
            if (argv[j][index_expansion + 1] != 'm'){
                error("Incorrect expansion");
                return -1;
            }
            if (argv[j][index_expansion + 2] != 'p'){
                error("Incorrect expansion");
                return -1;
            }
        }else{
            error("Incorrect expansion");
            return -1;
        }
    }
    return 0;
}

int make_their_bmp(){

    UCHAR	r, g, b;
    UINT	width, height;
    USHORT depth;
    UINT	x, y;

    /* Get image's dimensions */
    width = BMP_GetWidth( bmp );
    height = BMP_GetHeight( bmp );
    depth = BMP_GetDepth( bmp );

    /* Iterate through all the image's pixels */ // Нужны битовые операции
    if (depth == 24) {
        for (x = 0; x < width; ++x) {
            for (y = 0; y < height; ++y) {
                /* Get pixel's RGB values */
                BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
                if(BMP_LAST_ERROR_CODE != BMP_OK)
                    return -3;
                /* Invert RGB values */
                BMP_SetPixelRGB(bmp, x, y, ~r, ~g, ~b);
            }
        }
    }else if (depth == 8){
        for (int i = 0; i < bmp->Header.ColorsUsed; ++i){
            BMP_GetPaletteColor(bmp, i, &r, &g, &b);
            if (BMP_LAST_ERROR_CODE != BMP_OK)
                return -3;
            BMP_SetPaletteColor(bmp, i, ~r, ~g, ~b);
        }
        // Инвертирование таблицы цветов
    }else return -3;

    return 0;
}

int make_mine_bmp( char* argv[] ){
    struct BMP_FILE bmp_info;
    FILE* file;
    file = fopen(argv[2], "rb+");
    int return_error = get_bmp_info(file, &bmp_info);
    if (return_error != 0) return return_error;

    fseek(file, 0, SEEK_SET);
    FILE *write_file;
    write_file = fopen(argv[3], "wb+");
    return_error= convert_bmp(file, write_file, &bmp_info);
    if (return_error != 0) return return_error;
    return 0;
}

int main( int argc, char* argv[] ){
    /* Check arguments */
    int is_mine, return_make_bmp;

    int read_arg_return = read_arguments(argc, argv, &is_mine);
    if (read_arg_return){
        return read_arg_return;
    }

    if (is_mine){
        return_make_bmp = make_mine_bmp(argv);
        if (return_make_bmp)
            return return_make_bmp;
    }else{
        bmp = BMP_ReadFile( argv[ 2 ] );
        if(BMP_LAST_ERROR_CODE != BMP_OK)
            return -3;
        return_make_bmp = make_their_bmp();
        BMP_WriteFile( bmp, argv[ 3 ] );
        if(BMP_LAST_ERROR_CODE != BMP_OK)
            return -3;
        BMP_Free( bmp );
        if (return_make_bmp)
            return return_make_bmp;
    }
    return 0;
}

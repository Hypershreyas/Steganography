
#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

#define DECODE_IMAGE_BUF_SIZE 8
#define DECODE_MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char *output_fname;
    FILE *fptr_output;

    char image_data[DECODE_IMAGE_BUF_SIZE];

    char extn_secret_file[DECODE_MAX_FILE_SUFFIX];
    int extn_size;

    int size_secret_file;

} DecodeInfo;

// Read and validate decode arguments
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

// Open files
Status open_decode_files(DecodeInfo *decInfo);

// Main decoding operation
Status do_decoding(DecodeInfo *decInfo);

// Decode one byte from 8 image bytes
Status decode_byte_from_lsb(char *data, char *image_buffer);

// Decode multiple bytes from image
Status decode_data_from_image(char *data, int size, FILE *fptr_image);

// Decode size information
Status decode_size(int *size, FILE *fptr_image);

// Decode magic string
Status decode_magic_string(DecodeInfo *decInfo);

// Decode secret file extension
Status decode_secret_file_extn(DecodeInfo *decInfo);

// Decode secret file size
Status decode_secret_file_size(DecodeInfo *decInfo);

// Decode secret file data
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
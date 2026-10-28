
#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "common.h"
#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE 8
#define MAX_FILE_SUFFIX 5

typedef struct _EncodeInfo
{
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    uint bits_per_pixel;
    char image_data[MAX_IMAGE_BUF_SIZE];

    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;

    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;


// Check the operation type (encoding or decoding)
OperationType check_operation_type(char *argv[]);

// Read and validate the encoding command-line arguments
Status read_and_validate_encode_args(char *argv[],
                                     EncodeInfo *encInfo);

// Perform the complete encoding operation
Status do_encoding(EncodeInfo *encInfo);

// Open the source image, secret file, and output image
Status open_files(EncodeInfo *encInfo);

// Check whether the source image has enough capacity
Status check_capacity(EncodeInfo *encInfo);

// Get the image data size from the BMP image
uint get_image_size_for_bmp(FILE *fptr_image);

// Get the size of a file in bytes
uint get_file_size(FILE *fptr);

// Copy the BMP header from the source image to the output image
Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_dest_image);

// Encode the magic string into the image
Status encode_magic_string(const char *magic_string,EncodeInfo *encInfo);

// Encode the secret file extension into the image
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

// Encode the secret file size into the image
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo);

// Encode the secret file data into the image
Status encode_secret_file_data(EncodeInfo *encInfo);

// Encode multiple bytes of data into the image
Status encode_data_to_image(const char *data,int size,FILE *fptr_src_image,FILE *fptr_stego_image,EncodeInfo *encInfo);

// Encode one byte into the LSBs of 8 image bytes
Status encode_byte_to_lsb(char data, char *image_buffer);

// Copy the remaining image data 
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

// Encode size information into the image
Status encode_size(int size, FILE *fptr_src,FILE *fptr_stego);

// Encode size  into the LSBs 
Status encode_size_to_lsb(int data, char *image_buffer);

#endif
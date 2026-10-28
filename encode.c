
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"

/* Validate encoding arguments */
Status read_and_validate_encode_args(char *argv[],
                                     EncodeInfo *encInfo)
{
    char *ext;

    if (argv[2] == NULL || argv[3] == NULL)
    {
        printf("<< Missing input arguments >>\n");
        return e_failure;
    }

    ext = strrchr(argv[2], '.');

    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        printf("<< Source image must be BMP >>\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    if (argv[4] != NULL)
        encInfo->stego_image_fname = argv[4];
    else
        encInfo->stego_image_fname = "stego.bmp";

    return e_success;
}

/* Get file size */
uint get_file_size(FILE *fptr)
{
    long size;

    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    rewind(fptr);

    return (uint)size;
}

/* Get BMP image size */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    unsigned char header[4];

    fseek(fptr_image, 18, SEEK_SET);

    if (fread(header, 1, 4, fptr_image) != 4)
        return 0;

    return (uint)(header[0] |
                  (header[1] << 8) |
                  (header[2] << 16) |
                  (header[3] << 24));
}

/* Open files */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image =
        fopen(encInfo->src_image_fname, "rb");

    if (encInfo->fptr_src_image == NULL)
    {
        printf("<< Failed to open source image >>\n");
        return e_failure;
    }

    encInfo->fptr_secret =
        fopen(encInfo->secret_fname, "rb");

    if (encInfo->fptr_secret == NULL)
    {
        printf("<< Failed to open secret file >>\n");
        fclose(encInfo->fptr_src_image);
        encInfo->fptr_src_image = NULL;
        return e_failure;
    }

    encInfo->fptr_stego_image =
        fopen(encInfo->stego_image_fname, "wb");

    if (encInfo->fptr_stego_image == NULL)
    {
        printf("<< Failed to create stego image >>\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        encInfo->fptr_src_image = NULL;
        encInfo->fptr_secret = NULL;
        return e_failure;
    }

    return e_success;
}

/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_dest_image)
{
    char header[54];

    rewind(fptr_src_image);

    if (fread(header, 1, 54, fptr_src_image) != 54)
        return e_failure;

    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
        return e_failure;

    return e_success;
}

/* Encode one byte into 8 image bytes */
Status encode_byte_to_lsb(char data,
                          char *image_buffer)
{
    unsigned char ch = (unsigned char)data;

    for (int i = 7; i >= 0; i--)
    {
        image_buffer[i] =
            (image_buffer[i] & 0xFE) | (ch & 1);

        ch >>= 1;
    }

    return e_success;
}

/* Encode multiple bytes */
Status encode_data_to_image(const char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image,
                            EncodeInfo *encInfo)
{
    char image_buffer[8];

    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, 1, 8,
                  fptr_src_image) != 8)
            return e_failure;

        encode_byte_to_lsb(data[i], image_buffer);

        if (fwrite(image_buffer, 1, 8,
                   fptr_stego_image) != 8)
            return e_failure;
    }

    return e_success;
}

/* Encode 32-bit size */

Status encode_size_to_lsb(int data,
                          char *image_buffer)
{
    unsigned int value = (unsigned int)data;

    for (int i = 0; i < 32; i++)
    {
        int bit = (value >> (31 - i)) & 1;

        image_buffer[i] =
            (image_buffer[i] & 0xFE) | bit;
    }

    return e_success;
}

/* Store integer size using 32 image bytes */
Status encode_size(int size,
                   FILE *fptr_src,
                   FILE *fptr_stego)
{
    char image_buffer[32];

    if (fread(image_buffer, 1, 32, fptr_src) != 32)
        return e_failure;

    encode_size_to_lsb(size, image_buffer);

    if (fwrite(image_buffer, 1, 32, fptr_stego) != 32)
        return e_failure;

    return e_success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string,
                           EncodeInfo *encInfo)
{
    return encode_data_to_image(
        magic_string,
        strlen(magic_string),
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image,
        encInfo);
}

/* Encode extension */
Status encode_secret_file_extn(const char *file_extn,
                               EncodeInfo *encInfo)
{
    int size = strlen(file_extn);

    if (encode_size(size,
                    encInfo->fptr_src_image,
                    encInfo->fptr_stego_image) != e_success)
        return e_failure;

    return encode_data_to_image(
        file_extn,
        size,
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image,
        encInfo);
}

/* Encode secret file size */
Status encode_secret_file_size(int file_size,
                               EncodeInfo *encInfo)
{
    return encode_size(file_size,
                       encInfo->fptr_src_image,
                       encInfo->fptr_stego_image);
}

/* Encode secret file contents */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        if (encode_data_to_image(
                &ch, 1,
                encInfo->fptr_src_image,
                encInfo->fptr_stego_image,
                encInfo) != e_success)
            return e_failure;
    }

    return e_success;
}

/* Copy remaining image bytes */
Status copy_remaining_img_data(FILE *fptr_src,
                               FILE *fptr_dest)
{
    char buffer[1024];
    size_t count;

    while ((count = fread(buffer, 1,
                          sizeof(buffer), fptr_src)) > 0)
    {
        if (fwrite(buffer, 1, count, fptr_dest) != count)
            return e_failure;
    }

    return e_success;
}

/* Check image capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    long image_size;
    long secret_size;
    long required;

    fseek(encInfo->fptr_src_image, 0, SEEK_END);
    image_size = ftell(encInfo->fptr_src_image);

    fseek(encInfo->fptr_secret, 0, SEEK_END);
    secret_size = ftell(encInfo->fptr_secret);

    rewind(encInfo->fptr_src_image);
    rewind(encInfo->fptr_secret);

    encInfo->size_secret_file = secret_size;

    required = 54 +
        (long)strlen(MAGIC_STRING) * 8 +
        32 +  /* extension length */
        (long)strlen(encInfo->extn_secret_file) * 8 +
        32 +  /* secret size */
        secret_size * 8;

    if (image_size < required)
    {
        printf("<< Image does not have enough capacity >>\n");
        return e_failure;
    }

    return e_success;
}

/* Main encoding process */
Status do_encoding(EncodeInfo *encInfo)
{
    char *ext;
    int ext_size;

    ext = strrchr(encInfo->secret_fname, '.');

    if (ext == NULL)
    {
        printf("<< Secret file must have an extension >>\n");
        return e_failure;
    }

    ext_size = strlen(ext);

    if (ext_size >= MAX_FILE_SUFFIX)
    {
        printf("<< Secret file extension is too long >>\n");
        return e_failure;
    }

    strcpy(encInfo->extn_secret_file, ext);

    if (open_files(encInfo) != e_success)
        return e_failure;

    printf("<< Files opened successfully >>\n");

    if (check_capacity(encInfo) != e_success)
        goto failure;

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) != e_success)
        goto failure;

    printf("<< BMP header copied >>\n");

    if (encode_magic_string(MAGIC_STRING,
                            encInfo) != e_success)
        goto failure;

    printf("<< Magic string encoded >>\n");

    if (encode_secret_file_extn(
            encInfo->extn_secret_file,
            encInfo) != e_success)
        goto failure;

    printf("<< File extension encoded >>\n");

    if (encode_secret_file_size(
            (int)encInfo->size_secret_file,
            encInfo) != e_success)
        goto failure;

    printf("<< Secret file size encoded >>\n");

    if (encode_secret_file_data(encInfo) != e_success)
        goto failure;

    printf("<< Secret file data encoded >>\n");

    if (copy_remaining_img_data(
            encInfo->fptr_src_image,
            encInfo->fptr_stego_image) != e_success)
        goto failure;

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    printf("<< Encoding completed successfully >>\n");
    return e_success;

failure:
    printf("<< Encoding failed >>\n");

    if (encInfo->fptr_src_image)
        fclose(encInfo->fptr_src_image);

    if (encInfo->fptr_secret)
        fclose(encInfo->fptr_secret);

    if (encInfo->fptr_stego_image)
        fclose(encInfo->fptr_stego_image);

    return e_failure;
}
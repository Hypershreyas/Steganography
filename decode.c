
#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

/* Validate decoding arguments */
Status read_and_validate_decode_args(char *argv[],
                                     DecodeInfo *decInfo)
{
    char *ext;

    if (argv[2] == NULL)
    {
        printf("<< Missing stego image filename >>\n");
        return e_failure;
    }

    ext = strrchr(argv[2], '.');

    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        printf("<< Please provide a BMP image >>\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    if (argv[3] != NULL)
        decInfo->output_fname = argv[3];
    else
        decInfo->output_fname = "decoded";

    decInfo->fptr_stego_image = NULL;
    decInfo->fptr_output = NULL;

    return e_success;
}

/* Open stego image */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        printf("<< Failed to open stego image >>\n");
        return e_failure;
    }

    return e_success;
}

/* Decode one byte */
Status decode_byte_from_lsb(char *data,
                            char *image_buffer)
{
    unsigned char ch = 0;

    for (int i = 0; i < 8; i++)
    {
        ch = (ch << 1) |
             ((unsigned char)image_buffer[i] & 1);
    }

    *data = (char)ch;

    return e_success;
}

/* Decode multiple bytes */
Status decode_data_from_image(char *data,
                              int size,
                              FILE *fptr_image)
{
    char image_buffer[8];

    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, 1, 8,
                  fptr_image) != 8)
            return e_failure;

        decode_byte_from_lsb(&data[i], image_buffer);
    }

    return e_success;
}

/* Decode 32-bit size */
Status decode_size(int *size,
                   FILE *fptr_image)
{
    char image_buffer[32];
    unsigned int value = 0;

    if (fread(image_buffer, 1, 32,
              fptr_image) != 32)
        return e_failure;

    for (int i = 0; i < 32; i++)
    {
        value = (value << 1) |
                ((unsigned char)image_buffer[i] & 1);
    }

    *size = (int)value;

    return e_success;
}

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic[sizeof(MAGIC_STRING)] = {0};

    if (decode_data_from_image(
            magic,
            strlen(MAGIC_STRING),
            decInfo->fptr_stego_image) != e_success)
        return e_failure;

    magic[strlen(MAGIC_STRING)] = '\0';

    if (strcmp(magic, MAGIC_STRING) != 0)
    {
        printf("<< Invalid magic string >>\n");
        return e_failure;
    }

    printf("<< Magic string verified successfully >>\n");
    return e_success;
}

/* Decode extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    if (decode_size(&decInfo->extn_size,
                    decInfo->fptr_stego_image) != e_success)
        return e_failure;

    if (decInfo->extn_size <= 0 ||
        decInfo->extn_size >= DECODE_MAX_FILE_SUFFIX)
    {
        printf("<< Invalid file extension size >>\n");
        return e_failure;
    }

    if (decode_data_from_image(
            decInfo->extn_secret_file,
            decInfo->extn_size,
            decInfo->fptr_stego_image) != e_success)
        return e_failure;

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    printf("<< File extension decoded successfully >>\n");
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    if (decode_size(&decInfo->size_secret_file,
                    decInfo->fptr_stego_image) != e_success)
        return e_failure;

    if (decInfo->size_secret_file < 0)
    {
        printf("<< Invalid secret file size >>\n");
        return e_failure;
    }

    printf("<< Secret file size decoded successfully >>\n");
    return e_success;
}

/* Decode secret file contents */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;

    decInfo->fptr_output =
        fopen(decInfo->output_fname, "wb");

    if (decInfo->fptr_output == NULL)
    {
        printf("<< Failed to create output file >>\n");
        return e_failure;
    }

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (decode_data_from_image(
                &ch, 1,
                decInfo->fptr_stego_image) != e_success)
        {
            printf("<< Failed to read secret data >>\n");
            fclose(decInfo->fptr_output);
            decInfo->fptr_output = NULL;
            return e_failure;
        }

        if (fwrite(&ch, 1, 1,
                   decInfo->fptr_output) != 1)
        {
            printf("<< Failed to write decoded data >>\n");
            fclose(decInfo->fptr_output);
            decInfo->fptr_output = NULL;
            return e_failure;
        }
    }

    fclose(decInfo->fptr_output);
    decInfo->fptr_output = NULL;

    printf("<< Secret data decoded successfully >>\n");
    return e_success;
}

/* Main decoding process */
Status do_decoding(DecodeInfo *decInfo)
{
    char output_file[256];
    int result;

    if (open_decode_files(decInfo) != e_success)
        return e_failure;

    printf("<< Stego image opened successfully >>\n");

    if (fseek(decInfo->fptr_stego_image,
              54, SEEK_SET) != 0)
    {
        printf("<< Failed to skip BMP header >>\n");
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }

    if (decode_magic_string(decInfo) != e_success)
        goto failure;

    if (decode_secret_file_extn(decInfo) != e_success)
        goto failure;

    result = snprintf(output_file, sizeof(output_file),
                      "%s%s",
                      decInfo->output_fname,
                      decInfo->extn_secret_file);

    if (result < 0 || result >= sizeof(output_file))
    {
        printf("<< Output filename is too long >>\n");
        goto failure;
    }

    decInfo->output_fname = output_file;

    if (decode_secret_file_size(decInfo) != e_success)
        goto failure;

    if (decode_secret_file_data(decInfo) != e_success)
        goto failure;

    fclose(decInfo->fptr_stego_image);
    decInfo->fptr_stego_image = NULL;

    printf("<< Decoding completed successfully >>\n");
    return e_success;

failure:
    if (decInfo->fptr_stego_image != NULL)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
    }

    return e_failure;
}
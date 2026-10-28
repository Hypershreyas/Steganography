#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "types.h"
#include "decode.h"

/* Check operation type */
OperationType check_operation_type(char *argv[])
{
    if (argv[1] == NULL)
    {
        return e_unsupported;
    }

    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    printf("\n============================================\n");
    printf("             << STEGANOGRAPHY >>\n");
    printf("                   ~SHREYAS~\n");
    printf("============================================\n");

    if (argc < 2)
    {
        printf("<< Invalid arguments >>\n");
        return 1;
    }

    /* Encoding operation */
    if (check_operation_type(argv) == e_encode)
    {
        printf("\n");
        printf("+------------------------------------------+\n");
        printf("|      [ ENCODING OPERATION SELECTED ]     |\n");
        printf("+------------------------------------------+\n");

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("\n");
            printf("+------------------------------------------+\n");
            printf("|      [ ENCODE ARGUMENTS VALIDATED ]      |\n");
            printf("+------------------------------------------+\n");

            if (do_encoding(&encInfo) == e_success)
            {
                printf("\n");
                printf("+------------------------------------------+\n");
                printf("|          [ ENCODING COMPLETED ]          |\n");
                printf("+------------------------------------------+\n");
            }
            else
            {
                printf("\n");
                printf("+------------------------------------------+\n");
                printf("|            [ ENCODING FAILED ]           |\n");
                printf("+------------------------------------------+\n");
            }
        }
        else
        {
            printf("\n");
            printf("+------------------------------------------+\n");
            printf("|  [ ENCODE ARGUMENTS VALIDATION FAILED ]  |\n");
            printf("+------------------------------------------+\n");
        }
    }

    /* Decoding operation */
    else if (check_operation_type(argv) == e_decode)
    {
        printf("\n");
        printf("+------------------------------------------+\n");
        printf("|      [ DECODING OPERATION SELECTED ]     |\n");
        printf("+------------------------------------------+\n");

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("\n");
            printf("+------------------------------------------+\n");
            printf("|      [ DECODE ARGUMENTS VALIDATED ]      |\n");
            printf("+------------------------------------------+\n");

            if (do_decoding(&decInfo) == e_success)
            {
                printf("\n");
                printf("+------------------------------------------+\n");
                printf("|          [ DECODING COMPLETED ]          |\n");
                printf("+------------------------------------------+\n");
            }
            else
            {
                printf("\n");
                printf("+------------------------------------------+\n");
                printf("|            [ DECODING FAILED ]           |\n");
                printf("+------------------------------------------+\n");
            }
        }
        else
        {
            printf("\n");
            printf("+------------------------------------------+\n");
            printf("|   [ DECODE ARGUMENT VALIDATION FAILED ]  |\n");
            printf("+------------------------------------------+\n");
        }
    }
    else
    {
        printf("\n");
        printf("+------------------------------------------+\n");
        printf("|         [ UNSUPPORTED OPERATION ]        |\n");
        printf("+------------------------------------------+\n");
        printf("\n");
        printf("<< Encoding Hint : -e <secret.txt> <source.bmp> >>\n");
        printf("\n");
        printf("<< Decoding Hint : -d <stego.bmp> >>\n");
        printf("\n");
    }

    return 0;
}
#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    if (argc < 2)
    {
        printf("Usage:\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("Decoding: ./a.out -d <source.bmp> [output file]\n");
        return 0;
    }

    if (argv[1][0] != '-')
    {
        printf("Usage:\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("Decoding: ./a.out -d <source.bmp> [output file]\n");
        return 0;
    }

    if (check_operation_type(argv[1][1]) == e_encode)
    {
        if (argc < 4)
        {
            printf("Encoding usage: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
            return 0;
        }

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding done successfully\n");
            }
            else
            {
                printf("Encoding failed\n");
            }
        }
    }
    else if (check_operation_type(argv[1][1]) == e_decode)
    {
        if (argc < 3)
        {
            printf("Decoding usage: ./a.out -d <source.bmp> [output file]\n");
            return 0;
        }

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding done successfully\n");
            }
            else
            {
                printf("Decoding failed\n");
            }
        }
    }
    else
    {
        printf("Usage:\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("Decoding: ./a.out -d <source.bmp> [output file]\n");
    }

    return 0;
}
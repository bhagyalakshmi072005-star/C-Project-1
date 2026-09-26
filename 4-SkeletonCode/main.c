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
        printf("Encoding: ./lsb_steg -e <source.bmp> <secret.txt>\n");
        printf("Decoding: ./lsb_steg -d <source.bmp>\n");
        return 0;
    }

    if (check_operation_type(argv[1][1]) == e_encode)
    {
        if (argc < 4)
        {
            printf("Usage: ./lsb_steg -e <source.bmp> <secret.txt>\n");
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
            printf("Usage: ./lsb_steg -d <source.bmp>\n");
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
        printf("Unsupported operation\n");
    }

    return 0;
}
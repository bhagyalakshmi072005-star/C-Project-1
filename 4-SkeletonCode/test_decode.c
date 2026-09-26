#include <stdio.h>
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    DecodeInfo decInfo;
    if (argc < 3)
    {
        printf("Usage: ./decode_test -d <.bmp_file> [output file]\n");
        return 0;
    }
    if (check_operation_type(argv[1][1]) == e_decode)
    {
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
        printf("ERROR: Unsupported operation\n");
    }

    return 0;
}
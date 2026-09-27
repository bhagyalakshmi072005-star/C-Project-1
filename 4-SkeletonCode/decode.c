#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char opt)
{
    if(opt=='e')
    {
        return e_encode;
    }
    else if(opt=='d')
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (strstr(argv[2], ".bmp")==NULL)
    {
        printf("ERROR : Source image should be a .bmp file\n");
        return e_failure;
    }

    decInfo->stego_image_fname=argv[2];
    if (argv[3]==NULL)
    {
        decInfo->user_output_fname="NULL";
    }
    else
    {
        decInfo->user_output_fname=argv[3];
    }

    if (open_decode_files(decInfo)==e_failure)
    {
        return e_failure;
    }

    return e_success;
}


Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n",decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

Status create_output_file(DecodeInfo *decInfo, char *user_name)
{
    char *dot;
    char name[100];
    if (user_name == NULL)
    {
        strcpy(name, "decoded");
    }
    else
    {
        strcpy(name, user_name);
        dot=strrchr(name, '.');
        if (dot != NULL)
        {
            *dot = '\0';
        }
    }
    strcpy(decInfo->output_fname, name);
    strcat(decInfo->output_fname, decInfo->extn_secret_file);
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to create output file %s\n",decInfo->output_fname);
        return e_failure;
    }
    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    char data=0;
    for (int i=0;i<8;i++)
    {
        data=data<<1;
        data=data | (image_buffer[i] & 1);
    }
    return data;
}


Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    char magic_string[3];
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
    for (int i = 0; i < 2; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
        magic_string[i] = decode_byte_from_lsb(buffer);
    }
    magic_string[2] = '\0';
    if (magic_string[0] == '#' &&
        magic_string[1] == '*')
    {
        return e_success;
    }
    return e_failure;
}


Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size = 0;
    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }
    for (int i = 0; i < 32; i++)
    {
        size = (size << 1) | (buffer[i] & 1);
    }
    decInfo->size_secret_file = size;
    return e_success;
}


Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
        decInfo->extn_secret_file[i] =
            decode_byte_from_lsb(buffer);
    }
    decInfo->extn_secret_file[decInfo->size_secret_file] = '\0';
    return e_success;
}


Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size = 0;
    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }
    for (int i = 0; i < 32; i++)
    {
        size = (size << 1) | (buffer[i] & 1);
    }
    decInfo->size_secret_file = size;
    return e_success;
}


Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char data;
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
        data = decode_byte_from_lsb(buffer);
        if (fwrite(&data, 1, 1, decInfo->fptr_output) != 1)
        {
            return e_failure;
        }
    }
    return e_success;
}


Status do_decoding(DecodeInfo *decInfo)
{
    if (decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR : Magic string decoding failed\n");
        return e_failure;
    }
    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        printf("ERROR : Failed to decode extension size\n");
        return e_failure;
    }
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("ERROR : Failed to decode extension\n");
        return e_failure;
    }
    if (create_output_file(decInfo, decInfo->user_output_fname) == e_failure)
{
    printf("ERROR : Failed to create output file\n");
    return e_failure;
}
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        printf("ERROR : Failed to decode secret file size\n");
        return e_failure;
    }
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR : Failed to decode secret file data\n");
        return e_failure;
    }
    return e_success;
}
#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("ERROR: Source image should be a .bmp file\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];

    encInfo->secret_fname = argv[3];

    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "output.bmp";
    }
    else
    {
        if (strstr(argv[4], ".bmp") == NULL)
        {
            printf("ERROR: Output file should be a .bmp file\n");
            return e_failure;
        }

        encInfo->stego_image_fname = argv[4];
    }

    if (open_files(encInfo) == e_failure)
    {
        return e_failure;
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Insufficient capacity in image\n");
        return e_failure;
    }
    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to copy BMP header\n");
        return e_failure;
    }
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode magic string\n");
        return e_failure;
    }
    if (encode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode extension size\n");
        return e_failure;
    }
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode extension\n");
        return e_failure;
    }
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode secret file size\n");
        return e_failure;
    }
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode secret file data\n");
        return e_failure;
    }
    if (copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to copy remaining image data\n");
        return e_failure;
    }
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    uint extn_size;
    uint required_size;

    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    extn_size = strlen(strrchr(encInfo->secret_fname, '.'));

    required_size = 2 + 4 + extn_size + 4 +
                    encInfo->size_secret_file;

    if (required_size * 8 > encInfo->image_capacity)
    {
        return e_failure;
    }

    return e_success;
}

uint get_file_size(FILE *fptr)
{
    uint size;
    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    rewind(fptr);
    return size;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];
    rewind(fptr_src_image);
    if (fread(buffer, 54, 1, fptr_src_image) != 1)
    {
        return e_failure;
    }
    if (fwrite(buffer, 54, 1, fptr_dest_image) != 1)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; magic_string[i] != '\0'; i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 7; i >= 0; i--)
    {
        image_buffer[7 - i]=(image_buffer[7 - i] & ~1) | ((data >> i) & 1);
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; file_extn[i] != '\0'; i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char *dot = strrchr(encInfo->secret_fname, '.');
    if (dot == NULL)
    {
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file, dot);
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(strlen(encInfo->extn_secret_file), buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_size_to_lsb(unsigned int size, char *image_buffer)
{
    for (int i = 31; i >= 0; i--)
    {
        if (size & (1 << i))
        {
            image_buffer[31 - i] = image_buffer[31 - i] | 1;
        }
        else
        {
            image_buffer[31 - i] = image_buffer[31 - i] & ~1;
        }
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char secret_data;
    while (fread(&secret_data, 1, 1, encInfo->fptr_secret) == 1)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(secret_data, buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}
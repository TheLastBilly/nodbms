#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef bool
#define bool                                    unsigned char
#define true                                    1
#define false                                   0
#endif

#define DELIMITER                               ','
#define END_OF_ROW                              '\"'

#define info(_fmt, ...)                         printf("[" "\033[1;96m" "info" "\033[0m" "]")
#define warn(_fmt, ...)                         printf("[" "\033[1;93m" "warn" "\033[0m" "]")
#define erro(_fmt, ...)                         printf("[" "\033[1;91m" "erro" "\033[0m" "]")

#define errmsg(_ret)                            strerror(_ret < 0 ? -_ret : _ret)

#define min(x, y, t)                            ({t _x = (x), _y = (y); ((_x) <= (_y) ? (_x) : (_y));})
#define max(x, y, t)                            ({t _x = (x), _y = (y); ((_x) > (_y) ? (_x) : (_y));})

extern int errno;

typedef char ** row_t;
typedef bool (*query_t)(row_t row, int len);

/**
 * @brief Reads file contents into dynamically allocated string.
 * 
 * @warning Memory MUST be freed by the user.
 * 
 * @param file_path     Path to file
 * @param data          Pointer to store file data start into
 * @param size          Pointer to store file size into
 * 
 * @return int          0 on success, error code on failure
 */
static int 
read_file( const char * path, char ** data, size_t *size )
{
    ssize_t read_size = 0;
    FILE * fp = NULL;

    fp = fopen(path, "r");
    if(fp == NULL)
    {
        warn("failed to open \"%s\"", path);
        return errno;
    }
    
    fseek(fp, 0, SEEK_END);
    read_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *data = (char *)calloc(read_size + 1, 1);

    if(size)
        *size = read_size;
    while(read_size > 0)
        read_size -= fread(*data, sizeof(**data), read_size, fp);

    fclose(fp);

    return 0;
}

/**
 * @brief Writes contents of NULL terminated string
 * into a file
 * 
 * @param path          Path to file
 * @param data          Pointer to string
 * 
 * @return int          0 on success, error code on failure
 */
int
write_file( const char * path, char * data )
{
    int ret = 0;
    FILE * fp = NULL;

    fp = fopen(path, "w");
    if(fp == NULL)
    {
        warn("failed to open \"%s\"", path);
        return errno;
    }

    ret = fprintf(fp, data);
    if(ret > 0)
        ret = 0;
    
    fclose(fp);

    return ret;
}

/**
 * @brief Appends contents of NULL terminated string
 * into a file
 * 
 * @param path          Path to file
 * @param data          Pointer to string
 * 
 * @return int          0 on success, error code on failure
 */
static int
append_file( const char * path, char * data )
{
    int ret = 0;
    FILE * fp = NULL;

    fp = fopen(path, "a");
    if(fp == NULL)
    {
        warn("failed to open \"%s\"", path);
        return errno;
    }

    ret = fprintf(fp, data);
    if(ret > 0)
        ret = 0;
    
    fclose(fp);

    return ret;
}

/**
 * @brief Reads row into results 2D string array 
 * 
 * @param row               Pointer to row string
 * @param results           Pointer to 2D array 
 * @param len               Len of results
 * @param cell_size         Max size of each 
 * @param read 
 * @return int 
 */
static int
read_row( char * row, row_t results, int len, int cell_size, int *read )
{
    char c = 0;
    int s = 0, i = 0, cell = 0;

    if(row == NULL || results == NULL)
        return -EINVAL;
    
    for(i = 0; (c = row[i]) && cell < len && c != END_OF_ROW; i++)
    {
        if(c != DELIMITER)
            continue;
        
        snprintf(results[cell], min(i - s, cell_size, int), 
            "%s", &row[s]);
            
        cell++;
        s = i+1;
    }

    if(read)
        *read = cell;

    return 0;
}

static int
write_row( const char * path, row_t row, int len )
{
    char * buffer = NULL;
    int i = 0, o = 0, ret = 0;
    size_t size = 0;

    if(row == NULL)
        return -EINVAL;

    if(!len)
        return 0;
    
    // Get the size of the row to write to
    size = 2;
    for(i = 0; i < len; i++)
        size += strlen(row[i]) + 1;

    // Allocate buffer row
    buffer = malloc(sizeof(buffer[0]) * (size + 1));

    // Write columns to buffer
    o = 0;
    for(i = 0; i < len; i++)
    {
        sprintf(&buffer[o], "%s%c", row[i], DELIMITER);
        o += strlen(row[i]) + 1;
    }
    snprintf(&buffer[o], "%c", END_OF_ROW);

    // Write buffer to file
    ret = append_file(path, buffer);
    free(buffer);

    return ret;
}

static int
query_table( char * table, query_t query, int results[], int max_results )
{
    char c = 0;
    char row[10][20] = {0};
    int start = 0, results_count = 0, row_index = 0,
        i = 0, len = 0, ret = 0;

    if(results == NULL)
        return -EINVAL;

    for(i = 0; (c = table[i]) && results_count < max_results; i++)
    {
        if(c != END_OF_ROW)
            continue;

        ret = read_row(&table[start], row, sizeof(row)/sizeof(row[0]), 
            sizeof(row[0]), &len);
        start = i + 1;

        if(ret)
            return ret;

        if(query(row, len))
            results[results_count++] = row_index;

        row_index++;
    }

    return 0;
}

int 
main( int argc, char const *argv[] )
{
    
    return 0;
}

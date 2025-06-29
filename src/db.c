#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <header.h>
#include <stdlib.h>

char *read_file(const char *path, size_t *out_size)
{
    FILE *fp = fopen(path, "rb"); /* always binary */
    if (!fp)
    {
        perror("fopen");
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        perror("fseek");
        fclose(fp);
        return NULL;
    }
    long len = ftell(fp);
    if (len < 0)
    {
        perror("ftell");
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    char *buf = malloc((size_t)len + 1);
    if (!buf)
    {
        perror("malloc");
        fclose(fp);
        return NULL;
    }

    size_t n = fread(buf, 1, (size_t)len, fp);
    if (n != (size_t)len)
    {
        if (feof(fp))
            fprintf(stderr, "Unexpected EOF (%zu/%ld)\n", n, len);
        else
            perror("fread");
        free(buf);
        fclose(fp);
        return NULL;
    }
    buf[len] = '\0';

    fclose(fp);
    if (out_size)
        *out_size = (size_t)len;
    return buf;
}

// Open DB connection
sqlite3 *openDatabase(const char *filename)
{
    sqlite3 *db;
    int rc = sqlite3_open(filename, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

int createTables(sqlite3 *db, char *schemaFile)
{

    char *sql = NULL;
    char *errMsg = NULL;
    int rc = SQLITE_ERROR;

    sql = read_file(schemaFile, NULL);
    if (!sql)
    {
        fprintf(stderr, "Could not read %s\n", schemaFile);
        return rc;
    }

    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error creating users table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return 1; // Success
}

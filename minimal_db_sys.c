#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct Address
{
    // four fields in an Address struct
    int id;
    int set;
    char *name;
    char *email;
};
struct Database
{
    // the size of a database is fixed.
    int MAX_DATA;
    int MAX_ROWS;
    // rows is a pointer that points to address
    struct Address *rows;
};
// Connection is a struct that holds a file and a database. They have to keep updated.
struct Connection
{
    FILE *file;
    struct Database *db;
};
void Database_close(struct Connection *conn);
void die(const char *message, struct Connection *conn)
{
    if (errno)
    {
        perror(message);
    }
    else
    {
        printf("ERROR: %s\n", message);
    }

    if (conn)
    {
        Database_close(conn);
    }

    exit(1);
}
void Address_print(struct Address *addr)
{
    printf("%i,%i,%s,%s\n", addr->id, addr->set, addr->name, addr->email);
}
// write file info to the db.
// file contains the whole content of rows, but in db struct, rows is just a pointer pointing to the contents.
void Database_load(struct Connection *conn)
{
    // flags to see if writing has been properly handled.
    int flag_load_MACRO;
    int flag_load_ints[conn->db->MAX_ROWS];
    int flag_load_strs[conn->db->MAX_ROWS];
    // In file we save the concrete address data. But when loaded to the db, we have to first malloc space then use a pointer.
    // Every time we open and load a db, we have to malloc a new space to keep the data in file.
    // Do we have to have an end flag for non-char array?
    conn->db->rows = malloc((sizeof(struct Address)) * conn->db->MAX_ROWS);
    // Notice that only the pointer addrpt is assigned to conn->db->rows. The data in file is not assigned to conn->db->rows.
    // write data from to db pointer points to, from the file.
    // first write 2 ints, MAX_DATA and MAX_ROW. Notice that the file pointer automatically update its position.
    flag_load_MACRO = fread(conn->db, sizeof(int), 2, conn->file);
    // if successfully it should return 2.
    // write data into where rows point to.
    for (int i = 0; i < conn->db->MAX_ROWS; i++)
    {
        /*
        handle writing of different granularities with the help of type casting and char *
        rows is an address pointer, so it increments with the size of an address.
        If we want to write data within the address, we need a pointer that is of finer granularity, like char * and do type casting to write other type to the address
         */

        // // temp is a char * pointer with the granularity of 1 byte pointing at the start of this address
        // char *temp = (char *)((conn->db->rows) + i);
        // // convert char * to int * to write int id and int set
        // flag_load_ints[i]= fread((int *)temp,sizeof(int), 2, conn->file);
        flag_load_ints[i] = (fread(&(conn->db->rows[i].id), sizeof(int), 1, conn->file) &
                             fread(&(conn->db->rows[i].set), sizeof(int), 1, conn->file));
        // flag_load_ints[i] = fread(&(conn->db->rows[i].set), sizeof(int), 1, conn->file);
        conn->db->rows[i].name = malloc(conn->db->MAX_DATA);
        conn->db->rows[i].email = malloc(conn->db->MAX_DATA);
        flag_load_strs[i] = (fread(conn->db->rows[i].name, sizeof(char), conn->db->MAX_DATA, conn->file)) & (fread(conn->db->rows[i].email, sizeof(char), conn->db->MAX_DATA, conn->file));
        // if successfully it should return MAX_ROWS.
        if ((flag_load_ints[i] * flag_load_strs[i]) == 0)
        {
            die("Failed to load database", conn);
        }
    }
}
// when you have just a single file(char *), Database_open gives you the connection, while Database_load help you update the db.
// In Database_load, based on the db pointer, we allocate space for MAX_DATA, MAX_ROWS and rows.
struct Connection *Database_open(const char *filename, char mode)
{
    struct Connection *conn = malloc(sizeof(struct Connection));
    conn->db = malloc(sizeof(struct Database));
    // After searching the web, I found that the malloc function does not initialize the memory it allocates. So we have to initialize the memory ourselves. But we have access to the member elements now using ->.
    if (!conn)
    {
        die("Memory error", conn);
    }
    // the function argument filename is a char * but we want a FILE * in the struct connection. fopen does that.
    if (mode == 'c')
    {
        conn->file = fopen(filename, "w");
    }
    else
    {
        conn->file = fopen(filename, "r+");
        if (conn->file)
        {
            Database_load(conn);
        }
    }
    if (!conn->file)
    {
        die("Failed to open the file", conn);
    }

    return conn;
}
// free the memory that db asked for. close the file.
void database_close(struct Connection *conn)
{
    if (conn)
    {
        if (conn->file)
        {
            fclose(conn->file);
        }
        if (conn->db)
        {
            if (conn->db->rows)
            {
                // first free all the name and email pointers in the rows struct.
                for (int i = 0; i < conn->db->MAX_ROWS; i++)
                {
                    // free the name and email pointer in the rows struct.
                    if (conn->db->rows[i].name)
                    {
                        free(conn->db->rows[i].name);
                    }
                    if (conn->db->rows[i].email)
                    {
                        free(conn->db->rows[i].email);
                    }
                }
                // free rows pointer itself
                free(conn->db->rows);
            }
            free(conn->db);
        }
        free(conn);
    }
}
// After operation to the db, first Db_write then Db_close.
// database is updated according to the user. Now write database info back to the file.
void Database_write(struct Connection *conn)
{
    // NOT READY
    // there is only one db saved in one file.
    // set the FILE pointer to the start of the file.
    rewind(conn->file);
    int flag_write_ints;
    int flag_write_rows;
    // write two ints to the file.
    flag_write_ints = fwrite(conn->db, sizeof(int), 2, conn->file);
    // write rows to the file.
    // in the db, rows is a pointer. We need to retrieve the values and write those values to the file.

    flag_write_rows = (fwrite(conn->db->rows, sizeof(char), conn->db->MAX_DATA, conn->file) &
                       fwrite(conn->db->rows, sizeof(char), conn->db->MAX_DATA, conn->file));
    if ((flag_write_ints * flag_write_rows) == 0)
    {
        die("Failed to write database", conn);
    }

    // data might be in the internal buffer of the file, so we need to flush the file.
    int flag_flush_file = fflush(conn->file);
    if (flag_flush_file == EOF)
    {
        die("Cannot flush database.", conn);
    }
}
// In Database_open, the conn->db pointer points to a block of memory with nothing in it.
// after Database_create, database pointer not only has memory, but also has int, rows in it.
void Database_create(struct Connection *conn, int MAX_DATA, int MAX_ROWS)
{
    // initialize two ints in the db struct.
    conn->db->MAX_DATA = MAX_DATA;
    conn->db->MAX_ROWS = MAX_ROWS;
    // initialize rows pointer in the db struct.
    // for a pointer, first allocate space, then assign concrete data type to the space.
    // allocate space first
    conn->db->rows = malloc(sizeof(struct Address) * MAX_ROWS);
    for (int i = 0; i < conn->db->MAX_ROWS; i++)
    {
        // assign to the space
        conn->db->rows[i] = (struct Address){.id = i, .set = 0};
    }
}
// set the name and email of a specific row in the conn database
void Database_set(struct Connection *conn, int id, const char *name, const char *email)
{
    // set is used to see if we have already set this row
    if (conn->db->rows[id].set)
    {
        die("Already set, delete it first", conn);
    }
    // assign the function argument pointer to name and email of db, which are also pointers.
    // conn->db->rows[id].name = name;
    // conn->db->rows[id].email = email;
    // the reason why it does look like that is that we cannot assign a const to a non-const
    // the result is we could change the const string by the newly assigned pointer.
    // it's better we use strdup to copy the string to a new space.
    conn->db->rows[id].name = strdup(name);
    conn->db->rows[id].email = strdup(email);
    conn->db->rows[id].set = 1;
}
// get the info of a specific address in the conn database
void Database_get(struct Connection *conn, int id)
{
    struct Address *temp_addr = &(conn->db->rows[id]);
    if (temp_addr->set)
    {
        Address_print(temp_addr);
    }
    else
    {
        die("ID is not set", conn);
    }
}
// delete an address in the db rows by assigning it a new blank address.
void Database_delete(struct Connection *conn, int id)
{
    struct Address temp_addr = {.id = id, .set = 0};
    conn->db->rows[id] = temp_addr;
}
// list the addresses that you have changed
void Database_list(struct Connection *conn)
{
    for (int i = 0; i < conn->db->MAX_ROWS; i++)
    {
        if (conn->db->rows[i].set)
        {
            Address_print(&conn->db->rows[i]);
        }
    }
}

int main(int argc, char *argv[])
{
    int MAX_DATA = 0;
    int MAX_ROWS = 0;
    int id = 0;
    if (argc < 5)
    {
        die("USAGE: ex17 <dbfile> MAX_DATA MAX_ROWS <action> [action params]", NULL);
    }
    char *filename = argv[1];
    MAX_DATA = atoi(argv[2]);
    MAX_ROWS = atoi(argv[3]);
    char action = argv[4][0];
    if (argc > 5)
    {
        id = atoi(argv[5]);
    }
    struct Connection *conn = Database_open(filename, action);
    switch (action)
    {
    case 'c':
        Database_create(conn, MAX_DATA, MAX_ROWS);
        Database_write(conn);
        break;
    case 'g':
        if (argc != 6)
        {
            die("Need an id to get", conn);
        }
        Database_get(conn, id);
        break;
    case 's':
        if (argc != 8)
        {
            die("Need id, name, email to set", conn);
        }
        Database_set(conn, id, argv[6], argv[7]);
        Database_write(conn);
        break;
    case 'd':
        if (argc != 6)
        {
            die("Need id to delete", conn);
        }
        Database_delete(conn, id);
        Database_write(conn);
        break;
    case 'l':
        Database_list(conn);
        break;
    default:
        die("Invalid action, only: c=create, g=get, s=set, d=del, l=list", conn);
    }
    database_close(conn);
    return 0;
}
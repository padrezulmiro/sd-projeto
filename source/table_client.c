#include "client_stub.h"
#include "data.h"
#include "entry.h"
#include "table_client.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Invalid args!\nUsage: table-client <server>:<port>\n");
        exit(-1);
    }

    //FIXME Error checking needs to be done in several parts of this function

    //char* address_port = argv[1];
    struct rtable_t* rtable = rtable_connect(argv[1]);

    if (!rtable) {
        perror("Error connecting to remote server");
        exit(-1);
    }

    int terminated = 0;
    while (!terminated) {
        printf("Command:\n");
        char line[100]; //FIXME Remove magic number
        char* ret_fgets = fgets(line, 99, stdin);
        switch (parse_operation(ret_fgets)) {
            case PUT: {
                char* key = strtok(line, " ");
                char* data = strtok(line, "\n");

                //XXX does data need to be null terminated? Doesn't seem so...
                struct data_t* data_obj = data_create(strlen(data), data);
                struct entry_t* entry = entry_create(key, data_obj);

                int ret_put = rtable_put(rtable, entry);
                if (ret_put == 0) {
                    printf("Entry with key \"%s\" was added", key);
                } else {
                    printf(
                        "There was an error adding entry with key \"%s\"", key
                    );
                }

                entry_destroy(entry);
                break;
            }

            case GET: {
                char* key = strtok(line, "\n");
                struct data_t* data = rtable_get(rtable, key);

                if (!data) {
                    printf(
                        "There was an error retrieving data from key %s",
                        key
                    );
                    break;
                }

                printf("%.*s\n", data->datasize, (char*) data->data);
                data_destroy(data);
                break;
            }

            case DEL: {
                char* key = strtok(line, "\n");
                int ret_destroy = rtable_del(rtable, key);

                if (!ret_destroy) {
                    printf("Key %s was destroyed\n", key);
                } else {
                    printf("Key %s does not exist or there was an error\n", key);
                }

                break;
            }

            case SIZE: {
                int size = rtable_size(rtable);
                if (size < 0) {
                    printf("There was an error retrieving table's size");
                } else {
                    printf("%d\n", size);
                }
                break;
            }

            case GETKEYS: {
                char** keys = rtable_get_keys(rtable);

                if (!keys) {
                    printf("There was an error retrieving keys");
                    break;
                }

                printf("Keys in table:\n");

                int i = 0;
                char* key = keys[i];
                while (key) {
                    printf("'%s'\n", key);
                    key = keys[++i];
                }

                rtable_free_keys(keys);
                break;
            }

            case GETTABLE: {
                struct entry_t** entries = rtable_get_table(rtable);

                if (!entries) {
                    printf("There was an error retrieving table");
                    break;
                }

                printf("Entries in table:\n");

                int i = 0;
                struct entry_t* entry = entries[i];
                while (entry) {
                    printf("'%s': %.*s", entry->key, entry->value->datasize,
                           (char*) entry->value->data);
                    entry = entries[++i];
                }

                rtable_free_entries(entries);
                break;
            }

            case QUIT:
                terminated = 1;
                printf("Quitting...\n");
                break;

            default:
                printf("Invalid command. Please try again.\n");
                break;
        }
    }

    if (rtable_disconnect(rtable) == -1) {
        perror("Error disconnecting from remote server");
        exit(-1);
    }

    return 0;
}

operation parse_operation(char *op_str) {
    if (strcmp(op_str, SIZE_STR) == 0) {
        return SIZE;
    } else if (strcmp(op_str, DEL_STR) == 0) {
        return DEL;
    } else if (strcmp(op_str, GETKEYS_STR) == 0) {
        return GETKEYS;
    } else if (strcmp(op_str, GETTABLE_STR) == 0) {
        return GETTABLE;
    } else if (strcmp(op_str, QUIT_STR) == 0) {
        return QUIT;
    }

    char* operation = strtok(op_str, " ");
    if (strcmp(operation, PUT_STR) == 0) {
        return PUT;
    } else if (strcmp(operation, GET_STR) == 0) {
        return GET;
    }

    return INVALID;
}

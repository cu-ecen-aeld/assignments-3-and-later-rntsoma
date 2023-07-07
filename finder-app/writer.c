#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *args[]) {
    char *string;
    char *file_path;
    FILE *file;

    if (argc < 3) {
        printf("Missing FILE or WRITE_STR\n");
        syslog(LOG_USER | LOG_PERROR, "Missing FILE or WRITE_STR");
        return 1;
    }

    file_path = args[1];
    string = args[2];
    file = fopen(file_path, "w");

    if (file == NULL) {
        printf("Failed to open file %s\n", file_path);
        syslog(LOG_USER | LOG_PERROR, "%s\n", strerror(errno));
        return 1;
    }
    fputs(string, file);
    fclose(file);
    syslog(LOG_USER | LOG_DEBUG, "Writing %s to %s", string, file_path);

    return 0;
}

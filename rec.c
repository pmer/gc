#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
    char *pattern = argv[1];
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    struct timespec tp;
    int err;
    int master;
    FILE* masterfile;
    pid_t pid;
    size_t writes;

    master = posix_openpt(O_RDWR);
    assert(master != -1);

    pid = fork();
    assert(pid != -1);

    if (pid == 0) {
        err = grantpt(master);
        assert(err != -1);

        err = unlockpt(master);
        assert(err != -1);

        char* slavepath = ptsname(master);
        assert(slavepath);

        close(master);

        int slave = open(slavepath, O_RDWR);
        assert(slave != -1);

        dup2(slave, 1);
        dup2(slave, 2);

        execvp(argv[2], argv + 2);
        perror("execvp");
    }

    else {
        masterfile = fdopen(master, "r");
        assert(masterfile);

        while ((linelen = getline(&line, &linecap, masterfile)) > 0) {
            if (strstr(line, pattern)) {
                writes = fwrite("\33c\33[3J", 6, 1, stdout);
                assert(writes == 1);
            }

            writes = fwrite(line, linelen, 1, stdout);
            assert(writes == 1);
        }
    }

    return 0;
}

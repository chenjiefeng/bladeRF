#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "script.h"

struct script {
    struct script *prev;
    unsigned int line_no;
    FILE *file;
    const char *name;
    ino_t inode;
};

int cli_open_script(struct script **s, const char *filename)
{
    FILE *f;
    int error;
    struct stat sb;
    struct script *to_check;
    int fd;

    f = fopen(filename, "r");
    if (!f) {
        error = -errno;
        goto cli_open_script__file_err;
    }

    fd = fileno(f);
    if (fd < 0) {
        error = -errno;
        goto cli_open_script__file_err;
    }

    struct script *new_script = malloc(sizeof(*new_script));
    if (!new_script) {
        return -errno;;
        goto cli_open_script__malloc_err;
    }

    if (fstat(fd, &sb) < 0) {
        error = -errno;
        goto cli_open_script__stat_err;
    }

    for (to_check = *s; to_check != NULL; to_check = (*s)->prev) {
        if (to_check->inode == sb.st_ino) {
            error = 1;
            goto cli_open_script__recursive_err;
        }
    }

    new_script->name = strdup(filename);
    if (!new_script->name) {
        error = -errno;
        goto cli_open_script__name_err;
    }

    new_script->inode = sb.st_ino;
    new_script->prev = *s;
    new_script->line_no = 0;
    new_script->file = f;
    *s = new_script;

    return 0;

cli_open_script__recursive_err:
cli_open_script__name_err:
cli_open_script__stat_err:
    free(new_script);
cli_open_script__malloc_err:
    fclose(f);
cli_open_script__file_err:
    return error;
}

int cli_close_script(struct script **s)
{
    int ret = 0;
    struct script *prev;

    if (*s == NULL) {
        return 0;
    }

    prev = (*s)->prev;
    free((void*)(*s)->name);

    if (fclose((*s)->file) < 0) {
        ret = -errno;
    }
    free(*s);
    *s = prev;
    return ret;
}

int cli_close_all_scripts(struct script **s)
{
    int ret = 0;

    while (!ret && *s) {
        ret = cli_close_script(s);
    }

    return ret;
}

unsigned int cli_num_scripts(struct script *curr_script) {
    struct script *s;
    unsigned int ret = 0;

    for (s = curr_script; s != NULL; s = curr_script->prev) {
        ret++;
    }

    return ret;
}

bool cli_script_loaded(struct script *s)
{
    bool is_loaded;

    is_loaded = s != NULL;

    /* Sanity check...if s is non-null we *should* have a file opened */
    assert(!is_loaded || s->file != NULL);

    return is_loaded;
}

FILE * cli_script_file(struct script *s)
{
    FILE *ret = NULL;

    if (s && s->file) {
        ret = s->file;
    }

    return ret;
}

const char * cli_script_file_name(struct script *s)
{
    if (s) {
        return s->name;
    } else {
        return "(unknown file)";
    }
}

void cli_script_bump_line_count(struct script *s)
{
    if (s) {
        s->line_no++;
    }
}

unsigned int cli_script_line(struct script *s)
{
    unsigned int ret;

    if (s) {
        ret = s->line_no;
    } else {
        ret = 0;
    }

    return ret;
}

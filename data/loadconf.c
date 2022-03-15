/*
** $Id: loadconf.c 2703 2008-08-27 20:27:34Z andre.dig $
** watchcatd - Watchcat Daemon
** See copyright notice in distro's COPYRIGHT file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GLIBC__
#include <alloca.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>

#include "loadconf.h"
#include "watchcatd.h"

#define DEFAULT_PID_FILE    "/var/run/catmaster.pid"
#define DEFAULT_DEVICE      "/var/run/watchcat.socket"

static int handle_master_debug_level(char *val, int *mod)
{
    int n = atoi(val);
    if (n < 0 || n > 32767)
        return EINVAL;
    GLOBAL_CONF.master.debug_level = n;
    *mod = 1;
    return 0;
}

static int handle_priority(char *val, int *mod)
{
    int n = atoi(val);
    if (n <= 0 || n >= 100)
        return EINVAL;
    GLOBAL_CONF.priority = n;
    *mod = 1;
    return 0;
}

static int handle_realtime(char *val, int *mod)
{
    int n;
    if (strcmp(val, "true") == 0)
        n = 1;
    else if (strcmp(val, "false") == 0)
        n = 0;
    else
        return EINVAL;
    GLOBAL_CONF.realtime_mode = n;
    *mod = 1;
    return 0;
}

static int handle_user(char *val, int *mod)
{
    if (strlen(val) >= sizeof GLOBAL_CONF.user)
        return ENAMETOOLONG;

    if (getpwnam(val) == NULL) {
        if (errno == 0 || errno == EBADF || errno == ESRCH)
            return EINVAL;  /* user not found */
        else
            return errno;
    }
    strcpy(GLOBAL_CONF.user, val);
    *mod = 1;
    return 0;
}

static int handle_slave_debug_level(char *val, int *mod)
{
    int n;

    n = atoi(val);
    if (n < 0 || n > 32767)
        return EINVAL;
    GLOBAL_CONF.slave.debug_level = n;
    *mod = 1;
    return 0;
}

#define S_ISFTYPE(mode, type) (((mode) & S_IFMT) == (type))

static int test_path_and_stat(char *dest, int size, char *def, int type,
                              char *val, int *mod)
{
    char *end_val;
    int r, len;
    struct stat st;

    if (*val != '/')
        return EINVAL;
    len = strlen(val);
    if (len >= size)
        return ENAMETOOLONG;
    end_val = alloca(size);
    strcpy(end_val, val);

    if (lstat(end_val, &st) == -1) {
        char *p;

        if (errno != ENOENT)
            return errno;

        while (end_val[1] == '/')
            end_val++;
        p = strrchr(end_val, '/');
        assert(p);
        
        /* If there is no file, check if the base directory exists. */
        if (p > end_val) {
            *p = '\0';
            if (lstat(end_val, &st) == -1)
                return errno == ENOENT ? ENOENT : EINVAL;
            if (!S_ISDIR(st.st_mode))
                return EINVAL;
            *p = '/';
        }

        strcpy(dest, end_val);
        *mod = 1;
        return 0;
    }

    if (S_ISDIR(st.st_mode)) {
        char *p;
        while (len > 0 && end_val[len - 1] == '/') {
            len--;
            end_val[len] = '\0';
        }
        p = strrchr(def, '/');
        assert(p);
        if (size <= len + strlen(p))
            return ENAMETOOLONG;
        strcat(end_val, p);
	r = lstat(end_val, &st);
	if ((r == 0 || errno != ENOENT) &&
            (r == -1 || !S_ISFTYPE(st.st_mode, type))) {
            return EINVAL;
        }
    }
    else if (!S_ISFTYPE(st.st_mode, type))
        return EINVAL;

    strcpy(dest, end_val);
    *mod = 1;
    return 0;
}

static int handle_master_pid_file(char *val, int *mod)
{
    return test_path_and_stat(GLOBAL_CONF.master.pid_file,
                              sizeof(GLOBAL_CONF.master.pid_file),
                              DEFAULT_PID_FILE, S_IFREG, val, mod);
}

static int handle_slave_device(char *val, int *mod)
{
    return test_path_and_stat(GLOBAL_CONF.slave.device,
                              sizeof(GLOBAL_CONF.slave.device),
                              DEFAULT_DEVICE, S_IFSOCK, val, mod);
}

#define MAX_CONN (64 * 1024)

static int handle_slave_max_connections(char *val, int *mod)
{
    int n = atoi(val);
    if (n < 1 || n > MAX_CONN)
        return EINVAL;
    GLOBAL_CONF.slave.max_conections = n;
    *mod = 1;
    return 0;
}

static int handle_slave_max_per_pid(char *val, int *mod)
{
    int n = atoi(val);
    if (n < 0 || n > MAX_CONN)
        return EINVAL;
    GLOBAL_CONF.slave.max_conections_per_pid = n;
    *mod = 1;
    return 0;
}

/* Config Engine */

typedef int (*VALUE_HANDLER)(char *, int *);

typedef struct {
    char var[128];
    VALUE_HANDLER handler;
    char defval[128];
} PARM;

static PARM CONF_PARMS[] = {
  /* var                        handler                     defval */
    {"realtime",                handle_realtime,            "true"},
    {"priority",                handle_priority,            "1"},
    {"user",                    handle_user,                "nobody"},
    {"master_debug_level",      handle_master_debug_level,  "1023"},
    {"master_pid_file",         handle_master_pid_file,     DEFAULT_PID_FILE},
    {"slave_debug_level",       handle_slave_debug_level,   "1023"},
    {"slave_device",            handle_slave_device,        DEFAULT_DEVICE},
    {"slave_max_connections",   handle_slave_max_connections, "128"},
    {"slave_max_connections_per_pid", handle_slave_max_per_pid, "2"},
};

#define NPARMS       (sizeof(CONF_PARMS)/sizeof(PARM))
#define MAX_LINELEN  1024

typedef int MODIFIED[NPARMS];

static void store_config_parm(char *var, char *val, MODIFIED mod)
{
    int i;

    for (i = 0; i < NPARMS; i++)
        if (strcmp(var, CONF_PARMS[i].var) == 0) {
            int err = CONF_PARMS[i].handler(val, &mod[i]);
            if (err != 0) {
                fprintf(stderr, "%s(= `%s'): %s\n", var, val, strerror(err));
                exit(err);
            }
            return;
        }
    fprintf(stderr, "Unknown parameter: `%s'\n", var);
    exit(EINVAL);
}

static void set_defaults(MODIFIED mod)
{
    int i;
    for (i = 0; i < NPARMS; i++)
        if (!mod[i]) {
            PARM *parm = &CONF_PARMS[i];
            int err = parm->handler(parm->defval, &mod[i]);
            if (err != 0) {
                fprintf(stderr, "%s(= `%s'): %s\n", parm->var, parm->defval,
                        strerror(err));
                exit(err);
            }
        }
}

void load_conf(char *filename)
{
    int err;
    char line[MAX_LINELEN];
    char *varp, *valp;
    char *endvar, *endval;
    FILE *fp;
    MODIFIED mod;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(errno);
    }
    
    memset(mod, 0, sizeof(mod));
    err = 0;
    while (fgets(line, sizeof(line), fp) != NULL && ferror(fp) == 0) {
        varp = line;
        while (isspace(*varp))
            varp++;
        if (*varp == '#' || *varp == '\0')   /* comment or empty line */
            continue;
        
        valp = strchr(line, '=');
        if (valp == NULL) {
            fclose(fp);
            fprintf(stderr, "Invalid line: `%s'\n", line);
            exit(EINVAL);
        }
        endvar = valp - 1;
        valp++;
        
        while (isspace(*endvar))
            endvar--;
        *(endvar+1) = '\0';

        while (isspace(*valp))
            valp++;

        endval = strchr(valp, '#');
        if (endval == NULL)
            endval = valp + strlen(valp);
        endval--;

        while (isspace(*endval))
            endval--;
        *(endval+1) = '\0';

        store_config_parm(varp, valp, mod);
	}
    set_defaults(mod);
    fclose(fp);
}


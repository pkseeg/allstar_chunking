/*
** $Id: master.c 2699 2008-08-27 15:02:49Z andre.dig $
** watchcatd - Watchcat Daemon
** See copyright notice in distro's COPYRIGHT file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <event.h>
#include <syslog.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>

#include "master.h"
#include "utils.h"
#include "watchcatd.h"
#include "loadconf.h"

/* DEBUG_LEVEL
 * Bit  Description         Volume
 * 0    Security.           low
 * 1    Kill send.          moderate/high
 * 2    Kill fail.          low
 * 3    Children sock.      low
 * 4    Children die.       low
 * 5    Children finished.  low
 */

#define BUG_SECURITY    1
#define BUG_KILL_SEND   2
#define BUG_KILL_FAIL   4
#define BUG_CHILD_SOCK  8
#define BUG_CHILD_DIE   16
#define BUG_CHILD_END   32

/* Alias for commonly used and miscellaneous configuration parameters.
 * These values need to be treated as constants!
 */
static int DEBUG_LEVEL = 1023;
#ifdef WCAT_PREFIX
static char EXEC_SLAVE[256] = WCAT_PREFIX "/lib/watchcatd";
#else
static char EXEC_SLAVE[256] = "/usr/lib/watchcatd";
#endif
static int LISTENER;
static int CHILD_COUNT;
static int FINISHING;

#define DEFAULT_CONFIG_FILE     "/etc/watchcatd.conf"

/* NOTE, only assert can be used in mysyslog! */
void mysyslog(int priority, const char *format, ...)
{
    va_list ap;
    openlog("cat/master", LOG_NDELAY | LOG_PID, LOG_DAEMON);
    va_start(ap, format);
    vsyslog(priority, format, ap);
    va_end(ap);
    closelog();
}

static void mykill(int fd, pid_t pid, int sig)
{
    if (pid <= 1 || pid == getpid()) {
        if (DEBUG_LEVEL & BUG_SECURITY)
            mysyslog(LOG_ERR, "SECURITY invalid pid to kill, fd=%i, pid=%i, "
                     "mypid=%i", fd, pid, getpid());
        return;
    }

    if (kill(pid, SIGKILL)) {
        if (DEBUG_LEVEL & BUG_KILL_FAIL)
            mysyslog(LOG_ERR, "KILL FAILED, fd=%i, pid=%i, signal=%i, "
                     "errno=%i, strerror=`%s'", fd, pid, sig, errno,
                     strerror(errno));
        return;
    }

    if (DEBUG_LEVEL & BUG_KILL_SEND)
        mysyslog(LOG_ERR, "KILL sent, fd=%i, pid=%i, signal=%i",
                 fd, pid, sig);
}

static void myread(int fd, void *buf, size_t count, jmp_buf env)
{
    ssize_t r;
    ssize_t remaining = count;
    
    do {
        r = read(fd, (char *)buf + count - remaining, remaining);
        remaining -= r > 0 ? r : 0;
    } while (r > 0 && remaining > 0);
    
    if (r > 0 && remaining == 0)
        return;
            
    if (r < 0 && (DEBUG_LEVEL & BUG_CHILD_SOCK))
        mysyslog(LOG_ERR, "SOCK read FAIL, fd=%i", fd);
    longjmp(env, 1);
}

static void child_read(int fd, short event, void *arg)
{
    jmp_buf env;

    if (setjmp(env) == 0) {
        int type;
        myread(fd, &type, sizeof(type), env);
        switch (type) {
        case MSG_KILL: {
            struct kill_msg msg;
            myread(fd, &msg, sizeof(msg), env);
            mykill(msg.fd, msg.pid, msg.signal);
            break;
        }
        case MSG_LOG: {
            struct log_msg msg;
            char buf[128];
            const int size = sizeof(buf);
            myread(fd, &msg, sizeof(msg), env);
            myassert(msg.length < size);
            myread(fd, buf, msg.length, env);
            buf[msg.length] = '\0';
            openlog("cat/slave", LOG_NDELAY | LOG_PID, LOG_DAEMON);
            syslog(msg.priority, "%s", buf);
            closelog();
            break;
        }
        default:
            myassert(0);
        }
    }
    else {
        struct event *ev = arg;
        event_del(ev);
        free(ev);
        myassert_sys(close(fd) == 0);
    }
}

static void chfdn(int oldfd, int newfd)
{
    if (oldfd == newfd)
        return;
    myassert(!is_fd_open(newfd));
    myassert_sys(dup2(oldfd, newfd) == newfd);
    myassert_sys(close(oldfd) == 0);
}

/* CAUTION: exec_on_slave only can be called on master's children!
 * This function doesn't return.
 * * */
static void exec_on_slave(int sfd, int slistener)
{
    struct stat st;

    /* Control and socket fds. */
    myassert_sys(close(FROM_SLAVE_TO_MASTER_FD) == 0);
    myassert_sys(close(SOCKET_LISTENER_FD) == 0);
    chfdn(sfd, FROM_SLAVE_TO_MASTER_FD);
    chfdn(slistener, SOCKET_LISTENER_FD);

    if (lstat(EXEC_SLAVE, &st) == 0 && S_ISREG(st.st_mode) &&
        (st.st_mode & S_IXUSR)) {
        myassert_sys(execl(EXEC_SLAVE, EXEC_SLAVE, NULL) == 0);
    }
    mysyslog(LOG_ERR, "File `%s' doesn't exist or denied access or it's a "
             "simbolic link", EXEC_SLAVE);
    exit(0);
}

static void create_a_slave(void)
{
    static int spare = 10;
    static time_t t = 0;

    int socks[2];
    int mfd; /* Alias for master fd. */
    int sfd; /* Alias for slave fd. */
    int slistener;
    pid_t pid;
    struct event *evchild;
    int i;

    i = time(NULL);
    if (i != t) {
        t = i;
        spare = 10;
    }
    if (spare <= 0) {
        myassert(spare == 0);
        mysyslog(LOG_ERR, "The catslave is dying very fast, "
                 "aborting the catmaster...");
        exit(1);
    }
    spare--;
            
    myassert_sys(socketpair(PF_UNIX, SOCK_STREAM, 0, socks) == 0);
    mfd = socks[0];
    sfd = socks[1];
    myassert_sys(fcntl(mfd, F_SETFD, FD_CLOEXEC) == 0);

    /* Slave's copy from LISTENER */
    slistener = dup(LISTENER);
    myassert(slistener >= 0);

    i = 3;
    do {
        pid = fork();
        if (pid < 0)
            sleep(1);
        i--;
    } while (pid < 0 && i > 0);
    myassert(pid >= 0);
    
    if (pid == 0)
        exec_on_slave(sfd, slistener); /* Child. */

    /* Father. */

    CHILD_COUNT++;

    myassert_sys(close(sfd) == 0);
    myassert_sys(close(slistener) == 0);

    /* Pass configuration. */
    myassert_sys(write(mfd, &GLOBAL_CONF, sizeof(GLOBAL_CONF)) ==
                 sizeof(GLOBAL_CONF));

    /* Monitor requests to send signals. */
    evchild = malloc(sizeof(struct event));
    myassert(evchild);
    event_set(evchild, mfd, EV_READ | EV_PERSIST, child_read, evchild);
    event_add(evchild, NULL);
}

static int process_slave_exit(pid_t pid, int status)
{
    if (DEBUG_LEVEL & BUG_CHILD_END)
        mysyslog(LOG_NOTICE, "CHILDREN ended, pid=%i, status=%i", pid, status);
                
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status) {
            if (DEBUG_LEVEL & BUG_CHILD_DIE) {
                mysyslog(LOG_ERR, "CHILDREN unexpected DEATH, pid=%i, "
                         "exit_status=%i", pid, exit_status);
            }
            create_a_slave();
            return 1;
        }
        return 0;
    }
    
    myassert(WIFSIGNALED(status));
    mysyslog(LOG_ERR, "CHILDREN unexpected DEATH by signal, pid=%i, signal=%i",
             pid, WTERMSIG(status));
    create_a_slave();
    return 1;
}

static void sigchld_cb(int fd, short event, void *arg)
{
    pid_t pid;
    do {
        int status;
        pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0)
            if (!process_slave_exit(pid, status))
                CHILD_COUNT--;
    } while (pid > 0);
    myassert_sys(pid == 0 || errno == ECHILD);
    myassert(CHILD_COUNT >= 0);

    if (CHILD_COUNT > 0)
        return;

    if (FINISHING) {
        event_del((struct event *)arg);
        myassert(FINISHING == 1);
        return;
    }
    
    mysyslog(LOG_ALERT, "Something unexpected happened. "
             "I am not finishing, but I am without children. "
             "I go to initiate a son, but this problem must be investigated!");
    create_a_slave();
}

static void sigterm_cb(int fd, short event, void *arg)
{
    FINISHING++;
    unlink(GLOBAL_CONF.master.pid_file);
    myassert_sys(kill(0, SIGTERM) == 0);
}

static int create_pid_file(char *filename)
{
    FILE *file;
    struct stat st;

    if (lstat(filename, &st) == 0) {
        pid_t pid;

        if (!S_ISREG(st.st_mode)) {
            mysyslog(LOG_ERR, "The pid file already exists and it is not a "
                     "regular file, filename=`%s'", filename);
            return 0;
        }

        file = fopen(filename, "r+");
        myassert(file);
        fscanf(file, "%i", &pid);
        fclose(file);

        if (kill(pid, 0) == 0) {
            mysyslog(LOG_ERR, "Already there is a watchcatd running, pid=%i, "
                     "filename=%s", pid, filename);
            return 0;
        }
    }
    
    file = fopen(filename, "w");
    myassert(file);
    fprintf(file, "%i\n", getpid());
    fclose(file);
    return 1;
}

static int work(int nodaemon)
{
    struct event signal_chld;
    struct event signal_term;
    int sock;
    struct sockaddr_un addr;
    char *dev;
    
    /* Global alias settings. */
    DEBUG_LEVEL = GLOBAL_CONF.master.debug_level;

    /* Protections. */
    protect_process_group(nodaemon, GLOBAL_CONF.priority);
    protect_daemon(GLOBAL_CONF.realtime_mode);

    if (!create_pid_file(GLOBAL_CONF.master.pid_file))
        return 1;

    /* Listener. */
    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    myassert(sock >= 0);
    dev = GLOBAL_CONF.slave.device;
    unlink(dev);
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, dev, sizeof(addr.sun_path)-1); 
    addr.sun_path[sizeof(addr.sun_path)-1] = '\0';
    myassert_sys(bind(sock, (struct sockaddr *)&addr, SUN_LEN(&addr)) == 0);
    myassert_sys(listen(sock, 5) == 0);
    myassert_sys(chmod(dev, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
                 S_IWOTH) == 0);
    myassert_sys(fcntl(sock, F_SETFD, FD_CLOEXEC) == 0);
    LISTENER = sock;

    /* Events. */
    event_init();
    event_set(&signal_chld, SIGCHLD, EV_SIGNAL | EV_PERSIST, sigchld_cb,
              &signal_chld);
    event_add(&signal_chld, NULL);
    event_set(&signal_term, SIGTERM, EV_SIGNAL, sigterm_cb, &signal_term);
    event_add(&signal_term, NULL);
    create_a_slave();
    mysyslog(LOG_INFO, "watchcatd started");
    event_dispatch();
    mysyslog(LOG_INFO, "watchcatd finished");

    /* The kernel will make all housekeeping. */
    return 0;
}

/*** Before of change to daemon mode ***/

static void usage(FILE *fp)
{
    fprintf(fp,
            "Usage: watchcatd [options]\n"
            "Options:\n"
            "  -h, --help             Show this message.\n"
            "  -f, --config-file      Choose an alternate configuration file.\n"
    );
}

static void myclosefrom(int from)
{
    struct rlimit rl;
    int i;

    /* The code below is not perfect, but it is well reasonable.
     * Sample problem: dup2(0, rl.rlim_max + 1000);
     */
    assert(getrlimit(RLIMIT_NOFILE, &rl) == 0);
    for (i = from; i < rl.rlim_max; i++)
        close(i);
}

int main(int argc, char **argv)
{
    int opt;
    char config_file[256] = DEFAULT_CONFIG_FILE;
    char *optstr = "f:hp:X";
    int nodaemon = 0;
    int filedes[2];

    struct option longopts[] = {
        {"config-file", required_argument, NULL, 'f'},
        {"help",        no_argument,       NULL, 'h'},
        {"exec-path",   required_argument, NULL, 'p'},
        {"dont-detach", no_argument,       NULL, 'X'},
    };

    myclosefrom(3);
    
    if (geteuid()) {
        fprintf(stderr, "Only root can start this service\n");
        return 1;
    }
    
    /* Reserve fd positions. */
    myassert_sys(pipe(filedes) == 0);
    myassert(filedes[0] == FROM_SLAVE_TO_MASTER_FD);
    myassert_sys(fcntl(filedes[0], F_SETFD, FD_CLOEXEC) == 0);
    myassert(filedes[1] == SOCKET_LISTENER_FD);
    myassert_sys(fcntl(filedes[1], F_SETFD, FD_CLOEXEC) == 0);

    while ((opt = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
        switch(opt) {
        case 'f':
            assert(strlen(optarg) < sizeof(config_file));
            strcpy(config_file, optarg);
            break;
        case 'h':
            usage(stdout);
            return 0;
        case 'p':
            assert(strlen(optarg) < sizeof(EXEC_SLAVE));
            strcpy(EXEC_SLAVE, optarg);
            break;
        case 'X':
            nodaemon = 1;
            break;
        case '?':
            usage(stderr);
            return 1;
        default:
            fprintf(stderr, "watchcatd: error handling command line flags.\n");
            return 1;
        }
    }
    load_conf(config_file);
    assert(strlen(EXEC_SLAVE) + strlen("/catslave") < sizeof(EXEC_SLAVE));
    strcat(EXEC_SLAVE, "/catslave");
    return work(nodaemon);
}

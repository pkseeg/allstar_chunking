/*
** $Id: slave.c 2829 2009-02-13 20:55:03Z andre.dig $
** watchcatd - Watchcat Daemon
** See copyright notice in distro's COPYRIGHT file
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <syslog.h>
#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>
#include <sys/resource.h>

#include "slave.h"
#include "master.h"
#include "utils.h"
#include "watchcatd.h"

/* WARNING TO THE CODE WRITERS
 * Subtle protection
 *
 * The watchcat slave do just a read per fd returned by select. In addition,
 * the read is done with a small buffer. Don't forget this when editing the
 * code.
 * 
 * Without this protection, a client can send much data to a cat and harm
 * the "timeout chronometer"'s precision.
 */
 
/* DEBUG_LEVEL
 * Bit  Description             Volume
 * 0    Create / finish a cat.  high
 * 1    Parser header.          high
 * 2    Body.                   high
 * 3    Timeout.                low
 * 4    Security.               low
 * 5    BIT FREE.               -
 * 6    Critical fail.          low
 * 7    Warnings.               low/moderate
 * 8    Connection.             low/moderate
 * 9    Connection OK.          high
 * 10   Security fire.          high
 */

#define BUG_CREATE_DROP     1
#define BUG_PARSER          2
#define BUG_BODY            4
#define BUG_TIMEOUT         8
#define BUG_SECURITY        16
#define BUG_CRITICAL_FAIL   64
#define BUG_WARNING         128
#define BUG_CONNECTION      256
#define BUG_CONNECTION_OK   512
#define BUG_SECURITY_FIRE   1024

/* Alias for configuration items very used and miscellaneous.
 * These values need to be treated as constants!
 */
static int DEBUG_LEVEL =    2047;
static int CAT_MAXCAT  =    1;

/* (struct cat).status values */
#define CAT_PARSING     0
#define CAT_WORKING     1

#define CMD_NIL         0
#define CMD_STAT        1

struct cat {
    LIST_ENTRY(cat) cats;

    int status;

    int fd;             /* Cat fd. */
    
    char *buf;          /* Buffer for header parser. */
    int pos;            /* End of buf. */
    
    int version;        /* Protocol version. */
    int timeout;        /* Timeout in seconds. */
    pid_t pid;          /* Client's PID. */
    uid_t uid;          /* Client's UID. */
    int signal;         /* Signal to send on timeout. */
    char info[128];     /* A string to insert in log on timeout. */
    int cmd;            /* Command. */

    struct event event; /* libevent's struct for event handler. */
};

struct stats {
    unsigned int total_cat;    /* Number of cats created so far. */
    unsigned int total_kill;   /* Number of kills. */
};

/* Globals */
static LIST_HEAD(catlisthead, cat) CATS;
static int CAT_COUNT;
static struct stats STATS;

/* NOTE, only assert can be used in write_to_master! */
void write_to_master(const void *buf, size_t count)
{
    size_t r;
    do {
        r = write(FROM_SLAVE_TO_MASTER_FD, buf, count);
    } while(r == -1 && errno == EINTR);
    assert(r > 0);
    assert(r == count);
}

/* NOTE, only assert can be used in mysyslog! */
void mysyslog(int priority, const char *format, ...)
{
    char buf[128];
    const int size = sizeof(buf);
    const int type = MSG_LOG;
    struct log_msg msg;
    va_list ap;
    int len;

    va_start(ap, format);
    len = vsnprintf(buf, size, format, ap);
    va_end(ap);
    assert(len > 0);
    len = (len < size) ? len : (size - 1);
    msg.priority = priority;
    msg.length = len;

    write_to_master(&type, sizeof(type));
    write_to_master(&msg, sizeof(msg));
    write_to_master(buf, len);
}

void mywrite(int fd, const void *buf, size_t count)
{
    ssize_t r;
    do {
        r = write(fd, buf, count);
    } while(r == -1 && errno == EINTR);
    
    /* Debug purpose. DON'T enable in production environment! */
#if 0
    myassert_sys(r > 0);
    myassert(r == count);
#endif
}

static void myclose(int fd)
{
    int r = close(fd);
    if (r && DEBUG_LEVEL & BUG_WARNING)
        mysyslog(LOG_WARNING, "close FAIL, fd=%i, errno=%i, strerror=`%s'",
                 fd, errno, strerror(errno));
}

#define BUFSIZE 128

/* The msg parameter is only returned to client, before close fd, when
 * cat->status == CAT_PARSING.
 */
static void cat_finish(struct cat *cat, char *msg)
{
    int fd;
    myassert(cat->status == CAT_PARSING || cat->status == CAT_WORKING);
    fd = cat->fd;
    
    if (cat->status == CAT_PARSING) {
        char *buf = cat->buf;
        int count = snprintf(buf, BUFSIZE, "%s\n", msg);
        myassert(count > 0 && count < BUFSIZE);
        mywrite(fd, buf, count);
        free(buf);
        cat->buf = NULL;
    }

    CAT_COUNT--;
    myclose(fd);

    LIST_REMOVE(cat, cats);
    free(cat);

    if (DEBUG_LEVEL & BUG_CREATE_DROP)
        mysyslog(LOG_INFO, "DROP fd=%i, CAT_COUNT=%i, msg=`%s'", fd, CAT_COUNT,
                 msg);
}

static void cat_ev_header_read(int fd, short event, void *arg);
static void cat_ev_body_read(int fd, short event, void *arg);

static struct cat *cat_create(int fd, pid_t pid, uid_t uid)
{
    struct cat *cat;

    cat = calloc(1, sizeof(struct cat));
    if (cat == NULL) {
        if (DEBUG_LEVEL & BUG_CRITICAL_FAIL)
            mysyslog(LOG_CRIT, "FAIL out of memory, fd=%i", fd);
        return NULL;
    }

    cat->buf = malloc(BUFSIZE);
    if (cat->buf == NULL) {
        if (DEBUG_LEVEL & BUG_CRITICAL_FAIL)
            mysyslog(LOG_CRIT, "FAIL out of memory, fd=%i, pid=%i", fd, pid);
        free(cat);
        return NULL;
    }

    LIST_INSERT_HEAD(&CATS, cat, cats);
    cat->status = CAT_PARSING;
    cat->fd = fd;
    cat->timeout = 60;
    cat->pid = pid;
    cat->uid = uid;
    cat->signal = SIGKILL;
    cat->info[0] = '\0';
    cat->cmd = CMD_NIL;
    CAT_COUNT++;

    if ((DEBUG_LEVEL & BUG_SECURITY) &&
        STATS.total_cat == (unsigned int) -1)
        mysyslog(LOG_INFO, "TOTAL_CAT limit reached, count restarted");
    STATS.total_cat++;

    if (DEBUG_LEVEL & BUG_CREATE_DROP)
        mysyslog(LOG_INFO, "CREATE fd=%i, CAT_COUNT=%i", fd, CAT_COUNT);
    return cat;
}

static void command_stat(struct cat *cat)
{
    struct cat *curr;
    int fd = cat->fd;
    char buf[128];
    const int size = sizeof(buf);
    int buf_len;
    
    buf_len = snprintf(buf, size, "total_cat: %u\ntotal_kill: %u\n",
                       STATS.total_cat, STATS.total_kill);
    mywrite(fd, buf, buf_len);

    for (curr = CATS.lh_first; curr != NULL; curr = curr->cats.le_next) {
        buf_len = snprintf(buf, size,
                           "block:\nstatus: %i\nfd: %i\nversion: %i\n"
                           "timeout: %i\npid: %i\nuid: %i\n",
                           curr->status, curr->fd, curr->version, curr->timeout,
                           curr->pid, curr->uid);
        mywrite(fd, buf, buf_len);
        
        buf_len = snprintf(buf, size, "signal: %i\ninfo: ",
                           curr->signal);
        mywrite(fd, buf, buf_len);

        mywrite(fd, curr->info, strlen(curr->info));
        mywrite(fd, "\n", 1);
    }

    mywrite(fd, "\n", 1);
}

static char *clean_in_place(char *s)
{
    char *ns = s;
    char *end;
    myassert(s);
    if (!*s) return s;
    while (isspace(*ns)) ns++;
    end = ns;
    while (*end) end++;
    end--;
    while (isspace(*end)) end--;
    *(end + 1) = '\0';
    return ns;
}

/* ATENTION: the strings field_name and field_value can to be modified! */
static int cat_eval_field(struct cat *cat, char *field_name, char *field_value)
{
    int fd = cat->fd;
    field_name = clean_in_place(field_name);
    field_value = clean_in_place(field_value);

    if (strcmp(field_name, "version") == 0) {
        cat->version = atoi(field_value);
    } else if (strcmp(field_name, "timeout") == 0) {
        cat->timeout = atoi(field_value);
    } else if (strcmp(field_name, "signal") == 0) {
        cat->signal = atoi(field_value);
    } else if (strcmp(field_name, "info") == 0) {
        char *p = cat->info;
        strncpy(p, field_value, sizeof(cat->info)-1);
        p[sizeof(cat->info)-1] = '\0';
        for (; *p; p++)
            if (!isprint(*p))
                *p = '_';
    } else if (strcmp(field_name, "command") == 0) {
        if (cat->uid) {
            if (DEBUG_LEVEL & BUG_SECURITY)
                mysyslog(LOG_ERR, "SECURITY command field requires super user, "
                         "pid=%i, uid=%i", cat->pid, cat->uid);
            return 1;
        }

        if (strcmp(field_value, "stat") == 0) {
            cat->cmd = CMD_STAT;
        } else {
            if (DEBUG_LEVEL & BUG_PARSER)
                mysyslog(LOG_ERR, "PARSER command=`%s' unknown, fd=%i",
                         field_value, fd);
            return 1;
        }
    } else {
        if (DEBUG_LEVEL & BUG_PARSER)
            mysyslog(LOG_ERR, "PARSER field_name=`%s' unknown, fd=%i",
                     field_name, fd);
        return 1;
    }
    if (DEBUG_LEVEL & BUG_PARSER)
        mysyslog(LOG_INFO, "PARSER fd=%i, field_name=`%s' field_value=`%s'",
                 fd, field_name, field_value);
    return 0;
}

static int cat_process_header(struct cat *cat)
{
    int version = cat->version;
    int timeout = cat->timeout;
    int signal = cat->signal;
    pid_t pid = cat->pid;
    uid_t uid = cat->uid;
    int fd = cat->fd;
    const char *info = cat->info;
    int cmd = cat->cmd;

    myassert(cat->status == CAT_PARSING);

    if (cmd == CMD_NIL) {
        struct event *ev;
        struct timeval tm;
        
        if (version != 1 || timeout < 5 || signal < 1 || signal > 64 ||
            pid <= 1) {
            if (DEBUG_LEVEL & BUG_PARSER)
                mysyslog(LOG_ERR, "PARSER header FAIL, fd=%i, version=%i, "
                         "timeout=%i, pid=%i, uid=%i, signal=%i, info=`%s'",
                         fd, version, timeout, pid, uid, signal, info);
            return 1;
        }

        ev = &cat->event;
        tm.tv_sec = timeout;
        tm.tv_usec = 0;
        event_del(ev);
        event_set(ev, fd, EV_READ, cat_ev_body_read, cat);
        event_add(ev, &tm);
        mywrite(fd, "ok\n", 3);
    } else {
        myassert(cat->cmd == CMD_STAT);
    }
    cat->status = CAT_WORKING;
    free(cat->buf);
    cat->buf = NULL;

    if (DEBUG_LEVEL & BUG_PARSER)
        mysyslog(LOG_INFO, "PARSER header OK, fd=%i, version=%i, "
                 "timeout=%i, pid=%i, uid=%i, signal=%i, info=`%s'",
                 fd, version, timeout, pid, uid, signal, info);
    return 0;
}

/* Returns zero on success and 1 on fail. */
static int cat_parser(struct cat *cat)
{
    int fd = cat->fd;
    char *buf = cat->buf;
    int pos = cat->pos;
    char *nl_start;
    char *fld_start;
    int len;
    int count;

    myassert(cat->status == CAT_PARSING);
    len = BUFSIZE - pos - 1;
    count = read(fd, buf + pos, len);
    if (count < 0) {
        if (DEBUG_LEVEL & BUG_PARSER)
            mysyslog(LOG_ERR, "PARSER read FAIL, fd=%i, "
                     "result=%i, errno=%i, strerror=`%s'",
                     fd, count, errno, strerror(errno));
        return 1;
    }
    if (count == 0) {
        if (DEBUG_LEVEL & BUG_PARSER)
            mysyslog(LOG_ERR, "PARSER socket closed, fd=%i", fd);
        return 1;
    }
    buf[pos + count] = '\0';
    
    /* Eliminate big lines. */
    nl_start = strchr(buf, '\n');
    if (nl_start == NULL) {
        if (count < len) {
            cat->pos += count;
            return 0;
        }
        if (DEBUG_LEVEL & BUG_PARSER)
            mysyslog(LOG_ERR, "PARSER big line, fd=%i, line=`%s'", fd, buf);
        return 1;
    }

    /* Parser fields. */
    fld_start = buf;
    while (nl_start) {
        char *colon_start;
        if (fld_start == nl_start) {
            /* End of header. */
            return cat_process_header(cat);
        }
        colon_start = strchr(fld_start, ':');
        if (colon_start == NULL) {
            if (DEBUG_LEVEL & BUG_PARSER)
                mysyslog(LOG_ERR, "PARSER invalid line, fd=%i", fd);
            return 1;
        }
        *colon_start = '\0';
        *nl_start = '\0';
        if (cat_eval_field(cat, fld_start, colon_start + 1))
            return 1;
        fld_start = nl_start + 1;
        nl_start = strchr(fld_start, '\n');
    }
    cat->pos = pos + count - (fld_start - buf) / sizeof(char);
    memmove(buf, fld_start, cat->pos);
    return 0;
}

static void cat_ev_header_read(int fd, short event, void *arg)
{
    struct cat *cat = arg;
    myassert(cat->status == CAT_PARSING);

    if (event & EV_READ) {
        if (cat_parser(cat)) {
            event_del(&cat->event);
            cat_finish(cat, "error");
            return;
        }
        
        if (cat->status == CAT_WORKING) {
            if (cat->cmd == CMD_STAT) {
                command_stat(cat);
                event_del(&cat->event);
                cat_finish(cat, "stat");
            }
            return;
        }

        if (!(event & EV_TIMEOUT)) {
            return;
        }
    }

    /* Timeout on header parsing. */
    myassert(event & EV_TIMEOUT);
    if (DEBUG_LEVEL & BUG_TIMEOUT)
        mysyslog(LOG_ERR, "TIMEOUT on header parsing, pid=%i", cat->pid);
    cat_finish(cat, "timeout");
}

static void mykill(int fd, pid_t pid, int sig)
{
    struct kill_msg msg;
    const int type = MSG_KILL;
    msg.fd = fd;
    msg.pid = pid;
    msg.signal = sig;
    write_to_master(&type, sizeof(type));
    write_to_master(&msg, sizeof(msg));
}

static int cat_ev_body_read_with_ret(int fd, short event, void *arg)
{
    struct cat *cat = arg;
    myassert(cat->status == CAT_WORKING);

    if (event & EV_READ) {
        char buf[BUFSIZE];
        int count = read(fd, buf, sizeof(buf));
        if (count > 0) {
            struct timeval tm;
            if (DEBUG_LEVEL & BUG_BODY)
                mysyslog(LOG_INFO, "BODY heart beat, fd=%i", fd);
            tm.tv_sec = cat->timeout;
            tm.tv_usec = 0;
            event_add(&cat->event, &tm);
            return 0;
        }
        if (count < 0) {
            if (DEBUG_LEVEL & BUG_BODY)
                mysyslog(LOG_ERR, "BODY read FAIL, fd=%i, result=%i, errno=%i, "
                         "strerror=`%s'", fd, count, errno, strerror(errno));
            cat_finish(cat, "end_error");
            return 1;
        }
        cat_finish(cat, "end");
        return 1;
    }

    /* Cat's timeout. */
    myassert(event & EV_TIMEOUT);
    if (DEBUG_LEVEL & BUG_TIMEOUT)
        mysyslog(LOG_ERR, "TIMEOUT, pid=%i, uid=%i, signal=%i, timeout=%i, "
                 "info=`%s'", cat->pid, cat->uid, cat->signal, cat->timeout,
                 cat->info);
    mykill(fd, cat->pid, cat->signal);
    cat_finish(cat, "timeout");
    
    if ((DEBUG_LEVEL & BUG_SECURITY) &&
        STATS.total_kill == (unsigned int) -1)
        mysyslog(LOG_INFO, "TOTAL_KILL limit reached, count restarted");
    STATS.total_kill++;

    return 1;
}

static void cat_ev_body_read(int fd, short event, void *arg)
{
    cat_ev_body_read_with_ret(fd, event, arg);
}

static int cat_conn_per_pid_limit(int conn, pid_t pid, int limit)
{
    int count = CAT_COUNT;
    int can = limit;
    struct cat *cat;

    myassert(limit >= 0);

    if (limit == 0 || can > count)
        return 0;

    for (cat = CATS.lh_first; cat != NULL && can <= count;
         cat = cat->cats.le_next) {
        if (cat->pid == pid) {
            struct event *ev = &cat->event;
            if (ev->ev_flags & EVLIST_ACTIVE) {
                event_del(ev);
                if (cat_ev_body_read_with_ret(ev->ev_fd, ev->ev_res,
                    ev->ev_arg))
                    return 0;
            }
            can--;
            if (can <= 0)
                break;
        }
        count--;
    }
    return can < 1;
}

static pid_t get_pid_from_fd(int fd, uid_t *uid)
{
#if defined(SO_PEERCRED) /* Tested on Linux */
    struct ucred crd;
    socklen_t cl = sizeof(crd);

    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &crd, &cl)) {
        if (DEBUG_LEVEL & BUG_SECURITY)
            mysyslog(LOG_ERR, "SECURITY get credentials FAILED, fd=%i, "
                     "errno=%i, strerror=`%s'", fd, errno, strerror(errno));
        return -1;
    }
    if (crd.pid > 1) {
        if (DEBUG_LEVEL & BUG_SECURITY_FIRE)
            mysyslog(LOG_INFO, "SECURITY fd=%i, peer's data: pid=%i, uid=%i, "
                     "gid=%i", fd, crd.pid, crd.uid, crd.gid);
        if (uid) *uid = crd.uid;
        return crd.pid;
    }
    if (DEBUG_LEVEL & BUG_SECURITY)
        mysyslog(LOG_ERR, "SECURITY invalid pid, fd=%i, pid=%i, uid=%i, gid=%i",
                 fd, crd.pid, crd.uid, crd.gid);
    return -1;

#elif defined(SCM_CREDS) /* Tested on FreeBSD */
    char buf;
    ssize_t r;
    struct msghdr msg;
    struct iovec iov;
    struct cmsgcred *crd;
    union {
        struct cmsghdr hdr; /* for alignment */
        char crd[CMSG_SPACE(sizeof(struct cmsgcred))];
    } cmsg;

    iov.iov_base = &buf;
    iov.iov_len = 1;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    memset(&cmsg, 0, sizeof(cmsg));
    msg.msg_control = (caddr_t)&cmsg;
    msg.msg_controllen = CMSG_SPACE(sizeof(struct cmsgcred));

    do {
        r = recvmsg(fd, &msg, 0);
    } while(r == -1 && errno == EINTR);

    myassert(cmsg.hdr.cmsg_len == CMSG_LEN(sizeof(struct cmsgcred)));
    myassert(cmsg.hdr.cmsg_type == SCM_CREDS);

    crd = (struct cmsgcred *)CMSG_DATA(&cmsg.hdr);
    if (crd->cmcred_pid > 1) {
        if (DEBUG_LEVEL & BUG_SECURITY_FIRE)
            mysyslog(LOG_INFO, "SECURITY fd=%i, peer's data: pid=%i, uid=%i, "
                     "gid=%i", fd, crd->cmcred_pid, crd->cmcred_uid,
                     crd->cmcred_gid);
        if (uid) *uid = crd->cmcred_uid;
        return crd->cmcred_pid;
    }
    if (DEBUG_LEVEL & BUG_SECURITY)
        mysyslog(LOG_ERR, "SECURITY invalid pid, fd=%i, pid=%i, uid=%i, gid=%i",
                 fd, crd->cmcred_pid, crd->cmcred_uid, crd->cmcred_gid);
#endif

    return -1;
}

static void listen_read(int fd, short event, void *arg)
{
    /* New connection. */

    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);
    int conn = accept(fd, (struct sockaddr *)&addr, &addr_len);
    pid_t pid;
    uid_t uid;
    struct cat *cat;
    myassert(conn >= 0);
    myassert_sys(fcntl(conn, F_SETFD, FD_CLOEXEC) == 0);

    if (CAT_COUNT >= CAT_MAXCAT) {
        myassert(CAT_COUNT == CAT_MAXCAT);
        if (DEBUG_LEVEL & BUG_CONNECTION)
            mysyslog(LOG_ERR, "CONNECT FAIL, too many conections, "
                     "CAT_MAXCAT=%i", CAT_MAXCAT);
    }
    else if ((pid = get_pid_from_fd(conn, &uid)) < 0) {
        if (DEBUG_LEVEL & BUG_CONNECTION)
            mysyslog(LOG_ERR, "CONNECT FAIL, pid for conn=%i unknown", conn);
    }
    else if (cat_conn_per_pid_limit(conn, pid,
             GLOBAL_CONF.slave.max_conections_per_pid)) {
        if (DEBUG_LEVEL & BUG_CONNECTION)
            mysyslog(LOG_ERR, "CONNECT FAIL, max_conections_per_pid=%i, "
                     "pid=%i, uid=%i", GLOBAL_CONF.slave.max_conections_per_pid,
                     pid, uid);
    }
    else if ((cat = cat_create(conn, pid, uid)) == NULL) {
        if (DEBUG_LEVEL & BUG_CONNECTION)
            mysyslog(LOG_ERR, "CONNECT FAIL, cat_create returned error, "
                     "conn=%i, pid=%i, uid=%i", conn, pid, uid);
    }
    else {
        struct event *ev;
        struct timeval tm = {5, 0};

        myassert(cat);
        ev = &cat->event;
        event_set(ev, conn, EV_READ | EV_PERSIST, cat_ev_header_read, cat);
        event_add(ev, &tm);

        if (DEBUG_LEVEL & BUG_CONNECTION_OK)
            mysyslog(LOG_INFO, "CONNECT OK, conn=%i", conn);
        return;
    }

    /* Fail. */
    mywrite(conn, "connect\n", 8);
    myclose(conn);
}

static void sigterm_cb(int fd, short event, void *arg)
{
    exit(0);
}

static void work(void)
{
    struct event ev_listen_read;
    struct event ev_signal_term;

    event_init();

    event_set(&ev_listen_read, SOCKET_LISTENER_FD, EV_READ | EV_PERSIST,
              listen_read, &ev_listen_read);
    event_add(&ev_listen_read, NULL);
    event_set(&ev_signal_term, SIGTERM, EV_SIGNAL, sigterm_cb, &ev_signal_term);
    event_add(&ev_signal_term, NULL);

    event_dispatch();
}

int main(void)
{
    struct rlimit rlim;

    /* Close-on-exec. */
    myassert_sys(fcntl(FROM_SLAVE_TO_MASTER_FD, F_SETFD, FD_CLOEXEC) == 0);
    myassert_sys(fcntl(SOCKET_LISTENER_FD, F_SETFD, FD_CLOEXEC) == 0);

    /* Get GLOBAL_CONF. */
    myassert_sys(read(FROM_SLAVE_TO_MASTER_FD, &GLOBAL_CONF,
                 sizeof(GLOBAL_CONF)) == sizeof(GLOBAL_CONF));
    
    DEBUG_LEVEL = GLOBAL_CONF.slave.debug_level;
    CAT_MAXCAT  = GLOBAL_CONF.slave.max_conections;
    LIST_INIT(&CATS);

    /* RLIMIT restrictions. */
    myassert_sys(getrlimit(RLIMIT_NOFILE, &rlim) == 0);
    if (rlim.rlim_cur < GLOBAL_CONF.slave.max_conections + 10) {
        /*
         * Set new file descriptor limits:
         *  - std{in,out,err};
         *  - parent<->child socket;
         *  - listen socket;
         *  - five eventual fds (just do be safe).
         */
        rlim.rlim_cur = GLOBAL_CONF.slave.max_conections + 10;

        if (rlim.rlim_cur > rlim.rlim_max) {
            rlim.rlim_max = rlim.rlim_cur;
            mysyslog(LOG_WARNING, "Changing file descriptor hard limit to %i",
                     (int)rlim.rlim_max);
        }

        myassert_sys(setrlimit(RLIMIT_NOFILE, &rlim) == 0);
    }

    protect_daemon(GLOBAL_CONF.realtime_mode);
    drop_privileges(GLOBAL_CONF.user);

    work();

    /* The kernel will make all housekeeping. */
    return 0;
}

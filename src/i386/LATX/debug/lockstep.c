/*
 * Lockstep Execution
 *
 * Allows you to execute two QEMU instances in lockstep and report
 * when their execution diverges. This is mainly useful for developers
 * who want to see where a change to code generation has
 * introduced a subtle and hard to find bug.
 *
 * Caveats:
 *   - single-threaded linux-user apps only with non-deterministic syscalls
 *   - no MTTCG enabled system emulation (icount may help)
 *
 * While icount makes things more deterministic it doesn't mean a
 * particular run may execute the exact same sequence of blocks. An
 * asynchronous event (for example X11 graphics update) may cause a
 * block to end early and a new partial block to start. This means
 * serial only test cases are a better bet. -d nochain may also help.
 *
 * This code is not thread safe!
 *
 * !! This code is translanted from original QEMU 
 *    commit:c81950a2f1923dec3f6b952ec6bb9b921be58a70
 *    For our XQM, please use lockstep with icount.
 */

#include <inttypes.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include "common.h"
#include "lockstep.h"

/* The execution state we compare 
 * More state information can be added here
 */
typedef struct {
    target_ulong pc;
} ExecState;

static int socket_fd;

static void lockstep_cleanup(void)
{
    close(socket_fd);
}

static void report_divergance(ExecState* us, ExecState* them)
{
  fprintf(stderr, "\n us @ 0x%x vs them 0x%x\n",
	       us->pc, them->pc);
  lockstep_cleanup();
  exit(0);
}

void vcpu_tb_exec(CPUX86State *env)
{
    ExecState us, them;
    ssize_t bytes;

    us.pc = env->eip;

    /*
     * Write our current position to the other end. If we fail the
     * other end has probably died and we should shut down gracefully.
     */
    bytes = write(socket_fd, &us, sizeof(target_ulong));
    if (bytes < sizeof(target_ulong)) {
        printf(bytes < 0 ?
               "problem writing to socket" :
               "wrote less than expected to socket");
        lockstep_cleanup();
        return;
    }
    
    /*
     * Now read where our peer has reached. Again a failure probably
     * indicates the other end died and we should close down cleanly.
     */
    bytes = read(socket_fd, &them, sizeof(target_ulong));
    if (bytes < sizeof(target_ulong)) {
        printf(bytes < 0 ?
               "problem reading from socket" :
               "read less than expected");
        lockstep_cleanup();
        return;
    }

    /*
     * Compare and report if we have diverged.
     */
    if (us.pc != them.pc) {
        report_divergance(&us, &them);
    }
}

/*
 * Instead of encoding master/slave status into what is essentially
 * two peers we shall just take the simple approach of checking for
 * the existence of the pipe and assuming if it's not there we are the
 * first process.
 */
static bool setup_socket(const char *path)
{
    struct sockaddr_un sockaddr;
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("create socket");
        return false;
    }

    sockaddr.sun_family = AF_UNIX;
    g_strlcpy(sockaddr.sun_path, path, sizeof(sockaddr.sun_path) - 1);
    if (bind(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
        perror("bind socket");
        close(fd);
        return false;
    }

    if (listen(fd, 1) < 0) {
        perror("listen socket");
        close(fd);
        return false;
    }

    socket_fd = accept(fd, NULL, NULL);
    if (socket_fd < 0 && errno != EINTR) {
        perror("accept socket");
        return false;
    }

    printf("setup_socket::ready\n");

    return true;
}

static bool connect_socket(const char *path)
{
    int fd;
    struct sockaddr_un sockaddr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("create socket");
        return false;
    }

    sockaddr.sun_family = AF_UNIX;
    g_strlcpy(sockaddr.sun_path, path, sizeof(sockaddr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
        perror("failed to connect");
        return false;
    }

    printf("connect_socket::ready\n");

    socket_fd = fd;
    return true;
}

static bool setup_unix_socket(const char *path)
{
    if (g_file_test(path, G_FILE_TEST_EXISTS)) {
        return connect_socket(path);
    } else {
        return setup_socket(path);
    }
}


void lockstep_init(void)
{
    const char path[20] = "lockstep-i386.sock";
    if (!setup_unix_socket(path)) {
       lsassertm(0, "Failed to setup socket for communications.");
    }
}

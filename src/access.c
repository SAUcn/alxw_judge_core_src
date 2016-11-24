#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include <string.h>

int checkAccess(int pid, struct user_regs_struct *regs) {
    if (!calls[REG_SYS_CALL(regs)])
    {
        printf("Illegal syscall: %d\n", REG_SYS_CALL(regs));
        return ACCESS_CALL_ERR;
    }
    if (REG_SYS_CALL(regs) == SYS_open)
    {
        printf("Try to open file!\n");
        return ACCESS_FILE_ERR;
    }
    return ACCESS_OK;
}

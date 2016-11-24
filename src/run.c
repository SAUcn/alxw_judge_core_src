#include <string.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <fcntl.h>

#include "cjson/cJSON.h"

#include "proc.c"
#include "access.c"
#include "limit.c"

void error(const char *err)
{
    //TODO
}

int traceLoop(int java, int time_limit, int memory_limit, pid_t pid, struct Result *rst)
{
    long memory;
    int status, incall = 0;
    struct rusage ru;
    struct user_regs_struct regs;
    
    rst->memory_used = get_proc_status(pid, "VmRSS:");

    rst->judge_result = AC;

    while (1)
    {
        if (wait4(pid, &status, 0, &ru) == -1)
            error("wait4 [WSTOPPED] failure");

        //Get memory
        if (java)
            memory = get_page_fault_mem(ru, pid);
        else
            memory = get_proc_status(pid, "VmPeak:");
        if (memory > rst->memory_used)
            rst->memory_used = memory;

        //Check mempry
        if (rst->memory_used > memory_limit)
        {
            rst->judge_result = MLE;
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }

        //If exited 
        if (WIFEXITED(status))
            break;
    
        //Get exitcode
        int exitcode = WEXITSTATUS(status);
        /**
            exitcode == 5 waiting for next CPU allocation
            ruby using system to run, exit 17 ok
        **/
        if (java || exitcode == 0x05 || exitcode == 0)
            ;
        else {
            if (rst->judge_result == AC)
            {
                switch (exitcode)
                {
                    case SIGCHLD:
                    case SIGALRM:
                        rst->judge_result = RE;
                    case SIGKILL:
                    case SIGXCPU:
                        rst->judge_result = TLE;
                        break;
                    case SIGXFSZ:
                        rst->judge_result = OLE;
                        break;
                    default:
                        rst->judge_result = RE;
                }
                rst->re_signum = WTERMSIG(status);
            }
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }

        /*  WIFSIGNALED: if the process is terminated by signal */
        if (WIFSIGNALED(status))
        {
            int sig = WTERMSIG(status);
            if (rst->judge_result == AC)
            {
                switch (sig)
                {
                    case SIGCHLD:
                    case SIGALRM:
                        rst->judge_result = RE;
                    case SIGKILL:
                    case SIGXCPU:
                        rst->judge_result = TLE;
                        break;
                    case SIGXFSZ:
                        rst->judge_result = OLE;
                        break;
                    default:
                        rst->judge_result = RE;
                }
                rst->re_signum = WTERMSIG(status);
            }
            break;
        }

        if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1)
            error("PTRACE_GETREGS failure");
        if (incall && !java)
        {
            int ret = checkAccess(pid, &regs);
            if (ret != ACCESS_OK)
            {
                ptrace(PTRACE_KILL, pid, NULL, NULL);
                waitpid(pid, NULL, 0);

                rst->judge_result = RE;
                if (ret == ACCESS_CALL_ERR) {
                    rst->re_call = REG_SYS_CALL(&regs);
                } else {
                    rst->re_file_flag = REG_ARG_2(&regs);
                }
                return 0;
            }
            incall = 0;
        } else
            incall = 1;
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

    }
	//Don't forget this !!!
	rst->time_used = ru.ru_utime.tv_sec * 1000
        + ru.ru_utime.tv_usec / 1000
        + ru.ru_stime.tv_sec * 1000
        + ru.ru_stime.tv_usec / 1000;

    //In case zombie process
    waitpid(pid, NULL, 0);

    return 0;
}

int run(const char * argv[], const char* fd_in, const char* fd_out, const char* fd_err, int java, int time, int memory, struct Result *rst)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        error("Fork error");
        return 1;
    }

    if (pid == 0)
    {
        freopen(fd_in, "r", stdin);
        freopen(fd_out, "w", stdout);
        freopen(fd_err, "w", stderr);
        setResLimit(time, memory, java);
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[0], (char * const *)argv);
    } else {
        int t = nice(19);
        t = traceLoop(java, time, memory, pid, rst);
        return t;
    }
}

char* runit(const char* json)
{
    cJSON *root = cJSON_Parse(json);
    char *args = cJSON_GetObjectItem(root, "args")->valuestring;
    char *argv[20];
    char *split = " ";
    int p = 0;
    argv[p++] = strtok(args, split);
    while (argv[p++] = strtok(NULL, split));
    argv[p++] = NULL;

    char *fd_in = cJSON_GetObjectItem(root, "fd_in")->valuestring;
    char *fd_out = cJSON_GetObjectItem(root, "fd_out")->valuestring;
    char *fd_err = cJSON_GetObjectItem(root, "fd_err")->valuestring;
    int memory = cJSON_GetObjectItem(root, "memorylimit")->valueint;
    int time = cJSON_GetObjectItem(root, "timelimit")->valueint;
    int java = cJSON_GetObjectItem(root, "java")->valueint;

    struct Result rst;

    int ret = run((const char **)argv, fd_in, fd_out, fd_err, java, time, memory, &rst);

    cJSON *fmt = cJSON_CreateObject();
    cJSON_AddNumberToObject(fmt, "timeused", rst.time_used);
    cJSON_AddNumberToObject(fmt, "memoryused", rst.memory_used);
    cJSON_AddNumberToObject(fmt, "result", rst.judge_result);

    char * r = cJSON_Print(fmt);

    printf("Result:\n%s\n", r);

    return r;

}



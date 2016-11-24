#include <sys/syscall.h>

#ifdef __i386
    int SYS_CALLS[256] = {3, 4, 5, 6, 8, 11, 13, 33, 45, 85, 91, 122, 125, 140, 192, 197, 243, 252};
#else
    int SYS_CALLS[256] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 21, 59, 63, 89, 158, 201, 205, 231, 240, 252};
#endif

#define BUFF_SIZE 1024
#define CALLS_MAX 400

#define ACCESS_CALL_ERR 1
#define ACCESS_FILE_ERR 2
#define ACCESS_OK 0

#if __WORDSIZE == 64
    #define REG_SYS_CALL(x) ((x)->orig_rax)
    #define REG_ARG_1(x) ((x)->rdi)
    #define REG_ARG_2(x) ((x)->rsi)
#else
    #define REG_SYS_CALL(x) ((x)->orig_eax)
    #define REG_ARG_1(x) ((x)->ebx)
    #define REG_ARG_2(x) ((x)->ecx)
#endif

u_char *calls;

enum JUDGE_RESULT {
    AC = 0,   //0 Accepted
    PE,	    //1 Presentation Error
    TLE,	//2 Time Limit Exceeded
    MLE,	//3 Memory Limit Exceeded
    WA,	    //4 Wrong Answer
    RE,	    //5 Runtime Error
    OLE,	//6 Output Limit Exceeded
    CE,	    //7 Compile Error
    SE,     //8 System Error
};

struct Result {
    int judge_result; //JUDGE_RESULT
    int time_used, memory_used;
    int re_signum;
    int re_call;
    const char* re_file;
    int re_file_flag;
};

void initcall()
{
    int i = 0;
    calls = (u_char *)malloc(sizeof(sizeof(u_char) * CALLS_MAX));
    printf("SYSCALLS:");
    while (SYS_CALLS[i] || !i)
    {
        printf(" %d", SYS_CALLS[i]);
        calls[SYS_CALLS[i++]] = 1;
    }
    printf("\n");
}

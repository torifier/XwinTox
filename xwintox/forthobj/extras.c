#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#ifndef EMBEDDED
#include <termios.h>
#include <sys/select.h>
#include <sys/wait.h>
#endif

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "ficl.h"

#ifdef SOCKET
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#ifndef EMBEDDED
#include <dlfcn.h>
#include <sys/utsname.h>
#include <termios.h>
#endif

#if (defined(LINUX))
#include <sys/mman.h>
static int systemTick=0;
#endif

char           *strsave(char *);
extern int      errno;
#ifndef FICL_ANSI

void fatal(char *message) {
    fprintf(stderr, "fatal error: %s\n", message);
    exit(1);
}


int kbhit() {
    struct timeval  tv;
    fd_set          fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    //STDIN_FILENO is 0
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

#define NB_DISABLE 1
#define NB_ENABLE 0

#ifndef EMBEDDED
void nonblock(int state) {
    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==NB_ENABLE)     {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    } else if (state==NB_DISABLE) {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}
#endif

char keystroke(int t) {
    static char     buf[10];
    static int      total, next;
    //    struct sgttyb   ns;

    if (next >= total)     {
        switch (total = read(0, buf, sizeof(buf)))         {
            case -1:
                fatal("System call failure: read\n");
                break;
            case 0:
                fatal("Mysterious EOF\n");
                break;
            default:
                next = 0;
                break;
        }
    }
    return (buf[next++]);
}

static void athStdoutFlush(ficlVm * vm) {
    fflush( (FILE *)NULL );
}

/*
 ** Ficl interface to _getcwd (Win32)
 ** Prints the current working directory using the VM's
 ** textOut method...
 */
#ifndef EMBEDDED
static void ficlPrimitiveGetCwd(ficlVm * vm) {
    char           *directory;

    directory = (char *) getcwd((char *) NULL, 80);
    ficlVmTextOut(vm, directory);
    ficlVmTextOut(vm, "\n");
    free(directory);
    return;
}

/*
 ** Ficl interface to _chdir (Win32)
 ** Gets a newline (or NULL) delimited string from the input
 ** and feeds it to the Win32 chdir function...
 ** Example:
 **    cd c:\tmp
 */
static void ficlPrimitiveChDir(ficlVm * vm) {
    ficlCountedString *counted = (ficlCountedString *) vm->pad;
    ficlVmGetString(vm, counted, '\n');

    if (counted->length > 0) {
        int             err = chdir(counted->text);
        if (err) {
            ficlVmTextOut(vm, "Error: path not found\n");
            ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
        }
    } else {
        ficlVmTextOut(vm, "Warning (chdir): nothing happened\n");
    }
    return;
}
#endif


static void ficlPrimitiveClock(ficlVm * vm) {
    clock_t         now = clock();
    ficlStackPushUnsigned(vm->dataStack, (ficlUnsigned) now);
    return;
}

#endif				/* FICL_ANSI */

/*
 ** Ficl interface to system (ANSI)
 ** Gets a newline (or NULL) delimited string from the input
 ** and feeds it to the ANSI system function...
 ** Example:
 **    system del *.*
 **    \ ouch!
 */
static void ficlPrimitiveSystem(ficlVm * vm) {
    ficlCountedString *counted = (ficlCountedString *) vm->pad;

    ficlVmGetString(vm, counted, '\n');
    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) > 0)
    {
        int             returnValue = system(FICL_COUNTED_STRING_GET_POINTER(*counted));
        if (returnValue)
        {
            sprintf(vm->pad, "System call returned %d\n", returnValue);
            ficlVmTextOut(vm, vm->pad);
            ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
        }
    } else
    {
        ficlVmTextOut(vm, "Warning (system): nothing happened\n");
    }
    return;
}

static void athFeatures(ficlVm *vm) {
#ifdef MAC
    printf ("    MAC\n");
#else
    printf ("NOT MAC\n");
#endif

#ifdef COBALT
    printf ("    COBALT\n");
#else
    printf ("NOT COBALT\n");
#endif

#ifdef LINUX
    printf ("    LINUX\n");
#else
    printf ("NOT LINUX\n");
#endif

#ifdef UCLINUX
    printf ("    UCLINUX\n");
#else
    printf ("NOT UCLINUX\n");
#endif

#ifdef SOLARIS
    printf("    SOLARIS\n");
#else
    printf("NOT SOLARIS\n");
#endif

#ifdef MTHREAD
    printf("    MTHREAD\n");
#else
    printf("NOT MTHREAD\n");
#endif


#ifdef QNX
    printf("    QNX\n");
#else
    printf("NOT QNX\n");
#endif
    printf("\n");

#ifdef FICL_ANSI 
    printf("    FICL_ANSI\n");
#else
    printf("NOT FICL_ANSI\n");
#endif

#ifdef FICL_WANT_MINIMAL
    printf("    FICL_WANT_MINIMAL\n");
#else
    printf("NOT FICL_WANT_MINIMAL\n");
#endif

#ifdef SYSV_IPC
    printf ("    SYSV_IPC\n");
#else    
    printf ("NOT SYSV_IPC\n");    
#endif

#ifdef DYNLIB
    printf("    DYNLIB\n");
#else
    printf("NOT DYNLIB\n");
#endif


#ifdef SOCKET
    printf("    SOCKET\n");
#else
    printf("NOT SOCKET\n");
#endif
#if FICL_WANT_FILE
    printf("    WANT_FILE\n");
#else
    printf("NOT WANT_FILE\n");
#endif

    printf("    WANT_FLOAT\n");

#if FICL_WANT_DEBUGGER
    printf("    WANT_DEBUGGER\n");
#else
    printf("NOT WANT_DEBUGGER\n");
#endif
}

struct timeval now;

static void athNow(ficlVm *vm) {
    int status=0;

    status=gettimeofday(&now, NULL);
}

static void athElapsed(ficlVm *vm) {
    int status=0;
    int sec;
    int us;
    int msec;

    struct timeval here;

    status=gettimeofday(&here, NULL);

    sec = here.tv_sec - now.tv_sec;
    us  = here.tv_usec - now.tv_usec;

    msec = (sec * 1000) + ( us / 1000 );

    ficlStackPushInteger(vm->dataStack, msec);

}

static void athTime(ficlVm * vm) {
    time_t          t;

    t = time((time_t *) NULL);
    ficlStackPushInteger(vm->dataStack, t);
}

static void athDate(ficlVm * vm) {
    time_t          t;
    struct tm *d;

    t = (time_t)ficlStackPopInteger(vm->dataStack);
    d = localtime(&t);

    ficlStackPushInteger(vm->dataStack, (d->tm_year+1900));
    ficlStackPushInteger(vm->dataStack, (d->tm_mon+1));
    ficlStackPushInteger(vm->dataStack, d->tm_mday );
    ficlStackPushInteger(vm->dataStack, d->tm_sec );
    ficlStackPushInteger(vm->dataStack, d->tm_min );
    ficlStackPushInteger(vm->dataStack, d->tm_hour );
}

static void athPrimitiveDollarSystem(ficlVm * vm) {
    char           *cmd;
    int             len;
    int             status;

    len = ficlStackPopInteger(vm->dataStack);
    cmd = ficlStackPopPointer(vm->dataStack);

    if (len > 0)
        cmd[len] = '\0';

    status = system(cmd);

    ficlStackPushInteger(vm->dataStack, status);
}

/*
 ** Ficl add-in to load a text file and execute it...
 ** Cheesy, but illustrative.
 ** Line oriented... filename is newline (or NULL) delimited.
 ** Example:
 **    load test.f
 */
#define BUFFER_SIZE 256

char           *pathToFile(char *fname) {
    int             i;
    int             fd;
    char		    *loadPath =NULL;
    char            path[255];
    char            scr[255];
    char           *scratch;
    char           *tok;

    if ((loadPath == (char *) NULL) || (*fname == '/') || (*fname == '.'))
        return (fname);

    strcpy(path, loadPath);

    tok = (char *) strtok(path, ":");
    i = 0;

    for (; tok != (char *) NULL;) {
        strcpy(scr, tok);
        strcat(scr, "/");
        strcat(scr, fname);

        scratch = strsave(scr);

        fd = open(scratch, O_RDONLY);

        if (fd >= 0) {
            close(fd);
            return (scratch);
        } else {
            free(scratch);
        }

        tok = (char *) strtok(NULL, ":");
    }
    return (NULL);
}

static void ficlDollarPrimitiveLoad(ficlVm * vm) {
    char            buffer[BUFFER_SIZE];
    char            filename[BUFFER_SIZE];
    char            fullName[255];

    char           *scratch;
    ficlCountedString *counted = (ficlCountedString *) filename;
    int             line = 0;
    FILE           *f;
    int             result = 0;
    ficlCell        oldSourceId;
    ficlString      s;
    int             nameLen;
    char           *name;
    char *ptr=(char *)NULL;

    nameLen = ficlStackPopInteger(vm->dataStack);
    ptr=ficlStackPopPointer(vm->dataStack);
    name=strtok(ptr," ");
    name[nameLen] = '\0';

    scratch = pathToFile(name);

    if (scratch == (char *) NULL) {
        sprintf(buffer, "File not found :%s", name);
        ficlVmTextOut(vm, buffer);
        ficlVmTextOut(vm, FICL_COUNTED_STRING_GET_POINTER(*counted));
        ficlVmTextOut(vm, "\n");
        ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
    } else {
        strcpy(fullName, scratch);
    }
    /*
     ** get the file's size and make sure it exists
     */
    f = fopen(fullName, "r");
    if (!f) {
        sprintf(buffer, "Unable to open file %s", name);
        ficlVmTextOut(vm, buffer);
        ficlVmTextOut(vm, FICL_COUNTED_STRING_GET_POINTER(*counted));
        ficlVmTextOut(vm, "\n");
        ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
    }
    oldSourceId = vm->sourceId;
    vm->sourceId.p = (void *) f;

    /* feed each line to ficlExec */
    while (fgets(buffer, BUFFER_SIZE, f)) {
        int             length = strlen(buffer) - 1;

        line++;
        if (length <= 0)
            continue;

        if (buffer[length] == '\n')
            buffer[length--] = '\0';

        FICL_STRING_SET_POINTER(s, buffer);
        FICL_STRING_SET_LENGTH(s, length + 1);
        result = ficlVmExecuteString(vm, s);
        /* handle "bye" in loaded files. --lch */
        switch (result) {
            case FICL_VM_STATUS_OUT_OF_TEXT:
                break;
            case FICL_VM_STATUS_USER_EXIT:
                exit(0);
                break;

            default:
                vm->sourceId = oldSourceId;
                fclose(f);
                ficlVmThrowError(vm, "Error loading file <%s> line %d", FICL_COUNTED_STRING_GET_POINTER(*counted), line);
                break;
        }
    }
    /*
     ** Pass an empty line with SOURCE-ID == -1 to flush
     ** any pending REFILLs (as required by FILE wordset)
     */
    vm->sourceId.i = -1;
    FICL_STRING_SET_FROM_CSTRING(s, "");
    ficlVmExecuteString(vm, s);

    vm->sourceId = oldSourceId;
    fclose(f);

    /* handle "bye" in loaded files. --lch */
    if (result == FICL_VM_STATUS_USER_EXIT)
        ficlVmThrow(vm, FICL_VM_STATUS_USER_EXIT);
    return;
}

static void ficlPrimitiveLoad(ficlVm * vm) {
    char            filename[BUFFER_SIZE];

    extern char    *loadPath;
    char           *name;

    ficlCountedString *counted = (ficlCountedString *) filename;
    ficlVmGetString(vm, counted, '\n');

    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) <= 0) {
        ficlVmTextOut(vm, "Warning (load): nothing happened\n");
        return;
    }
    name = FICL_COUNTED_STRING_GET_POINTER(*counted);


    ficlStackPushPointer(vm->dataStack, name);
    ficlStackPushInteger(vm->dataStack, FICL_COUNTED_STRING_GET_LENGTH(*counted));
    ficlDollarPrimitiveLoad(vm);
}

static void ficlDollarPrimitiveLoadDir(ficlVm * vm) {
    char           *dirName, *fileName;
    char            buffer[255];

    int             dirLen, fileLen;

    fileLen = ficlStackPopInteger(vm->dataStack);
    fileName = ficlStackPopPointer(vm->dataStack);
    fileName[fileLen] = '\0';

    dirLen = ficlStackPopInteger(vm->dataStack);
    dirName = ficlStackPopPointer(vm->dataStack);
    dirName[dirLen] = '\0';

    sprintf(buffer, "%s/%s", dirName, fileName);
    ficlStackPushPointer(vm->dataStack, buffer);
    ficlStackPushInteger(vm->dataStack, strlen(buffer));
    ficlDollarPrimitiveLoad(vm);
}


/*
 ** Dump a tab delimited file that summarizes the contents of the
 ** dictionary hash table by hashcode...
 */
    static void
ficlPrimitiveSpewHash(ficlVm * vm)
{
    ficlHash       *hash = ficlVmGetDictionary(vm)->forthWordlist;
    ficlWord       *word;
    FILE           *f;
    unsigned        i;
    unsigned        hashSize = hash->size;

    if (!ficlVmGetWordToPad(vm))
        ficlVmThrow(vm, FICL_VM_STATUS_OUT_OF_TEXT);

    f = fopen(vm->pad, "w");
    if (!f)
    {
        ficlVmTextOut(vm, "unable to open file\n");
        return;
    }
    for (i = 0; i < hashSize; i++)
    {
        int             n = 0;

        word = hash->table[i];
        while (word)
        {
            n++;
            word = word->link;
        }

        fprintf(f, "%d\t%d", i, n);

        word = hash->table[i];
        while (word)
        {
            fprintf(f, "\t%s", word->name);
            word = word->link;
        }

        fprintf(f, "\n");
    }

    fclose(f);
    return;
}

char           *strsave(char *s) {
    char           *p;

    if ((p = (char *) malloc(strlen(s) + 1)) != NULL)
        strcpy(p, s);
    return (p);
}

#if FICL_WANT_STRING
#warning "String stack words...."
static void athStringPush(ficlVm *vm) {
    char *p,*n;
    int l;

    l= ficlStackPopInteger(vm->dataStack);
    p = ficlStackPopPointer(vm->dataStack);

    ficlStackPushPointer(vm->stringStack,(char *)strsave(p));

}

static void athStringPop(ficlVm *vm) {
    char *d,*n;
    int l;

    l= ficlStackPopInteger(vm->dataStack);
    d=ficlStackPopPointer(vm->dataStack);
    n=ficlStackPopPointer(vm->stringStack);

    strncpy(d,n,l);
    free(n);
    ficlStackPushInteger(vm->dataStack,strlen(d));

}

static void athStringJoin(ficlVm *vm)
{
    char *a,*b;
    char *new;
    int la,lb,ln;

    b=ficlStackPopPointer(vm->stringStack);
    a=ficlStackPopPointer(vm->stringStack);

    la=strlen(a);
    lb=strlen(b);
    ln=la+lb;

    new=(char *)malloc( ln+1); // space for null terminator

    strcpy(new,a);
    strcat(new,b);
    free(a);
    free(b);

    ficlStackPushPointer(vm->stringStack,new);
}

#endif

    static void
athSlashString(ficlVm * vm)
{
    char           *str;
    int             len, cut;

    cut = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    str = ficlStackPopPointer(vm->dataStack);

    len = len - cut;
    str = str + cut;

    ficlStackPushPointer(vm->dataStack, str);
    ficlStackPushInteger(vm->dataStack, len);

}

    static void
athMinusTrailing(ficlVm * vm)
{
    char           *str;
    int             len;
    int             i;

    len = ficlStackPopInteger(vm->dataStack);
    str = ficlStackPopPointer(vm->dataStack);

    for (i = (len - 1); i >= 0; i--)
    {
        if (str[i] > ' ')
            break;
        else
            len--;
    }
    ficlStackPushPointer(vm->dataStack, str);
    ficlStackPushInteger(vm->dataStack, len);
}

    static void
athTwoRot(ficlVm * vm)
{
    void           *stuff[6];
    int             i = 0;

    for (i = 0; i < 6; i++)
    {
        stuff[i] = (void *) ficlStackPopInteger(vm->dataStack);
    }

    for (i = 3; i >= 0; i--)
    {
        ficlStackPushPointer(vm->dataStack, stuff[i]);
    }
    ficlStackPushPointer(vm->dataStack, stuff[5]);
    ficlStackPushPointer(vm->dataStack, stuff[4]);
}

static void athStrsave(ficlVm * vm) {
    int             len;
    char           *str;
    char           *p;

    len = ficlStackPopInteger(vm->dataStack);
    str = ficlStackPopPointer(vm->dataStack);

    str[len] = '\0';
    p = (char *) strsave(str);

    ficlStackPushPointer(vm->dataStack, p);
    ficlStackPushInteger(vm->dataStack, len);
}

// Same as move except adds a 0 byte at the end.
// Traget area must be at least one byte bigger than len
//
// Stack: from to len --
static void athZmove(ficlVm *vm) {
    char *from, *to, *ret;
    int len;

    len = ficlStackPopInteger(vm->dataStack);
    to = ficlStackPopPointer(vm->dataStack);
    from = ficlStackPopPointer(vm->dataStack);

    ret = strncpy(to,from,len);
    *(to + len) = (char)NULL;

}
// 
// Add an end of line char (0x0a) to a string.
// Space must already exist for tis.
//
// Stack : addr len -- addr len+1
//
static void athAddCr(ficlVm *vm) {
    char *from;
    int len;

    len = ficlStackPopInteger(vm->dataStack);
    from = ficlStackPopPointer(vm->dataStack);

    *(from + len) = (char)0x0a;

    len++;

    ficlStackPushPointer( vm->dataStack,from);
    ficlStackPushInteger(vm->dataStack, len);

}

static void athSizeofInt(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, sizeof(int));
}

static void athSizeofChar(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, sizeof(char));
}

static void athSizeofCharPtr(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, sizeof(char *));
}

static void athGetPid(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, (int) getpid() );
}

#define READ 0
#define WRITE 1

#ifndef  EMBEDDED
pid_t popen2(const char *command, int *infp, int *outfp)
{
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];

    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];

    return pid;
}

/**
 * bidirectional popen() call
 *
 * @param rwepipe - int array of size three
 * @param exe - program to run
 * @param argv - argument list
 * @return pid or -1 on error
 *
 * The caller passes in an array of three integers (rwepipe), on successful
 * execution it can then write to element 0 (stdin of exe), and read from
 * element 1 (stdout) and 2 (stderr).
 */

static int popenRWE(int *rwepipe, const char *exe, const char *const argv[]) {
    int in[2];
    int out[2];
    int err[2];
    int pid;
    int rc;

    rc = pipe(in);
    if (rc<0)
        goto error_in;

    rc = pipe(out);
    if (rc<0)
        goto error_out;

    rc = pipe(err);
    if (rc<0)
        goto error_err;

    pid = fork();
    if (pid > 0) { // parent
        close(in[0]);
        close(out[1]);
        close(err[1]);
        rwepipe[0] = in[1];
        rwepipe[1] = out[0];
        rwepipe[2] = err[0];
        return pid;
    } else if (pid == 0) { // child
        close(in[1]);
        close(out[0]);
        close(err[0]);
        close(0);
        dup(in[0]);
        close(1);
        dup(out[1]);
        close(2);
        dup(err[1]);

        execvp(exe, (char**)argv);
        exit(1);
    } else {
        goto error_fork;
    }

    return pid;

error_fork:
    close(err[0]);
    close(err[1]);
error_err:
    close(out[0]);
    close(out[1]);
error_out:
    close(in[0]);
    close(in[1]);
error_in:
    return -1;
}

static int pcloseRWE(int pid, int *rwepipe)
{
    int rc, status;
    close(rwepipe[0]);
    close(rwepipe[1]);
    close(rwepipe[2]);
    rc = waitpid(pid, &status, 0);
    return status;
}

/*
 * Need to fix a number of BUGS in this
 * 1. Deal with popenRWE returning an error.
 * 2. Close needs to free the ficlFile descriptors.
 */
void athPopenRWE(ficlVm *vm) {
    int len;
    int rwepipe[3];
    int i=0;
    int ret;

    char *cmd;
    char *args[32];
    char *tmp;
    char *exe;
    ficlFile *pStdin;
    ficlFile *pStdout;
    ficlFile *pStderr;

    len = ficlStackPopInteger(vm->dataStack);
    cmd = ficlStackPopPointer(vm->dataStack);
    cmd[len] = '\0';

    exe=(char *)strtok(cmd," ");

    i=0;
    args[i++]=exe;

    while( (tmp=(char *)strtok(NULL," "))) {
        //        printf("%s\n",tmp);
        args[i++] = tmp;
    }
    args[i]=(char *)NULL;

    ret = access( exe , R_OK | X_OK );

    if( 0 == ret ) {

        pStdin = (ficlFile *)malloc(sizeof(struct ficlFile));
        pStdout= (ficlFile *)malloc(sizeof(struct ficlFile));
        pStderr= (ficlFile *)malloc(sizeof(struct ficlFile));

        /*
           exe=(char *)strtok(cmd," ");

           i=0;
           args[i++]=exe;

           while( (tmp=(char *)strtok(NULL," "))) {
        //        printf("%s\n",tmp);
        args[i++] = tmp;
        }
        args[i]=(char *)NULL;
        */

        ret=popenRWE(&rwepipe[0],cmd,(const char * const*)args);

        pStdin->fd = rwepipe[0];
        pStdin->f = fdopen(rwepipe[0],"w");
        strcpy( pStdin->filename, "Pipe:stdin");

        pStdout->fd=rwepipe[1];
        pStdout->f = fdopen(rwepipe[1],"r");
        strcpy( pStdout->filename, "Pipe:stdout");

        pStderr->fd=rwepipe[2];
        pStderr->f = fdopen(rwepipe[2],"r");
        strcpy( pStderr->filename, "Pipe:stderr");

        ficlStackPushPointer(vm->dataStack, pStderr);
        ficlStackPushPointer(vm->dataStack, pStdout);
        ficlStackPushPointer(vm->dataStack, pStdin);
        ficlStackPushInteger(vm->dataStack, ret); // pid
        ficlStackPushInteger(vm->dataStack, 0);   // Sucess
    } else {
        ficlStackPushInteger(vm->dataStack, errno); 
        ficlStackPushInteger(vm->dataStack, -1); // fail :(
    }
}

void athPcloseRWE(ficlVm *vm) 
{
    int rwepipe[3];
    int pid;

    ficlFile *pStdin;
    ficlFile *pStdout;
    ficlFile *pStderr;

    pid     = ficlStackPopInteger( vm->dataStack );
    pStderr = (ficlFile *)ficlStackPopPointer( vm->dataStack);
    pStdout = (ficlFile *)ficlStackPopPointer( vm->dataStack);
    pStdin  = (ficlFile *)ficlStackPopPointer( vm->dataStack);

    rwepipe[0] = pStdin->fd;
    rwepipe[1] = pStdout->fd;
    rwepipe[2] = pStdin->fd;
    pcloseRWE(pid,&rwepipe[0]);

    free( pStdin );
    free( pStdout );
    free( pStderr );
}
#endif

static int lastSignal;
    void
signalHandler(int sig)
{
    lastSignal = sig;
}

void getLastSignal(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, lastSignal);
    lastSignal=0;
}

#ifndef EMBEDDED
struct sigaction act;

static void athSignal(ficlVm * vm) {
    int             sig;
    sig = ficlStackPopInteger(vm->dataStack);
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    sigaction(sig, &act, NULL);
}

static void athKill(ficlVm *vm) {
    pid_t pid;
    int sig;

    pid = (pid_t)ficlStackPopInteger(vm->dataStack);
    sig = ficlStackPopInteger(vm->dataStack);

    ficlStackPushInteger(vm->dataStack, kill(pid,sig) );
}


static void athSetAlarm(ficlVm * vm) {
    int             t;
    t = ficlStackPopInteger(vm->dataStack);
    alarm(t);

}
#endif

    static void
athPerror(ficlVm * vm)
{
    perror("ficl");
}

    static void
athClrErrno(ficlVm * vm)
{
    extern int      errno;
    errno = 0;
}

    static void
athGetenv(ficlVm * vm)
{
    char           *ptr;
    char           *env;
    char           *tmp;

    int             len;

	FICL_STACK_CHECK(vm->dataStack, 2, 2);

    len = ficlStackPopInteger(vm->dataStack);
    env = (char *)ficlStackPopPointer(vm->dataStack);
    ptr = (char *)ficlStackPopPointer(vm->dataStack);

	if (ptr == 0)
	{
		ficlVmTextOut(vm, "Error: (ptr env len -- res len) ptr = 0\n");
		ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
	}

    env[len] = '\0';
    tmp = getenv(env);
    if (tmp)
    {
        strcpy(ptr, tmp);
        len = strlen(tmp);
    } else
        len = 0;

    ficlStackPushPointer(vm->dataStack, ptr);
    ficlStackPushInteger(vm->dataStack, len);

}

    static void
athGetErrno(ficlVm * vm)
{
    extern int      errno;
    ficlStackPushInteger(vm->dataStack, errno);
    errno = 0;
}

/*
 * Stack: sep ptr cnt --- addrn lenn ...... addr0 len0 n
 */

    static void
athStrTok(ficlVm * vm)
{
    char           *ptr;
    char            s;
    char            sep[2];
    int             len;
    char           *t[255];
    int             i = 0;
    int             count = 0;

    char           *tok;

    len = ficlStackPopInteger(vm->dataStack);
    ptr = (char *)ficlStackPopPointer(vm->dataStack);
    s = ficlStackPopInteger(vm->dataStack);

    sep[0] = s;
    sep[1] = '\0';

    *(ptr + len) = '\0';

    count = 0;
    do
    {
        if (count == 0)
            tok = (char *) strtok(ptr, sep);
        else
            tok = (char *) strtok(NULL, sep);

        if (tok)
            t[count++] = tok;
    }
    while (tok != NULL);
    //	count;

    for (i = count - 1; i >= 0; i--)
    {
        /*
         * printf("i = %d\n",i); printf("t[%d] = %s \n",i,t[i]);
         */

        len = strlen(t[i]);
        ficlStackPushPointer(vm->dataStack, t[i]);
        ficlStackPushInteger(vm->dataStack, len);
    }
    ficlStackPushInteger(vm->dataStack, count);
}

static void athFiclFileDump(ficlVm *vm)
{
    ficlFile *a;

    a = (ficlFile *)ficlStackPopPointer(vm->dataStack);

    printf("FILE *: %x\n",(unsigned int)a->f);
    printf("Name  : %s\n",a->filename);
    printf("fd    : %d\n",a->fd);
}

static void athMs(ficlVm * vm) {
    int             ms;
    int i;
#ifdef LINUX
    struct timespec tim,tim2;
#endif

    ms = ficlStackPopInteger(vm->dataStack);
    for (i=0;i < ms; i++) {
#ifdef LINUX
        usleep(1000);

        tim.tv_sec=0;
        tim.tv_nsec=( 1000 * 1000 * ms );
        (void) nanosleep(&tim,&tim2);
#else
        if ( usleep(1000) < 0)
            return;
#endif
    }
}

static void athSleep(ficlVm *vm)
{
    (void) sleep(ficlStackPopInteger(vm->dataStack));
}

char prompt[32];
static void athSetPrompt(ficlVm *vm)
{
    //    extern char prompt[];
    char *ptr;
    int len;
    int crFlag = 0;

    memset(prompt,0x00, 32);
    //    bzero(prompt,32);

    crFlag = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    ptr = (char *)ficlStackPopPointer(vm->dataStack);

    strncpy(prompt,ptr,len);

    if( crFlag != 0) {
        strcat(prompt,"\n");
    }

}

static void athResetPrompt(ficlVm *vm)
{
    extern char prompt[];

    memset(prompt,0x00, 32);
    //    bzero(prompt,32);

    strcpy(prompt,FICL_PROMPT);
}


#ifdef MTHREAD
/*
   static void athCreateThread(ficlVm * vm)
   {
   ficlVm         *newVm;
   char           *ptr;
   int             len;
   int             i;
   pthread_t       t;
   int             status;


   newVm = ficlStackPopPointer(vm->dataStack);
   len = ficlStackPopInteger(vm->dataStack);
   ptr = ficlStackPopPointer(vm->dataStack);
   ptr[len] = '\0';

   ficlStackPushPointer(newVm->dataStack, ptr);
   ficlStackPushInteger(newVm->dataStack, len);



   status = pthread_create(&t,
   NULL,
   (void *) athExecuteThread,
   (void *) newVm);

//	ficlStackPushPointer(vm->dataStack, (void *) t);
ficlStackPushInteger(vm->dataStack, status);

}
*/

/*
   static void
   athYieldThread(ficlVm * vm)
   {
#ifdef _POSIX_PRIORITY_SCHEDULING
int             status;
#warning "Yield works"
#ifdef MAC
#warning "sched_yield"
status = sched_yield();
#else
#warning "pthread_yield"
status=pthread_yield();
#endif
//    usleep(1);
}
*/
    static void
athDeleteThread(ficlVm * vm)
{
    pthread_t       t;

    t = (pthread_t) ficlStackPopPointer(vm->dataStack);
}

    static void
athCreateMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;

    mutexp = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutexp, NULL);

    ficlStackPushPointer(vm->dataStack, mutexp);
}

    static void
athLockMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;
    int             status;

    mutexp = ficlStackPopPointer(vm->dataStack);
    status = pthread_mutex_lock(mutexp);
    ficlStackPushInteger(vm->dataStack, status);
}

    static void
athUnlockMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;
    int             status;

    mutexp = ficlStackPopPointer(vm->dataStack);
    status = pthread_mutex_unlock(mutexp);
    ficlStackPushInteger(vm->dataStack, status);
}


    static void
athTryLockMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;
    int             status;

    mutexp = ficlStackPopPointer(vm->dataStack);
    status = pthread_mutex_trylock(mutexp);
    ficlStackPushInteger(vm->dataStack, status);
}
#endif

/* Stack -- socket */
#ifdef SOCKET

    static void
athSocket(ficlVm * vm)
{
    int             sock1;
    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    ficlStackPushInteger(vm->dataStack, sock1);
}

//Stack:--socket
/*
   static void athLocalSocket(ficlVm *vm)
   {
   int sock1;

   sock1 =  socket(AF_UNIX, SOCK_STREAM, 0);
   ficlStackPushInteger(vm->dataStack,sock1);
   }
   */
/* Stack: addr len port -- flag */

static void athConnect(ficlVm * vm) {
    char           *hostName;
    int             len, port;
    int             tmp;
    int             sock1;
    int             exitStatus = 0;
    struct sockaddr_in serv_addr;
    struct hostent *hp;
    int rc;

    struct addrinfo *result = NULL;
    struct addrinfo hint;

    char portNumber[8];

    memset(&hint, 0 , sizeof(hint));

//    hint.ai_family = AF_UNSPEC;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    port = ficlStackPopInteger(vm->dataStack);

    sprintf(portNumber,"%d",port);
    len = ficlStackPopInteger(vm->dataStack);

    hostName = (char *)ficlStackPopPointer(vm->dataStack);
//    hostName[len] = '\0';
    
//    rc = getaddrinfo(hostName, NULL /*service*/, &hint, &result);
    rc = getaddrinfo(hostName, portNumber, &hint, &result);

    if( 0 == rc ) {
        sock1 = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(sock1 < 0) {
            exitStatus = -1;
        } else {
            tmp = connect(sock1, result->ai_addr, result->ai_addrlen );
            if (tmp < 0)
                exitStatus = -1;
        }
    }

    if (exitStatus == 0) {
        ficlStackPushInteger(vm->dataStack, sock1);
    }
    ficlStackPushInteger(vm->dataStack, exitStatus);
}

/* Stack: port socket -- status */

    static void
athBind(ficlVm * vm)
{
    struct sockaddr_in serv_addr;
    int             port, status, sock1;
    struct hostent *hp;

    sock1 = ficlStackPopInteger(vm->dataStack);
    port = ficlStackPopInteger(vm->dataStack);

    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    status = bind(sock1, (struct sockaddr *) & serv_addr, sizeof(serv_addr));


    ficlStackPushInteger(vm->dataStack, status);
}

    static void
athListen(ficlVm * vm)
{
    int             sock1, status;

    sock1 = ficlStackPopInteger(vm->dataStack);
    status = listen(sock1, 5);
    ficlStackPushInteger(vm->dataStack, status);
}


    static void
athAccept(ficlVm * vm)
{
    int             sock1, status, clnt_len;
    int             sock2;

    struct sockaddr_in clnt_addr;

    sock1 = ficlStackPopInteger(vm->dataStack);
    sock2 = accept(sock1, (struct sockaddr *) & clnt_addr, (socklen_t *) & clnt_len);

    ficlStackPushInteger(vm->dataStack, sock2);
}

/* Stack: buffer len socket -- count */

static void athRecv(ficlVm * vm)
{
    int             n;
    int             sock2;
    int             len;
    int flag = 0;
    char           *msg;

    sock2 = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    msg =(char *)ficlStackPopPointer(vm->dataStack);
    n = recv(sock2, msg, len, 0);
    ficlStackPushInteger(vm->dataStack, n);
    //    flag = ( n < 0 );
    //	ficlStackPushInteger(vm->dataStack, flag);
}

    static void
athSend(ficlVm * vm)
{
    char           *buffer;
    int             len;
    int flag=0;
    int             sock2;
    int             status;

    sock2 = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    buffer = (char *)ficlStackPopPointer(vm->dataStack);

    status = send(sock2, buffer, len, 0);
    ficlStackPushInteger(vm->dataStack, status);
    flag = ( status < 0 );
    ficlStackPushInteger(vm->dataStack, flag);

}

    static void
athClose(ficlVm * vm)
{
    int             sock;

    sock = ficlStackPopInteger(vm->dataStack);
    close(sock);
}

#endif

/*
   arg O_NONBLOCK 0x004
   cmd F_SETFL 4
   */

static void athFdGet( ficlVm *vm) {
    int fd;
    ficlFile *ff;

    ff  = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    fd = (int)ff->fd;

    ficlStackPushInteger(vm->dataStack, fd);
}

static void athFcntl(ficlVm *vm) {
    int cmd;
    int arg;
    int fd;
    int status = 0;

    //    ficlFile *ff;

    arg = ficlStackPopInteger(vm->dataStack);
    cmd = ficlStackPopInteger(vm->dataStack);
    //	ff  = ficlStackPopPointer(vm->dataStack);
    fd  = ficlStackPopInteger(vm->dataStack);

    //    fd = ff->fd;

    //    printf("Before Flags = %02x\n",fcntl(fd,F_GETFL,0) );
    status = fcntl(fd,cmd,arg);
    //    printf("After Flags = %02x\n",fcntl(fd,F_GETFL,0) );
    ficlStackPushInteger(vm->dataStack, status);

}

/*
   static void athIoctl(ficlVm *vm)
   {
   int fd;
   unsigned long cmd;
   void *arg;

   arg = (void *)ficlStackPopInteger(vm->dataStack);
   cmd = (unsigned long)ficlStackPopInteger(vm->dataStack);
   fd  = (int)ficlStackPopInteger(vm->dataStack);
   }
   */

    static void
athSeal(ficlVm * vm)
{
    vm->sealed = -1;
}

    static void
athUnseal(ficlVm * vm)
{
    vm->sealed = 0;
}

/*
 * Investigate using sysctl as it may be more portable
 * than uname.
 */

// #ifndef ARM
#ifndef EMBEDDED
static void athUname(ficlVm * vm) {
    struct utsname  buf;
    int             res;
    //    extern char    *loadPath;

    res = uname(&buf);
    printf("System name :%s\n", buf.sysname);
    printf("Host name   :%s\n", buf.nodename);
    printf("Release     :%s\n", buf.release);
    printf("Version     :%s\n", buf.version);
    printf("Machine     :%s\n", buf.machine);
    //    printf("Load        :%s\n", loadPath);
}
#endif

int verbose;

static void athVerboseQ(ficlVm *vm) {
    ficlStackPushInteger(vm->dataStack, verbose);
}
/*
#define OS_UNKNOWN 0
#define OS_LINUX 1
#define OS_DARWIN 2
#define OS_FREEBSD 3
#define OS_SOLARIS 4
#define OS_UCLINUX 5
#define OS_QNX 6
*/

/*
#define CPU_UNKNOWN 0
#define CPU_X86 1
#define CPU_PPC 2
#define CPU_8XX 3
#define CPU_AMD64 4
#define CPU_SPARC 5
#define CPU_COLDFIRE 6
#define CPU_ARM 7
#define CPU_MIPS 8
#define CPU_SH4A 9
*/

// #ifndef ARM
static void athCpu(ficlVm * vm) {
    int             cpu = CPU_UNKNOWN;
#ifndef EMBEDDED
    struct utsname  buf;
    int             res;

    res = uname(&buf);

    //    printf("%s\n",buf.machine);

    if ((strcasecmp(buf.machine, "i686") == 0) || (strcasecmp(buf.machine, "i386") == 0) || (strcasecmp(buf.machine, "x86pc") == 0) || (strcasecmp(buf.machine,"x86_64") == 0) )
    {
        cpu = CPU_X86;
    } else if ( (strcasecmp(buf.machine, "Power Macintosh") == 0) || (strcasecmp(buf.machine, "ppc") == 0) )
    {
        cpu = CPU_PPC;
    } else if ((strcasecmp(buf.machine, "armv5tel") == 0) || !strcmp(buf.machine,"armv7l") || !strcmp(buf.machine,"armv6l")) {
        cpu = CPU_ARM;
    } else if (strcasecmp(buf.machine, "m68knommu") == 0) {
        cpu = CPU_COLDFIRE;
    } else if (strcasecmp(buf.machine, "mips") == 0) {
        cpu = CPU_MIPS;
    } else if (strcasecmp(buf.machine, "sh4a") == 0) {
        cpu = CPU_SH4A;
    }
#else
    cpu = CPU_ARM;
#endif

    ficlStackPushInteger(vm->dataStack, cpu);

}

static void athOs(ficlVm * vm) {
    int             os = OS_UNKNOWN;
#ifndef EMBEDDED
    struct utsname  buf;
    int             res;

    res = uname(&buf);

    //    printf("%s\n",buf.sysname);

    if (strcasecmp(buf.sysname, "linux") == 0) {
        os = OS_LINUX;
    } else if (strcasecmp(buf.sysname, "darwin") == 0) {
        os = OS_DARWIN;
    } else if (strcasecmp(buf.sysname, "uClinux") == 0) {
        os = OS_UCLINUX;
    }  else if (strcasecmp(buf.sysname, "QNX") == 0) {
        os = OS_QNX;
    }

#else
    os = OS_UNKNOWN;
#endif
    ficlStackPushInteger(vm->dataStack, os);
}

static void athLinuxQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_LINUX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athDarwinQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_DARWIN );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athBSDQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_FREEBSD );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athSolarisQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_SOLARIS );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}
static void athUcLinuxQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_UCLINUX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athQNXQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_QNX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athX86Q(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_X86 );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athPPCQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_PPC );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void ath8XXQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_8XX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athAMD64Q(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_AMD64 );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athSparcQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_SPARC  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athColdQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_COLDFIRE  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athARMQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_ARM  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athMIPSQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_MIPS  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athSH4AQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_SH4A  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}



/*
   Return the hostname at pad, and the length on the stack.
   */
static void athHostname(ficlVm * vm) {
    int             n;
#ifndef EMBEDDED
    struct utsname  buf;
    int             res;
    char *tmp;

    res = uname(&buf);

    tmp = (char *)strtok(buf.nodename,".");

    n = strlen(buf.nodename);
    //	strncpy((char *) dest, (char *) buf.nodename, (size_t) n);
    strncpy((char *) vm->pad, (char *) buf.nodename, (size_t) n);
#else
    n=0;
#endif
    ficlStackPushInteger(vm->dataStack, n);
}

#if FICL_WANT_FILE

void athRead(ficlVm *vm) {
    ssize_t size;
    size_t count;
    void *buf;
    int fd;
    extern int errno;

    count = ficlStackPopInteger(vm->dataStack);
    buf   = (void *)ficlStackPopPointer(vm->dataStack) ;
    fd    = ficlStackPopInteger(vm->dataStack);

    size = read(fd,buf,count);
    if( size < 0) {
        ficlStackPushInteger(vm->dataStack, errno);
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        ficlStackPushInteger(vm->dataStack, size);
        ficlStackPushInteger(vm->dataStack, 0);
    }

}

void athWrite(ficlVm *vm) {
    ssize_t size;
    size_t count;
    void *buf;
    int fd;
    extern int errno;

    count = ficlStackPopInteger(vm->dataStack);
    buf   = (void *)ficlStackPopPointer(vm->dataStack) ;
    fd    = ficlStackPopInteger(vm->dataStack);

    size = write(fd,buf,count);
    if( (size < 0) || ( (size == 0) && (errno !=0 ) ) ) {
        ficlStackPushInteger(vm->dataStack, errno);
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        ficlStackPushInteger(vm->dataStack, size);
        ficlStackPushInteger(vm->dataStack, 0);
    }
}

#endif

void ficlSystemCompileExtras(ficlSystem * system) {
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);

    //    ficlDictionarySetPrimitive(dictionary, (char *)"break", ficlPrimitiveBreak, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"get-pid", athGetPid, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"verbose?", athVerboseQ, FICL_WORD_DEFAULT);
#ifdef FICL_WANT_FILE
    ficlDictionarySetPrimitive(dictionary, (char *)"read",  athRead,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"write", athWrite, FICL_WORD_DEFAULT);
#endif

    //    ficlDictionarySetPrimitive(dictionary, (char *)"key", athGetkey, FICL_WORD_DEFAULT);
    //    ficlDictionarySetPrimitive(dictionary, (char *)"?key", athQkey, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"stdout-flush", athStdoutFlush, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"zmove", athZmove, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"add-cr", athAddCr, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"set-prompt", athSetPrompt, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"reset-prompt", athResetPrompt, FICL_WORD_DEFAULT);

#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, "popen", athPopenRWE, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "pclose", athPcloseRWE, FICL_WORD_DEFAULT);
#endif

    ficlDictionarySetPrimitive(dictionary, (char *)"load", ficlPrimitiveLoad, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"$load", ficlDollarPrimitiveLoad, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"$load-dir", ficlDollarPrimitiveLoadDir, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"spewhash", ficlPrimitiveSpewHash, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"system", ficlPrimitiveSystem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"$system", athPrimitiveDollarSystem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"elapsed", athElapsed, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"now", athNow, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"time", athTime, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"date", athDate, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)".features", athFeatures, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"(int)", athSizeofInt, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"(char)", athSizeofChar, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"(char*)", athSizeofCharPtr, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"getenv", athGetenv, FICL_WORD_DEFAULT);
#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, (char *)"signal", athSignal, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"kill", athKill, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"alarm", athSetAlarm, FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"last-signal", getLastSignal, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"perror", athPerror, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"clr-errno", athClrErrno, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"errno", athGetErrno, FICL_WORD_DEFAULT);
#ifdef SYSV_IPC    
    ficlDictionarySetPrimitive(dictionary, (char *)"relsem", athRelSem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"getsem", athGetSem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"setsemvalue", athSetSemValue, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"getsemvalue", athGetSemValue, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"rmsem", athRmSem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"semtran", athSemTran, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmat", athShmat, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmdt", athShmdt, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmrm", athShmrm, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmget", athShmGet, FICL_WORD_DEFAULT);
#endif    
    ficlDictionarySetPrimitive(dictionary, (char *)"ms", athMs, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"dump-fd",athFiclFileDump, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sleep", athSleep, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"strtok", athStrTok, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"strsave", athStrsave, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"/string", athSlashString, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"-trailing", athMinusTrailing, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"2rot", athTwoRot, FICL_WORD_DEFAULT);
    //    #ifndef ARM
#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, (char *)"uname", athUname, FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"os", athOs, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"linux?", athLinuxQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"bsd?", athBSDQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"darwin?", athDarwinQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"solaris?", athSolarisQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"uclinux?", athUcLinuxQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"QNX?", athQNXQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"x86?", athX86Q, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"ppc?", athPPCQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"8xx?", ath8XXQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"amd64?", athAMD64Q, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sparc?", athSparcQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"coldfire?", athColdQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"arm?", athARMQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"mips?", athMIPSQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sh4a?", athSH4AQ, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"cpu", athCpu, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"hostname", athHostname, FICL_WORD_DEFAULT);

#ifdef DB
    ficlDictionarySetPrimitive(dictionary, "sqlite-open", athSqliteOpen, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sqlite-compile", athSqliteCompile, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sqlite-fetch", athSqliteFetch, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sqlite-final", athSqliteFinal, FICL_WORD_DEFAULT);
#endif
    /*
#ifdef LIST
ficlDictionarySetPrimitive(dictionary, "mklist", athMkList, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "rmlist", athRmList, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-len", athListLen, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-append-string", athListAppendString, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-append-int", athListAppendInt, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-get", athListGet, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-get", athListGet, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-display", athListDisplay, FICL_WORD_DEFAULT);

#endif
*/
#ifdef MTHREAD
    //	ficlDictionarySetPrimitive(dictionary, "create-vm", athCreateVM, FICL_WORD_DEFAULT);
    //	ficlDictionarySetPrimitive(dictionary, "create-thread", athCreateThread, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "delete-thread", athDeleteThread, FICL_WORD_DEFAULT);
    //	ficlDictionarySetPrimitive(dictionary, "yield-thread", athYieldThread, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "create-mutex", athCreateMutex, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "lock-mutex", athLockMutex, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "trylock-mutex", athTryLockMutex, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "unlock-mutex", athUnlockMutex, FICL_WORD_DEFAULT);
    /*
       ficlDictionarySetPrimitive(dictionary, "mkqueue", athMkQueue, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qput", athQput, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qget", athQget, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qempty?", athQempty, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qfull?", athQfull, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qsize", athQsize, FICL_WORD_DEFAULT);
       */   
#endif

#ifdef SOCKET
    ficlDictionarySetPrimitive(dictionary, (char *)"socket", athSocket, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-bind", athBind, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-listen", athListen, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-accept", athAccept, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-recv", athRecv, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-send", athSend, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-connect", athConnect, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"socket-close", athClose, FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"fd@", athFdGet, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"fcntl", athFcntl, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"seal", athSeal, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"unseal", athUnseal, FICL_WORD_DEFAULT);

#ifndef FICL_ANSI
    ficlDictionarySetPrimitive(dictionary, (char *)"clock", ficlPrimitiveClock, FICL_WORD_DEFAULT);
    ficlDictionarySetConstant(dictionary,  (char *)"clocks/sec", CLOCKS_PER_SEC);
#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, (char *)"pwd", ficlPrimitiveGetCwd, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"cd", ficlPrimitiveChDir, FICL_WORD_DEFAULT);
#endif
#endif				/* FICL_ANSI */
#if FICL_WANT_STRING
#warning "Defining string primitives"
    ficlDictionarySetPrimitive(dictionary, "spush", athStringPush, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "spop", athStringPop, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "s+", athStringJoin, FICL_WORD_DEFAULT);

#endif
    return;
}

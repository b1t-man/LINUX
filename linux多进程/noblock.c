/*
管道的读写特点：
使用管道时，需要注意以下几种特殊的情况（假设都是阻塞I/O操作）
1.所有的指向管道写端的文件描述符都关闭了（管道写端引用计数为0），有进程从管道的读端
读数据，那么管道中剩余的数据被读取以后，再次read会返回0，就像读到文件末尾一样。

2.如果有指向管道写端的文件描述符没有关闭（管道的写端引用计数大于0），而持有管道写端的进程
也没有往管道中写数据，这个时候有进程从管道中读取数据，那么管道中剩余的数据被读取后，
再次read会阻塞，直到管道中有数据可以读了才读取数据并返回。

3.如果所有指向管道读端的文件描述符都关闭了（管道的读端引用计数为0），这个时候有进程
向管道中写数据，那么该进程会收到一个信号SIGPIPE, 通常会导致进程异常终止。

4.如果有指向管道读端的文件描述符没有关闭（管道的读端引用计数大于0），而持有管道读端的进程
也没有从管道中读数据，这时有进程向管道中写数据，那么在管道被写满的时候再次write会阻塞，
直到管道中有空位置才能再次写入数据并返回。

总结：
    读管道：
        管道中有数据，read返回实际读到的字节数。
        管道中无数据：
            写端被全部关闭，read返回0（相当于读到文件的末尾）
            写端没有完全关闭，read阻塞等待

    写管道：
        管道读端全部被关闭，进程异常终止（进程收到SIGPIPE信号）
        管道读端没有全部关闭：
            管道已满，write阻塞
            管道没有满，write将数据写入，并返回实际写入的字节数
*/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
/*
    设置管道非阻塞
    int flags = fcntl(fd[0], F_GETFL);  // 获取原来的flag
    flags |= O_NONBLOCK;            // 修改flag的值
    fcntl(fd[0], F_SETFL, flags);   // 设置新的flag
*/
int main() {

    // 在fork之前创建管道
    int pipefd[2];
    int ret = pipe(pipefd);
    if(ret == -1) {
        perror("pipe");
        exit(0);
    }

    // 创建子进程
    pid_t pid = fork();
    if(pid > 0) {
        // 父进程
        printf("i am parent process, pid : %d\n", getpid());

        // 关闭写端
        close(pipefd[1]);
        
        // 从管道的读取端读取数据
        char buf[1024] = {0};

        int flags = fcntl(pipefd[0], F_GETFL);  // 获取原来的flag
        flags |= O_NONBLOCK;            // 修改flag的值
        fcntl(pipefd[0], F_SETFL, flags);   // 设置新的flag

        while(1) {
            int len = read(pipefd[0], buf, sizeof(buf));
            printf("len : %d\n", len);
            printf("parent recv : %s, pid : %d\n", buf, getpid());
            memset(buf, 0, 1024);
            sleep(1);
        }

    } else if(pid == 0){
        // 子进程
        printf("i am child process, pid : %d\n", getpid());
        // 关闭读端
        close(pipefd[0]);
        char buf[1024] = {0};
        while(1) {
            // 向管道中写入数据
            char * str = "hello,i am child";
            write(pipefd[1], str, strlen(str));
            sleep(5);
        }
        
    }
    return 0;
}



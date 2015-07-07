#include <unistd.h>
#include <sys/param.h>
#include <stdio.h>
int main()
{
    char path[MAXPATHLEN];
    if(!getcwd(path, sizeof(path)))
        return -1;
    puts(path);
    return 0;
}

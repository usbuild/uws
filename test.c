/*列出 /etc/X11 目录下的子目录*/

#include <sys/stat.h>

#include <unistd.h>

#include <ftw.h>
#include <stdio.h>

int fn(const char *file, const struct stat *sb, int flag)

{

      printf("%s -- directory\n", file);

      return 0;

}

int main()

{

     ftw("/etc/X11", fn, 500);

}

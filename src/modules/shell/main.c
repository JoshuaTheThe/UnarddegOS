
#include <vfs/vnode.h>
#include <vfs/vfd.h>
#include <vmem/alloc.h>

/**
 * Commands - all create a new process (that exit on completion)
 *      exit    exit the shell
 *      print   print the stdin
 *      ls      list wd
 *      cd      change wd
 *      pwd     print wd
 *      touch   create dir/file
 *      rm      delete dir/file
 *      pipe    pipe first args output to seconds input
 */

int Init(void)
{
        return 0;
}

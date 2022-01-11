I used the fs pointer to format the filesystem by assigning struct pointers to
the fs pointer and editing the indices of those struct pointers as needed.
this was done with a separate pointer that was incremented between the address it contains
was assigned to a struct pointers as needed.
To list the contents of the directory I used a recursive function that cycles through by
checking each index of the blocks array for each inode that is not marked as free 
 while keeping track of the number of tabs needed with a function variable rec_count,
 which is passed as rec_count + 1 upon entering a recursive call. Although extract works 
 just fine, upon running with the -l option every directory entry file name is replaced with
 the name of the file that contains the filesystem.
 For addfs I used strtok to split the path and either find the file represented by the generated
 token tok, or create a new entry/file depending on the value of tok_next which is the original
 path passed into the function incremented by sizeof(tok) + 1. tok_next is passed into the function
 as path upon recursion and once it is equal to "", the file represented by tok is opened using a
 file descriptor, the size of the file's inode is grabbed through fstat and blocks are witten 
 to using a read buffer until the last block is smaller than block size or a counter reaches the
 number of blocks needed and when a block is written to, the current index belonging to the
 blocks array in the current inode is set to an integer representing the index of the block
 that was written to as to avoid contiguous allocation.
 Remove works similar to add in finding files using recursion and exiting the recursive call 
 if tok_next is equal to "" after setting all FBL->free_list values represented by the inode's 
 blocks array to free as well as the inode belonging to the file, the indices of the blocks array 
 in the inode are all set to -1. once recursion is exited, the inode->blocks index that held
 the found directory entry is set to -1 after setting the block it was stored in to free
 then all of the current inode->block indices are checked and if all are equal to -1 
 the node is cleared as stated above.
 extract finds the file in the same manner and if tok_next is equal to "", each block represented in
 the file's inode is printed to stdout. This was originally implemented by opening a file and
 writing to it because I initially didn't see the part about printing to stdout and this approach
 works but after changing it to print to stdout I get a segfault running the -e command outside
 of gdb but not when I run it in gdb although it prints nothing.

 add also has problems writing to blocks that previously contained a file which was removed.

 I worked alone so no point in a group.txt
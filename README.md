# Exec-Loader
The loader will load the executable into memory page by page, using a demand paging mechanism - a page will only be loaded when it is needed.
To run an executable, the loader will perform the following steps:

- It will initialize its internal structures.
- It will parse the binary file - to do this you have an ELF file parser on Linux in the theme skeleton. Find more details in the section describing the executable parser interface .
- It will run the first instruction of the executable ( the entry point ).
during execution, a page fault will be generated for each access to a non-memory-mapped page;
- It will detect each access to an unmapped page, and check which segment of the executable it belongs to.
      1. if it is not found in a segment, it means that it is an invalid memory access – the default page fault handler is run ;
      2.if the page fault is generated in an already mapped page, then an unauthorized memory access is attempted (the respective segment does not have the necessary permissions) – likewise, the default page fault handler is run ;
      3.if the page is found in a segment, and it has not yet been mapped, then it is mapped to the related address, with the permissions of that segment;
- Usage of mmap function (Linux) to allocate virtual memory within the process.
- The page must be fixedly mapped to the address indicated within the segment.

# Library interface
The user interface of the loader library is presented in the header file loader.h. It contains functions to initialize the loader( so_init_loader) and execute the binary ( so_execute).
The function so_init_loaderperforms library initialization. The function will generally register the page fault handler in the form of a routine for handling the SIGSEGV signal or an exception handler .
The function so_executeperforms parsing of the binary specified in pathand running the first instruction ( entry point ) in the executable.

# Parser Interface
The parser interface provides two functions:

so_parse_exec - parses the executable and returns a structure of type so_exec_t. This can further be used to identify executable segments and its attributes.
so_start_exec - prepares the program environment and starts its execution.
From now on, page faults will be executed for every new/unmapped page access.
The structures used by the interface are:

- so_exec_t - describes the structure of the executable:

     1.base_addr- indicates the address where the executable should be loaded
     
     2.entry- the address of the first instruction executed by the executable
     
    3.segments_no- the number of segments in the executable
    
    4.segments- a vector (of size segments_no) containing the segments of the executable
    
- so_seg_t - describes a segment within the executable

    1.vaddr- the address where the segment should be loaded
    
    2.file_size- the within-file size of the segment
    
    3.mem_size- the size occupied by the segment in memory; the size of the segment in memory may be larger than the size in the file (for example for the segment bss); in this case, the difference between memory space and file space must be zeroed
    
    4.offset- the offset within the file at which the segment begins
    
    5.perm- a bitmask representing the permissions that pages in the current segment must have
    
    6.PERM_R- read permissions
    
    7.PERM_W- write permissions
    
    8.PERM_X- execution permissions
    
    9.data- an opaque pointer that you can use to attach your own information related to the current segment (for example, you can store here information about already mapped pages in the segment)

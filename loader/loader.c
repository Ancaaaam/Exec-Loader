/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/mman.h>
#include<fcntl.h>
#include "exec_parser.h"

static so_exec_t *exec;

static int fd;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	/* TODO - actual loader implementation */
	int i, memory_ok = 0;
	char *address = (char *)info->si_addr;

	for (i = 0; i < exec->segments_no; i++)
		if ((char *)exec->segments[i].vaddr+exec->segments[i].mem_size > address && address >= (char *)(exec->segments[i].vaddr)) {
			//am gasit intr-un segment adresa problematica
			memory_ok = 1;
			break;
		}
	if (memory_ok == 0) {
		exit(139);
		return;
	}
	for (i = 0; i < exec->segments_no; i++)
		if ((char *)exec->segments[i].vaddr+exec->segments[i].mem_size > address && address >= (char *)(exec->segments[i].vaddr)) {
			int size = ((char *)info->si_addr-(char *)exec->segments[i].vaddr)/getpagesize();
			int fsize = size*getpagesize();
			int *d = (int *)exec->segments[i].data;
			//daca pagina a fost mapata, direct seg fault
			if (d[size] == 1)
				exit(139);
			else {
				int minim = 0;
				//mapam pagina
				char *p = mmap((char *)(exec->segments[i].vaddr)+fsize, getpagesize(), PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_SHARED, fd, 0);

				if (p == MAP_FAILED)
					exit(-1);
				if (fsize < exec->segments[i].file_size) {
					//determinam cat trebuie citit din fisier
					if (getpagesize() < (int)exec->segments[i].file_size-fsize)
						minim = getpagesize();
					else
						minim = (int)exec->segments[i].file_size-fsize;
				}
				lseek(fd, exec->segments[i].offset+fsize, SEEK_SET);
				read(fd, p, minim);
				//schimbam permisiunile cu permisiunile segmentului
				int r = mprotect(p, getpagesize(), exec->segments[i].perm);

				if (r == -1)
					exit(-1);
				d[size] = 1;
			}
		}
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	//deschidem fisierul
	fd = open(path, O_RDONLY);
	int i, no_pages;
	//adaugam memorie in sectiunea data ca sa salvam in data ce pagini avem mapate
	for (i = 0; i < exec->segments_no;i++) {
		float nr = (float)exec->segments[i].mem_size/getpagesize();

		no_pages = (int)nr+1;
		exec->segments[i].data = (int *)calloc(no_pages+1, sizeof(int));
	}
	if (!exec)
		return -1;
	so_start_exec(exec, argv);
	//eliberam memoria alocata
	for (i = 0; i < exec->segments_no; i++)
		free(exec->segments[i].data);
	close(fd);
	return -1;
}

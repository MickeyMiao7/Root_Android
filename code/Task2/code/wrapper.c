#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char** environ;

int main(int argc, char** argv){
	FILE* f = fopen("/system/dummy2", "w");
	if (f == NULL){
		printf("Permission Denied\n");
		exit(EXIT_FAILURE);
	}
	fclose(f);
	
	char* cmd = "/system/bin/app_process_original";
	execve(cmd, argv, environ);
	
	return EXIT_FAILURE;
}

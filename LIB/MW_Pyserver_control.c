/* Copyright 2019 The MathWorks, Inc. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdint.h>
#include "MW_Pyserver_control.h"


#define PID_LEN 8

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

static char exePath[65534+1];
static char pyserverLoc[65534+1];

void MW_getDefines()
{
    char pwd[65535];
    char *p;
    
    readlink("/proc/self/exe", pwd, 65535);
    
    strcpy(exePath, pwd);
    if((p = strrchr(exePath, '/')))
        *(p+1) = '\0';
    printf("PAth = %s\n",exePath);
    sprintf(pyserverLoc,"%s%s_ert_rtw/MW_pyserver.py", exePath, TOSTRING(MODEL));
    printf("PyserverLocation = %s\n",pyserverLoc);
    
}


void MW_killPyserver()
{
#ifdef MW_WEBSOCPUB_INUSE
    printf(" Attempt to kill\n ");
    fflush(stdout);
    char buff[100];
    sprintf(buff, "pgrep -f %s|grep -v grep|awk '{print $1}'|xargs sudo kill ", pyserverLoc);
    MW_execmd(buff);
#endif
}

void MW_launchPyserver()
{
#ifdef MW_WEBSOCPUB_INUSE
    char cmd[255];
    MW_getDefines();
    sprintf(cmd, "sudo %s %s %d > /dev/null &", pyserverLoc, TOSTRING(MODEL), MW_WESOCPUB_NUM);
    printf("Launching server at %s\n", cmd);
    MW_execmd(cmd);
#endif
}


void MW_execmd(char *buff)
{
    int ret = 0,bufferLoc = 0;
    FILE *fp, *fd;
    char buffer[65535];
    size_t readLength = 0;
    uint32_t outSize=65535;
    
    fd = fopen("/tmp/pylog.txt","w");
    fprintf(fd,"%s\n", buff);
    fclose(fd);
    
    fp = popen(buff,"r");
    if (fp == NULL){
        printf("Error in executing the cmd \n");
        return;
    }
    
    /*Read the cmd output if any */
    while (fgets((buffer+bufferLoc),(outSize - bufferLoc),fp) != NULL){
        readLength = strlen(buffer+bufferLoc);
        
        if((bufferLoc+readLength) < outSize){
            /*Advance the pointer to new location */
            bufferLoc += readLength;
        }
        else{
            /*cmd output exceeds outSize chars*/
            break;
        }
    }
    
    printf("%s",buffer);
    
    return ;
    
}

/* LocalWords:  PAth pyserver pgrep awk xargs sudo dev tmp pylog
 */

/* @(#) $Id$ */

/* Copyright (C) 2006 Daniel B. Cid <dcid@ossec.net>
 * All rights reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */
       

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "os_regex/os_regex.h"

#define OSSECCONF   "ossec.conf"
#define OSSECDEF    "default-ossec.conf"
#define CLIENTKEYS  "client.keys"
#define OS_MAXSTR   1024

int fileexist(char *file)
{
    FILE *fp;

    /* Opening file */
    fp = fopen(file, "r");
    if(!fp)
        return(0);

    fclose(fp);
    return(1);
}

int dogrep(char *file, char *str)
{
    char line[OS_MAXSTR +1];
    FILE *fp;

    /* Opening file */
    fp = fopen(file, "r");
    if(!fp)
        return(0);

    /* Clearing memory */
    memset(line, '\0', OS_MAXSTR +1);

    /* Reading file and looking for str */ 
    while(fgets(line, OS_MAXSTR, fp) != NULL)
    {
        if(OS_Match(str, line))
        {
            fclose(fp);
            return(1);
        }
    }

    fclose(fp);
    return(0);
}



/* Check is syscheck is present in the config */
int config_syscheck()
{
    FILE *fp;

    /* We add here the last entry */
    if(dogrep(OSSECCONF, "iis6.log</ignore>"))
    {
        return(0);
    }
    
    /* Add syscheck config */
    fp = fopen(OSSECCONF, "a");
    if(!fp)
        return(0); 
   
    fprintf(fp, "%s\r\n", 
    "\r\n"    
    "<!-- Updated syscheck config -->\r\n"
    "<ossec_config>\r\n"
    "  <syscheck>\r\n"
    "    <frequency>21600</frequency>\r\n"
    "    <ignore>C:\\WINDOWS/System32/LogFiles</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/WindowsUpdate.log</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/system32/wbem/Logs</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/Prefetch</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/Debug</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/PCHEALTH/HELPCTR/DataColl</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/SoftwareDistribution</ignore>\r\n"
    "    <ignore>C:\\Program Files/ossec-agent</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/Temp</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/SchedLgU.Txt</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/system32/config</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/system32/CatRoot</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/system32/wbem/Repository</ignore>\r\n"
    "    <ignore>C:\\WINDOWS/iis6.log</ignore>\r\n"
    "  </syscheck>\r\n"
    "</ossec_config>\r\n");

    fclose(fp);

    return(0);
                    
}

/* Setup windows after install */
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("%s: Invalid syntax.\n", argv[0]);
        printf("Try: '%s directory'\n\n", argv[0]);
        return(0);
    }
    
    if(chdir(argv[1]) != 0)
    {
        printf("%s: Invalid directory: '%s'.\n", argv[0], argv[1]);
        return(0);
    }
    
    /* Checking if ossec was installed already */
    if(!fileexist(OSSECCONF))
    {
        char cmd[OS_MAXSTR +1];
        
        /* Copy default config to ossec.conf */
        snprintf(cmd, OS_MAXSTR, "copy %s %s", OSSECDEF, OSSECCONF);
        system(cmd);

        /* Run iis-logs */
        snprintf(cmd, OS_MAXSTR, "setup-iis.exe .");
        system(cmd);


        /* Run manage agents */
        snprintf(cmd, OS_MAXSTR, "manage_agents.exe");
        system(cmd);
    }

    /* If it is present, we need to do the upgrade */
    else
    {
        char cmd[OS_MAXSTR +1];

        
        /* Look if syscheck is configured, if not, do so */
        config_syscheck();


        /* Call manage-agents if no key */
        if(!fileexist(CLIENTKEYS))
        {
            /* Run manage agents */
            snprintf(cmd, OS_MAXSTR, "manage_agents.exe");
            system(cmd);
        }


        /* Run iis-logs here too */
        snprintf(cmd, OS_MAXSTR, "setup-iis.exe .");
        system(cmd);

    }


    /* Setting up local files */
    system("add-localfile.exe \"C:\\Windows\\pfirewall.log\"");
    

    /* Configure ossec for automatic startup */
    system("sc config OssecSvc start= auto");
    return(0);
}

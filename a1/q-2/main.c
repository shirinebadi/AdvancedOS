#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <time.h>

struct sysinfo info;

#define DISK_NAME "sda"

void default_run(){
    printf("Processor Type:\n");
    system("cat /proc/cpuinfo | awk 'NR==5'");
    printf("\nKernel Version\n");
    system("cat /proc/sys/kernel/osrelease");
    printf("\nConfigured Memory: \n");
    system("cat /proc/meminfo | awk 'NR==1'");
    printf("\nAmount of time since the system was last booted: \n");
    system("cat /proc/uptime");
    printf("Virtual Memory Mapping:\n");
    system("cat /proc/iomem | grep 'System RAM'");

    // Display virtual memory address range mapped to Kernel code
    system("cat /proc/iomem | grep 'Kernel code'");

    // Display virtual memory address range mapped to Kernel data
    system("cat /proc/iomem | grep 'Kernel data'");

    // Display virtual memory address range mapped to Kernel bss
    system("cat /proc/iomem | grep 'Kernel bss'");
    printf("Available Memory:\n");
    system("cat /proc/meminfo | grep 'MemAvailable'");
}



void read_cpu(unsigned long long *user, unsigned long long *system, unsigned long long *idle){
    FILE * fp = fopen("/proc/stat", "r");

    unsigned long long processes_created;
    char * line = NULL;
    size_t len = 0;
    int read;
    read = getline(&line, &len, fp);
    unsigned long long user_time, nice_time, system_time, idle_time, iowait_time;
    sscanf(line, "cpu %llu %llu %llu %llu %llu", &user_time, &nice_time, &system_time, &idle_time, &iowait_time);

    *user += user_time;
    *system += system_time;
    *idle += idle_time;
}

unsigned long long read_memory(){
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return 1;
    }
    unsigned long long free_memory = info.freeram * info.mem_unit;
    return free_memory;
}

unsigned long long read_disck(){
    FILE * fp = fopen("/proc/diskstats", "r");

    unsigned long long sectors;
    char * line = NULL;
    size_t len = 0;
    int read;
    while ((read = getline(&line, &len, fp)) != -1) {
        int major, minor;
        char device[32];
        unsigned long long read_ios, read_merges, read_sectors, read_time;
        unsigned long long write_ios, write_merges, write_sectors, write_time;
        unsigned long long io_in_progress, io_time, weighted_io_time;
        unsigned long long discards, discard_merges, discard_sectors, discard_time;
        unsigned long long flush_request, time_flush;

        int ret = sscanf(line, "%d %d %s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
            &major, &minor, device,
            &read_ios, &read_merges, &read_sectors, &read_time,
            &write_ios, &write_merges, &write_sectors, &write_time,
            &io_in_progress, &io_time, &weighted_io_time,
            &discards, &discard_merges, &discard_sectors, &discard_time,
            &flush_request, &time_flush
        );

        //printf(" %s \n", line);

        if (strcmp(device, DISK_NAME) == 0) {
            //tf("write Disck %llu \n", write_sectors);
            sectors = write_sectors + read_sectors;
            break; 
        }

}
 return sectors;
}

unsigned long long read_context_switch(){
    FILE * fp = fopen("/proc/stat", "r");

    unsigned long long context_switches;
    char * line = NULL;
    size_t len = 0;
    int read;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (strncmp(line, "ctxt ", 5) == 0) {
            sscanf(line, "ctxt %llu", &context_switches);
            break;  // Found the line with context switches, exit the loop
        }
    }

    return context_switches;
}

unsigned long long read_process_creation(){
    FILE * fp = fopen("/proc/stat", "r");

    unsigned long long processes_created;
    char * line = NULL;
    size_t len = 0;
    int read;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (strncmp(line, "processes ", 10) == 0) {
            sscanf(line, "processes %llu", &processes_created);
            break;  // Found the line with process creation count, exit the loop
        }
    }

    return processes_created;
}

void eternal_mode(int read_rate, int print_rate){
    int interval = print_rate/read_rate;

    unsigned long long user, system, idle = 0;

    unsigned long long free_memory1 = read_memory();
    unsigned long long free_memory2;
    unsigned long long free_memory;

    unsigned long long disk1 = read_disck();
    unsigned long long disk2 = 0;
    int disk_number = 0;

    unsigned long long context_switch1 = read_context_switch();
    unsigned long long context_switch2 = 0;
    int contex_number = 0;

    unsigned long long process1 = read_process_creation();
    unsigned long long process2 = 0;
    int process_creation = 0;

    time_t sec = (time_t)print_rate;

    while(1){
        time_t startTime = time(NULL);
        while (time(NULL) - startTime < sec){
            read_cpu(&user,&system,&idle);
            free_memory2 = read_memory();

            if (free_memory2 - free_memory1 < 0)
                free_memory2 = free_memory1;

            free_memory += free_memory2 - free_memory1;
            free_memory1 = free_memory2;

            disk2 = read_disck();
            disk_number += disk2 - disk1;
            disk1 = disk2;

            context_switch2 = read_context_switch();
            contex_number += context_switch2 - context_switch1;
            context_switch1 = context_switch2;

            process2 = read_process_creation();
            process_creation += process2 - process1 ;
            process1 = process2;
            printf("\n Reading ... \n");
            sleep(read_rate);
            
        }
        printf("%llu Processor Time in user mode, %llu in kernel mode,%llu in idle mode\n", user/interval,system/interval, idle/interval);
        printf("%llu Free Memory\n", free_memory/interval);
        printf("%llu Read + Write Sectors(Disk)\n", disk_number);
        printf("%llu Number of Context Switch\n", contex_number/interval);
        printf("%llu New Process Created\n", process_creation/interval);
    }
}

void main(int argc, char **argv[]){

    printf("Start\n");
    int read = 0;
    int write = 0;
    if (argc == 3){
        read = atoi(argv[1]);
        write = atoi(argv[2]);
    }
    if (argc == 1) {
        default_run();
    }
    else {
        eternal_mode(read, write);
    }
}





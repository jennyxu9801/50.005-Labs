#include "processManagement_lab.h"

/**
 * The task function to simulate "work" for each worker process
 * TODO#3: Modify the function to be multiprocess-safe 
 * */
void task(long duration)
{
    // simulate computation for x number of seconds
    usleep(duration*TIME_MULTIPLIER);
    sem_wait(sem_global_data);
    // TODO: protect the access of shared variable below
    // update global variables to simulate statistics
    ShmPTR_global_data->sum_work += duration;
    ShmPTR_global_data->total_tasks ++;
    if (duration % 2 == 1) {
        ShmPTR_global_data->odd++;
    }
    if (duration < ShmPTR_global_data->min) {
        ShmPTR_global_data->min = duration;
    }
    if (duration > ShmPTR_global_data->max) {
        ShmPTR_global_data->max = duration;
    }

    sem_post(sem_global_data);
}


/**
 * The function that is executed by each worker process to execute any available job given by the main process
 * */
/*
void job_dispatch(int i){
    // TODO#3:  a. Always check the corresponding shmPTR_jobs_buffer[i] for new  jobs from the main process
    sem_wait(sem_jobs_buffer[i]);

    if(shmPTR_jobs_buffer[i].task_status == 1){
        
        char taskType = shmPTR_jobs_buffer[i].task_type;
        if(taskType == 't'){
            shmPTR_jobs_buffer[i].task_status=0;
            task(shmPTR_jobs_buffer[i].task_duration);
            
            sem_post(sem_jobs_buffer[i]);
            
        }else if(taskType == 'w'){
            shmPTR_jobs_buffer[i].task_status=0;
            usleep(shmPTR_jobs_buffer[i].task_duration*TIME_MULTIPLIER);
            
            sem_post(sem_jobs_buffer[i]);
            
        }else if(taskType =='z'){
            shmPTR_jobs_buffer[i].task_status = -1;
            
            exit(3);
            sem_post(sem_jobs_buffer[i]);
        }else if (taskType =='i'){
            shmPTR_jobs_buffer[i].task_status = -1;
            
            kill(getpid(),SIGKILL);
            sem_post(sem_jobs_buffer[i]);
        }
    }else{
        sem_post(sem_jobs_buffer[i]);
    }
    
    //          b. Use semaphore so that you don't busy wait
    //          c. If there's new job, execute the job accordingly: either by calling task(), usleep, exit(3) or kill(getpid(), SIGKILL)
    //          d. Loop back to check for new job 


    //printf("Hello from child %d with pid %d and parent id %d\n", i, getpid(), getppid());
    //exit(0); 

}*/
void job_dispatch(int i){
    

    // TODO#3:  a. Always check the corresponding shmPTR_jobs_buffer[i] for new  jobs from the main process
    //          b. Use semaphore so that you don't busy wait
    //          c. If there's new job, execute the job accordingly: either by calling task(), usleep, exit(3) or kill(getpid(), SIGKILL)
    //          d. Loop back to check for new job 



    //printf("Hello from child %d with pid %d and parent id %d\n", i, getpid(), getppid());
    //exit(0); 
    while(true){
        sem_wait(sem_jobs_buffer[i]);
        if(shmPTR_jobs_buffer[i].task_status == 1){
        
        char taskType = shmPTR_jobs_buffer[i].task_type;
        if(taskType == 't'){
            shmPTR_jobs_buffer[i].task_status=0;
            task(shmPTR_jobs_buffer[i].task_duration);
            
            sem_post(sem_jobs_buffer[i]);
            
        }else if(taskType == 'w'){
            shmPTR_jobs_buffer[i].task_status=0;
            usleep(shmPTR_jobs_buffer[i].task_duration*TIME_MULTIPLIER);
            
            sem_post(sem_jobs_buffer[i]);
            
        }else if(taskType =='z'){
            shmPTR_jobs_buffer[i].task_status = -1;
            
            exit(3);
            sem_post(sem_jobs_buffer[i]);
        }else if (taskType =='i'){
            shmPTR_jobs_buffer[i].task_status = -1;
            
            kill(getpid(),SIGKILL);
            sem_post(sem_jobs_buffer[i]);
        }
    }

         
        else{
            sem_post(sem_jobs_buffer[i]);
        } 
    }
}

/** 
 * Setup function to create shared mems and semaphores
 * **/
void setup(){
    //printf("setup\n");
    // TODO#1:  a. Create shared memory for global_data struct (see processManagement_lab.h)

    ShmID_global_data = shmget(IPC_PRIVATE, sizeof(global_data), IPC_CREAT | 0666);
    if (ShmID_global_data == -1){
        perror("Global data shared memory creation failed\n");
        exit(EXIT_FAILURE);
    }
    ShmPTR_global_data = (global_data *) shmat(ShmID_global_data, NULL, 0);
    if ((int) ShmPTR_global_data == -1){
        perror("Attachment of global data shared memory failed \n");
        exit(EXIT_FAILURE);
    }
    //          b. When shared memory is successfully created, set the initial values of "max" and "min" of the global_data struct in the shared memory accordingly
    // To bring you up to speed, (a) and (b) are given to you already. Please study how it works. 

    //set global data min and max
    ShmPTR_global_data->max = -1;
    ShmPTR_global_data->min = INT_MAX;
    
    
    //          c. Create semaphore of value 1 which purpose is to protect this global_data struct in shared memory 
    sem_global_data = sem_open("semglobaldata", O_CREAT| O_EXCL,0644,1 );
    while(true){
        if (sem_global_data == SEM_FAILED){
            sem_unlink("semglobaldata");
            sem_global_data = sem_open("semglobaldata", O_CREAT| O_EXCL,0644,1 );
        }else{
            break;
        }
    }

    //          d. Create shared memory for number_of_processes job struct (see processManagement_lab.h)
    ShmID_jobs = shmget(IPC_PRIVATE, sizeof(job)*number_of_processes, IPC_CREAT | 0666);
    if (ShmID_jobs == -1){
        perror("Jobs shared memory creation failed\n");
        exit(EXIT_FAILURE);
    }
    shmPTR_jobs_buffer = (job *) shmat(ShmID_jobs, NULL, 0);
    if ((int) shmPTR_jobs_buffer == -1){
        perror("Attachment of jobs shared memory failed \n");
        exit(EXIT_FAILURE);
    }

    //          e. When shared memory is successfully created, setup the content of the structs (see handout)
    shmPTR_jobs_buffer->task_status = 0;

    //          f. Create number_of_processes semaphores of value 0 each to protect each job struct in the shared memory. Store the returned pointer by sem_open in sem_jobs_buffer[i]
    for (int i=0; i<number_of_processes;i++){
        char semjob[10];
        sprintf(semjob,"%s%d","semjobs",i);
        
        sem_jobs_buffer[i] = sem_open(semjob, O_CREAT| O_EXCL,0644,0 );

        while(true){
       
            if (sem_jobs_buffer[i] == SEM_FAILED){
                sem_unlink(semjob);
                 sem_jobs_buffer[i] = sem_open(semjob, O_CREAT| O_EXCL,0644,0);
             }
            else{
                break;
             }  
         }
    }
    //          g. Return to main
    return;

}

/**
 * Function to spawn all required children processes
 **/
 
void createchildren(){
    //printf("createchildren\n");
    // TODO#2:  a. Create number_of_processes children processes
    pid_t pid;
    for(int i=0;i<number_of_processes;i++) // loop will run n times (n=5) 
    {  pid = fork();
        if(pid<0){
            printf("error\n");
            exit(-1);
        }else if(pid==0){
            //children_processes[i] = getpid();
            //printf("hellllllllo");
            //printf("%d\n",children_processes[i]);
            job_dispatch(i);
            
            
        }else{
            children_processes[i] =pid;
            //printf("hellllllllo");
            //printf("%d\n",children_processes[i]);
        
            //wait(NULL);
            //exit(0);
        }
    
        
           // printf("[parent] pid %d child\n",getpid()); 
            //exit(0);
        
    } 
    //for(int i=0;i<number_of_processes;i++) // loop will run n times (n=5) 
    //wait(NULL);
    //          c. For child process, invoke the method job_dispatch(i)
    //          d. For the parent process, continue creating the next child
    //          e. After number_of_processes children are created, return to main 

    return;
}

/**
 * The function where the main process loops and busy wait to dispatch job in available slots
 * */
void main_loop(char* fileName){

    // load jobs and add them to the shared memory
    FILE* opened_file = fopen(fileName, "r");
    char action; //stores whether its a 'p' or 'w'
    long num; //stores the argument of the job 

    while (fscanf(opened_file, "%c %ld\n", &action, &num) == 2) { //while the file still has input
 
        //TODO#4: create job, busy wait
        //      a. Busy wait and examine each shmPTR_jobs_buffer[i] for jobs that are done by checking that shmPTR_jobs_buffer[i].task_status == 0. You also need to ensure that the process i IS alive using waitpid(children_processes[i], NULL, WNOHANG). This WNOHANG option will not cause main process to block when the child is still alive. waitpid will return 0 if the child is still alive. 
        //      b. If both conditions in (a) is satisfied update the contents of shmPTR_jobs_buffer[i], and increase the semaphore using sem_post(sem_jobs_buffer[i])
        //      c. Break of busy wait loop, advance to the next task on file 
        //      d. Otherwise if process i is prematurely terminated, revive it. You are free to design any mechanism you want. The easiest way is to always spawn a new process using fork(), direct the children to job_dispatch(i) function. Then, update the shmPTR_jobs_buffer[i] for this process. Afterwards, don't forget to do sem_post as well 
        //      e. The outermost while loop will keep doing this until there's no more content in the input file. 
        while(true){
            
            int i=0;
            for(i=0;i<number_of_processes;i++){
      // printf("first for loop \n");
                int alive=waitpid(children_processes[i],NULL,WNOHANG);
                if(shmPTR_jobs_buffer[i].task_status==0&&alive==0){
                    shmPTR_jobs_buffer[i].task_status=1;
                    shmPTR_jobs_buffer[i].task_duration=num;
                    shmPTR_jobs_buffer[i].task_type=action;                    
                    sem_post(sem_jobs_buffer[i]);
                    break;
                }
                
                else if(alive!=0){
                    pid_t pid=fork();
                    children_processes[i]=pid;

                    if(pid==0){
                        job_dispatch(i);
                    }
                    else{
                        shmPTR_jobs_buffer[i].task_status=1;
                        shmPTR_jobs_buffer[i].task_duration=num;
                        shmPTR_jobs_buffer[i].task_type=action;
                        sem_post(sem_jobs_buffer[i]);
                        break;
                    }
                }
            }
            if(shmPTR_jobs_buffer[i].task_status==1){
                break;
            }
        }
    }
    fclose(opened_file);
    // TODO#4: Design a way to send termination jobs to ALL worker that are currently alive 
    
    while(true){
        int count =0;
        for(int i=0;i<number_of_processes;i++){
            //printf("second for loop\n");
            int alive=waitpid(children_processes[i],NULL,WNOHANG);
            if(shmPTR_jobs_buffer[i].task_status==0&&alive==0){
                shmPTR_jobs_buffer[i].task_duration=0;
                shmPTR_jobs_buffer[i].task_type='z';
                shmPTR_jobs_buffer[i].task_status=1;
                sem_post(sem_jobs_buffer[i]);
            }
            if(shmPTR_jobs_buffer[i].task_status==-1){
                count++;
            }
            
         }

         if(count == number_of_processes){
             break;
         }
    }
    

    //wait for all children processes to properly execute the 'z' termination jobs
    int process_waited_final = 0;
    pid_t wpid;
    while ((wpid = wait(NULL)) > 0){
        process_waited_final ++;
    }
    // print final results
    printf("Final results: sum -- %ld, odd -- %ld, min -- %ld, max -- %ld, total task -- %ld\n", ShmPTR_global_data->sum_work, ShmPTR_global_data->odd, ShmPTR_global_data->min, ShmPTR_global_data->max, ShmPTR_global_data->total_tasks);
}


void cleanup(){
    //printf("cleanup\n");
    //TODO#4: 
    // 1. Detach both shared memory (global_data and jobs)
    // 2. Delete both shared memory (global_data and jobs)
    // 3. Unlink all semaphores in sem_jobs_buffer
    int detach_status = shmdt((void *) ShmPTR_global_data); 

    int remove_status = shmctl(ShmID_global_data, IPC_RMID, NULL); 
    detach_status = shmdt((void *) shmPTR_jobs_buffer); 
    remove_status = shmctl(ShmID_jobs, IPC_RMID, NULL); 

    int sem_close_status = sem_unlink("semglobaldata");


    for (int i = 0; i<number_of_processes; i++){
        char *sem_name = malloc(sizeof(char)*16);
        sprintf(sem_name, "semjobs%d", i);
        sem_close_status = sem_unlink(sem_name);
        free(sem_name);
    }
}


// Real main
int main(int argc, char* argv[]){

    //printf("Lab 1 Starts...\n");

    struct timeval start, end;
    long secs_used,micros_used;

    //start timer
    gettimeofday(&start, NULL);

    //Check and parse command line options to be in the right format
    if (argc < 2) {
        printf("Usage: sum <infile> <numprocs>\n");
        exit(EXIT_FAILURE);
    }


    //Limit number_of_processes into 10. 
    //If there's no third argument, set the default number_of_processes into 1.  
    if (argc < 3){
        number_of_processes = 1;
    }
    else{
        if (atoi(argv[2]) < MAX_PROCESS) number_of_processes = atoi(argv[2]);
        else number_of_processes = MAX_PROCESS;
    }

    setup();
    createchildren();
    main_loop(argv[1]);

    //parent cleanup
    cleanup();

    //stop timer
    gettimeofday(&end, NULL);

    double start_usec = (double) start.tv_sec * 1000000 + (double) start.tv_usec;
    double end_usec =  (double) end.tv_sec * 1000000 + (double) end.tv_usec;

    printf("Your computation has used: %lf secs \n", (end_usec - start_usec)/(double)1000000);


    return (EXIT_SUCCESS);
}
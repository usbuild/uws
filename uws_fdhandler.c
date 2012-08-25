#include <pthread.h>
#include <semaphore.h>
#include "uws_config.h"
#include "uws_router.h"
#include "uws_header.h"
#include "uws_fdhandler.h"
#include "uws_utils.h"
#include "uws_datatype.h"
#define MAX_THREADS 1
struct thread_info{
    int client_sockfd;
};
static int_queue_t* fd_queue;
static sem_t sem_queue;
static pthread_mutex_t lock;
static bool is_first_run = true;

void deal_client_fd(client_sockfd)
{
    char line[BUFF_LEN] = "",
         type[10],
         httpver[10];
    int i = 0;

    FILE *input_file = fdopen(client_sockfd, "r+"); 

    fgets(line, BUFF_LEN, input_file);
    struct http_header request_header;
    request_header.path = (char*)calloc(PATH_LEN, sizeof(char));
    request_header.params = NULL;
    request_header.request_params = (char*)calloc(PATH_LEN, sizeof(char));

    sscanf(line, "%[^ ]%*[ ]%[^ ]%*[ ]%[^ \n]", type, request_header.path, httpver);
    request_header.method = type;
    request_header.url = (char*) calloc(strlen(request_header.path) + 1, sizeof(char)); //max index filename length
    strcpy(request_header.url, request_header.path);
    request_header.http_ver = httpver;

    while(fgets(line, BUFF_LEN, input_file) != NULL) {
        if(strcmp(line, "\r\n") != 0) {
            add_header_param(line, &request_header);
        }
        else {
            break;
        }
    }
    char* host = get_header_param("Host", &request_header);

    if(host != NULL) 
    {
        i = 0;
        while(uws_config.http.servers[i] != NULL) {
            if(wildcmp(uws_config.http.servers[i]->server_name, host) == 1) {
                //We've got a file regiestered in the config file;
                running_server = uws_config.http.servers[i];
                break;
            }
            i++;
        }
        if(running_server != NULL) {
            pathrouter(client_sockfd, &request_header);
        }
    }
    //
    fclose(input_file);//if we don't close file, will cause memory leak
    close(client_sockfd);
    free(request_header.url);
    free(request_header.path);
    free(request_header.request_params);
    free_header_params(&request_header);
}
void* thread_unit() {
    while(1) {
        sem_wait(&sem_queue);//wait for new connections
        pthread_mutex_lock(&lock);
        int fd = pop_int_queue(fd_queue);
        pthread_mutex_unlock(&lock);

        pthread_t tid;
        tid = pthread_self();

        printf("%lu handle: %d\n", tid, fd);
        deal_client_fd(fd);
    }
    return NULL;
}
void init_threadpool() {
    int i = 0, err;
    for(i = 0; i < MAX_THREADS; i++) {
        pthread_t ntid;
        err = pthread_create(&ntid, NULL, thread_unit, NULL);
        if(err != 0) {
            exit_err("Thread Pool");
        }
    }
}
void handle_client_fd(int client_sockfd) {
    //int err;
    //pthread_t ntid;
    //struct thread_info *info = (struct thread_info*)calloc(1, sizeof(struct thread_info));
    /*
    if(is_first_run) {
        int res;
        fd_queue = init_int_queue();
        res = sem_init(&sem_queue, 0, 0);
        if(res != 0) {
            exit_err("Init Sem");
        }
        init_threadpool();
        is_first_run = false;
    }
    pthread_mutex_lock(&lock);
    push_int_queue(fd_queue, client_sockfd);
    pthread_mutex_unlock(&lock);
    sem_post(&sem_queue); // post a semaphore to notify threads
    */
    deal_client_fd(client_sockfd);

    /*
     * e, it is not a good idea to use multi-thread, benchmark down, on my vmware 256M ubuntu
     */
    //err = pthread_create(&ntid, NULL, thread_unit, info);
    //if(err != 0) exit_err("Fdhandler Thread:");
    //thread_unit(info);//single thread
    //printf("original client_fd:%d\n", client_sockfd);
    //free(info);
    return;
}


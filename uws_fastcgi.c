#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "uws_memory.h"
#include "uws_fastcgi.h"
#include "uws_header.h"
#include "uws_config.h"
#include "uws_utils.h"
#include "uws_status.h"
#include "uws_router.h"
#include "uws_http.h"
#define PARAMS_BUFF_LEN     1024
#define MAX_BODY_LEN        2048


/**
 * flag definition
 * 0x00 not create socket
 * 0x01 not connect to fcgi server
 * 0x02 write header
 * 0x03 write stdin
 * 0x04 write header
 * 0x04 read response
 */


/*{{{*/
static char *
header_to_fcgi(const char *str) 
{
    int prefix_len = strlen("HTTP_");
    int len = strlen(str) + prefix_len + 2;
    char *newstr = (char*) uws_malloc(len * sizeof(char));
    int i = 0;
    strcpy(newstr, "HTTP_");
    while(str[i]) {
        if(str[i] == '-') {
            newstr[i + prefix_len] = '_';
        } else {
            newstr[i + prefix_len] = toupper(str[i]);
        }
        i++;
    }
    newstr[i+ prefix_len] = 0;
    return newstr;
}

static FCGI_Header
make_header(int type, int request_id, int content_len, int padding_len)
{
    FCGI_Header header;
    header.version          =           FCGI_VERSION_1;
    header.type             =           (unsigned char) type;
    header.requestIdB1      =           (unsigned char) ((request_id >> 8) & 0xff);
    header.requestIdB0      =           (unsigned char) (request_id & 0xff);
    header.contentLengthB1  =           (unsigned char) ((content_len >> 8) & 0xff);
    header.contentLengthB0  =           (unsigned char) (content_len & 0xff);
    header.paddingLength    =           (unsigned char) padding_len;
    header.reserved         =           0;
    return header;
}
static FCGI_BeginRequestBody
make_begin_request_body(int role, int keep_conn)
{
    FCGI_BeginRequestBody body;
    body.roleB1         =           (unsigned char) ((role >> 8) & 0xff);
    body.roleB0         =           (unsigned char) (role & 0xff);
    body.flags          =           (unsigned char) (keep_conn ? FCGI_KEEP_CONN : 0);
    bzero(body.reserved, sizeof(body.reserved));
    return body;
}

static void
build_name_value_body(char *name, int name_len, char *value, int value_len, unsigned char *body_buff, int *body_len)
{
    unsigned char *start_body_buff = body_buff;
    if( name_len < 0x80) {
        *body_buff++ = (unsigned char) name_len;
    } else {
        *body_buff++ = (unsigned char) ((name_len >> 24) | 0x80);
        *body_buff++ = (unsigned char) (name_len >> 16);
        *body_buff++ = (unsigned char) (name_len >> 8);
        *body_buff++ = (unsigned char) name_len;
    }

    if( value_len < 0x80) {
        *body_buff++ = (unsigned char) value_len;
    } else {
        *body_buff++ = (unsigned char) ((value_len >> 24) | 0x80);
        *body_buff++ = (unsigned char) (value_len >> 16);
        *body_buff++ = (unsigned char) (value_len >> 8);
        *body_buff++ = (unsigned char) value_len;
    }
    while(*name != '\0') *body_buff++ = *name++;
    while(*value != '\0') *body_buff++ = *value++;
    *body_len = body_buff - start_body_buff;
}

static void
add_fcgi_param(int request_id, char* name, char* value, memory_t *smem) {
    int name_len, value_len, body_len;
    unsigned char body_buff[PARAMS_BUFF_LEN];
    bzero(body_buff, PARAMS_BUFF_LEN);
    name_len        =           strlen(name);
    value_len       =           strlen(value);
    build_name_value_body(name, name_len, value, value_len, &body_buff[0], &body_len);

    FCGI_Header name_value_header;
    name_value_header = make_header(FCGI_PARAMS, request_id, body_len, 0);

    append_mem_t(smem, &name_value_header, FCGI_HEADER_LEN);
    append_mem_t(smem, body_buff, body_len);
}
static void
begin_build_request(int request_id, memory_t *smem) {
    FCGI_BeginRequestRecord begin_record;
    begin_record.header = make_header(FCGI_BEGIN_REQUEST, request_id, sizeof(begin_record.body), 0);
    begin_record.body = make_begin_request_body(FCGI_RESPONDER, 0);
    append_mem_t(smem, &begin_record, sizeof(begin_record));
}/*}}}*//*}}}*/

static void 
send_request(int sockfd, memory_t *smem) {
    writen(sockfd, smem->mem, smem->len);
}

static bool
read_response(int sockfd, memory_t *mem_file)
{
    int count;
    FCGI_Header response_header;

    //We defined max 1024*1024*2 byte per response
    int already_read = 1024 * 1024 * 2;

    unsigned char* content;
    int content_len;
    char tmp[8];
    bzero(mem_file, sizeof(mem_file));
    while(read(sockfd, &response_header, FCGI_HEADER_LEN) > 0) {

        if(mem_file->len >= already_read) return true;

        if(response_header.type == FCGI_STDOUT) {
            content_len = (response_header.contentLengthB1 << 8) + (response_header.contentLengthB0);
            content = (unsigned char*) uws_malloc(sizeof(char) * content_len);

            count = read(sockfd, content, content_len);

            append_mem_t(mem_file, content, content_len);

            uws_free(content);
            if(response_header.paddingLength > 0) {
                count = read(sockfd, tmp, response_header.paddingLength);
                if(count != response_header.paddingLength) perror("read response error");
            }
        }
        else if(response_header.type == FCGI_STDERR) {
            content_len = (response_header.contentLengthB1 << 8) + (response_header.contentLengthB0);
            content = (unsigned char*) uws_malloc(content_len * sizeof(char));
            count = read(sockfd, content, count);
            uws_free(content);

            if(response_header.paddingLength > 0) {
                count = read(sockfd, tmp, response_header.paddingLength);
                if(count != response_header.paddingLength) perror("read");
            }
        }
        else if(response_header.type == FCGI_END_REQUEST) {
            FCGI_EndRequestBody end_request;
            count = read(sockfd, &end_request, FCGI_HEADER_LEN);

            /*
               if(count != 8) perror("read");
               fprintf(stdout,"\nend_request:appStatus:%d,protocolStatus:%d\n",(end_request.appStatusB3<<24)+(end_request.appStatusB2<<16) +(end_request.appStatusB1<<8)+(end_request.appStatusB0),end_request.protocolStatus);
               */

        }
    }
    close(sockfd);
    return false;
}

typedef struct {
    memory_t *smem;
    int request_id;
    size_t mem_offset;
    size_t read_offset;
    size_t post;      // to determin if is post
    char fhost[20];
    char fport[10];
} FCGI_LOCAL_DATA;


void 
add_to_epoll(pConnInfo conn_info, uint32_t flag) {
        struct epoll_event ev;
        ev.events = flag | EPOLLET | EPOLLONESHOT;
        conn_info->status = CS_UPSTREAM_READ;
        ev.data.ptr = conn_info;
        if(epoll_ctl(conn_info->epollfd, EPOLL_CTL_ADD, conn_info->serverfd, &ev) == -1)
            exit_err("epoll_ctl");
}

void
fastcgi_router(pConnInfo conn_info) 
{
    FCGI_LOCAL_DATA *fdata;

    if(conn_info->flag == 0x00) {/*{{{*/
        if(conn_info->ptr == NULL) {
            conn_info->ptr = fdata = (FCGI_LOCAL_DATA*) uws_calloc(1, sizeof(FCGI_LOCAL_DATA));

            char *content_len = get_header_param("Content-Length", conn_info->request_header);
            if(strcmp(conn_info->request_header->method, "POST") == 0 && content_len != NULL) {
                fdata->post = (size_t)atol(content_len);//this is post method
            }

            //init some data
            fdata->smem = (memory_t*) uws_calloc(1, sizeof(memory_t));

            char *port = itoa(conn_info->running_server->listen);
            fdata->request_id = conn_info->clientfd;
            sscanf(conn_info->running_server->fastcgi_pass, "%[^:]:%s", fdata->fhost, fdata->fport);

            Param_Value pv[] = {/*{{{*/
                {"QUERY_STRING",conn_info->request_header->request_params},
                {"REQUEST_METHOD", conn_info->request_header->method},
                {"CONTENT_TYPE", nullstring(get_header_param("Content-Type", conn_info->request_header))},
                {"CONTENT_LENGTH", nullstring(get_header_param("Content-Length", conn_info->request_header))},
                {"SCRIPT_FILENAME", conn_info->request_header->path},
                {"SCRIPT_NAME", strrchr(conn_info->request_header->path, '/')},
                {"REQUEST_URI", conn_info->request_header->url},
                {"DOCUMENT_URI", conn_info->request_header->path + strlen(conn_info->running_server->root)},
                {"DOCUMENT_ROOT", conn_info->running_server->root},
                {"SERVER_PROTOCOL", conn_info->request_header->http_ver},
                {"GATEWAY_INTERFACE", "CGI/1.1"},
                {"SERVER_SOFTWARE", UWS_SERVER},
                {"REMOTE_ADDR", get_header_param("Client-IP", conn_info->request_header)},
                {"REMOTE_PORT", get_header_param("Client-Port", conn_info->request_header)},
                {"SERVER_ADDR", conn_info->server_ip},
                {"SERVER_PORT", port},
                {"SERVER_NAME", conn_info->running_server->server_name},
                {"HTTPS", ""},
                {"REDIRECT_STATUS", "200"},
                {NULL,NULL} 
            };/*}}}*/
            //start build request
            begin_build_request(fdata->request_id, fdata->smem);

            Param_Value *tmp = pv;
            while(tmp->name != NULL){
                add_fcgi_param(fdata->request_id, tmp->name, tmp->value, fdata->smem);
                tmp++;
            }

            Http_Param *params = conn_info->request_header->params;
            int count = 0;
            char *new_header;
            for(count = 0; count < conn_info->request_header->used_len; ++count) {
                new_header = header_to_fcgi(params->name);
                add_fcgi_param(fdata->request_id, new_header, params->value, fdata->smem);
                uws_free(new_header);
                params++;
            }
            uws_free(port);

            //add more http headers
            //terminate params
            FCGI_Header end_params;
            end_params = make_header(FCGI_PARAMS, fdata->request_id, 0, 0);
            append_mem_t(fdata->smem, &end_params, FCGI_HEADER_LEN);
        }

        struct sockaddr_in address;
        conn_info->serverfd  = socket(AF_INET, SOCK_STREAM, 0);
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(fdata->fhost);
        address.sin_port = htons(atoi(fdata->fport));

        setnonblocking(conn_info->serverfd);

        int result = connect(conn_info->serverfd, (struct sockaddr*)&address, sizeof(address));

        conn_info->flag = 0x01; //now go to next stage

        if(result < 0 ) {
            if(errno != EINPROGRESS) {
                conn_info->status_code =  502;
                apply_next_router(conn_info);
                return;
            } else {
                add_to_epoll(conn_info, EPOLLOUT);
                longjmp(conn_info->jmp_buff, 1);
                return;
            }
        }
        
    }/*}}}*/

    fdata = (FCGI_LOCAL_DATA*) conn_info->ptr;

    if(conn_info->flag == 0x01) {/*{{{*/

        for( ; ; ) {
            ssize_t res= write(conn_info->serverfd, fdata->smem->mem + fdata->mem_offset, fdata->smem->len - fdata->mem_offset);
            if(res == -1 ) {
                if(errno == EAGAIN) {
                    add_to_epoll(conn_info, EPOLLOUT);
                    longjmp(conn_info->jmp_buff, 1);
                    return;
                } else {
                    conn_info->status_code = 502;
                    apply_next_router(conn_info);
                    return;
                }
            }
            fdata->mem_offset += res;
            if(fdata->mem_offset == fdata->smem->len) break;
        }
        fdata->mem_offset = 0;
        free_mem_t(fdata->smem);
        conn_info->flag = 0x02;
    }/*}}}*/

    //setblocking(conn_info->serverfd);
    //
    if(conn_info->flag == 0x02) {
        if(fdata->post){
            char line[MAX_BODY_LEN];
            FCGI_Header content_header;

            for(; ;) {
                if(fdata->smem->len == 0) {
                    if(fdata->read_offset == fdata->post) break;

                    bzero(line, MAX_BODY_LEN);
                    size_t read_num = fread(line, sizeof(char), MAX_BODY_LEN, conn_info->input_file);
                    fdata->read_offset += read_num;
                    if(read_num == 0) {
                        if(errno == EAGAIN) {
                            struct epoll_event ev;
                            ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                            conn_info->status = CS_UPSTREAM_READ;
                            ev.data.ptr = conn_info;

                            if(epoll_ctl(conn_info->epollfd, EPOLL_CTL_ADD, conn_info->clientfd, &ev) == -1)
                                exit_err("epoll_ctl");

                            longjmp(conn_info->jmp_buff, 1);
                            return;
                        }
                    }
                    content_header = make_header(FCGI_STDIN, fdata->request_id, read_num, 0);
                    append_mem_t(fdata->smem, &content_header, FCGI_HEADER_LEN);
                    append_mem_t(fdata->smem, line, read_num);
                }

                for( ; ; ) {
                    ssize_t res= write(conn_info->serverfd, fdata->smem->mem + fdata->mem_offset, fdata->smem->len - fdata->mem_offset);
                    if(res == -1 ) {
                        if(errno == EAGAIN) {
                            add_to_epoll(conn_info, EPOLLOUT);
                            longjmp(conn_info->jmp_buff, 1);
                            return;
                        } else {
                            conn_info->status_code = 502;
                            apply_next_router(conn_info);
                            return;
                        }
                    }
                    fdata->mem_offset += res;
                    if(fdata->mem_offset == fdata->smem->len) break;
                }
                fdata->mem_offset = 0;
                free_mem_t(fdata->smem);
            }
        }
    }
    
    setblocking(conn_info->serverfd);
    //send finish request symbol
    FCGI_Header end_body;
    end_body = make_header(FCGI_STDIN, fdata->request_id, 0, 0);
    append_mem_t(fdata->smem, &end_body, FCGI_HEADER_LEN);
    FCGI_Header end_header;
    end_header = make_header(FCGI_PARAMS, fdata->request_id, 0, 0);
    append_mem_t(fdata->smem, &end_header, FCGI_HEADER_LEN);

    send_request(conn_info->serverfd, fdata->smem);

    //all_data_send!
    memory_t mem_file;
    bzero(&mem_file, sizeof(memory_t));
    // if we have more content from fastcgi
    bool more_content = read_response(conn_info->serverfd, &mem_file);

    if(mem_file.len == 0) {
        conn_info->status_code =  500;
        apply_next_router(conn_info);
        return;
    }

    char line[LINE_LEN] = {0};
    unsigned char *oldpos = mem_file.mem;
    unsigned char *pos;
    struct http_header fcgi_response_header;
    bzero(&fcgi_response_header, sizeof(fcgi_response_header));
    char key[LINE_LEN];
    char value[LINE_LEN];

    char *time_string = get_time_string(NULL);
    add_header_param("Server", UWS_SERVER, &fcgi_response_header);
    add_header_param("Date", time_string, &fcgi_response_header);
    uws_free(time_string);

    while((pos = strstr(oldpos, "\r\n"))) {
        if(oldpos == pos) break;
        bzero(line, LINE_LEN);
        strncpy(line, oldpos, pos - oldpos);
        sscanf(line, "%[^:]: %s", key, value);
        if(strcmp(key, "Status") == 0) {
            fcgi_response_header.status_code = atoi(value);
        } else {
            push_header_param(key, value, &fcgi_response_header);
        }
        oldpos = pos + strlen("\r\n");
    }
    int content_len = mem_file.len - (pos - mem_file.mem) - strlen("\r\n");

    char *str_len =  itoa(content_len);
    add_header_param("Content-Length", str_len, &fcgi_response_header);
    uws_free(str_len);

    fcgi_response_header.http_ver = "HTTP/1.1";
    if(fcgi_response_header.status_code == 0) fcgi_response_header.status_code = 200;
    fcgi_response_header.status = get_by_code(fcgi_response_header.status_code);

    char *header_str = str_response_header(&fcgi_response_header);
    writen(conn_info->clientfd, header_str, strlen(header_str));
    uws_free(header_str);
    writen(conn_info->clientfd, pos, content_len + strlen("\r\n"));

    while(more_content) {
        free_mem_t(&mem_file);
        more_content = read_response(conn_info->serverfd, &mem_file);
        writen(conn_info->clientfd, mem_file.mem, mem_file.len);
    }

    free_header_params(&fcgi_response_header);
    free_mem_t(fdata->smem);
    free_mem_t(&mem_file);

    uws_free(fdata);    
    conn_info->ptr = NULL;
    apply_next_router(conn_info);
}


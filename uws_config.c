#include "uws.h"
#include "uws_config.h"
#include "uws_mime.h"

static struct nv_pair** uws_configs;

void
read_conf_file() 
{
    FILE* conf_file = fopen(CONFIG_FILE, "rb");
    char buff[LINE_LEN];
    int i = 0;

    while((fgets(buff, LINE_LEN, conf_file)) != NULL) i++;

    uws_configs = (struct nv_pair**) malloc( sizeof(struct nv_pair) * i + 1);

    rewind(conf_file);

    i = 0;
    while((fgets(buff, LINE_LEN, conf_file)) != NULL) {
        uws_configs[i] = (struct nv_pair *) malloc(sizeof(struct nv_pair));;
        uws_configs[i]->name = (char*) malloc(sizeof(char) * OPT_LEN);
        uws_configs[i]->value = (char*) malloc(sizeof(char) * VLU_LEN);
        sscanf(buff, "%[^:]%*[:]%[^:\n]", uws_configs[i]->name, uws_configs[i]->value);//TODO:Not Safe
        i++;
    }
    uws_configs[i] = NULL;
}

char*
get_opt(const char* option) 
{
    int i = 0;
    while(uws_configs[i] != NULL) {
        if(strcmp(uws_configs[i]->name, option) == 0) {
            return  uws_configs[i]->value;
            break;
        }
        i++;
    }
    return NULL;
}

void
init_config() 
{
    read_conf_file();
    read_mime();
}

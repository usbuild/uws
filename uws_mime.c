#include "uws.h"
#include "uws_config.h"
#include "uws_mime.h"
static struct nv_pair** mime_maps;

void
read_mime() 
{
    int i = 0;
    char* mimefile;
    FILE* conf_file;
    char buff[LINE_LEN];
    if((mimefile = uws_config.mimefile) == NULL) exit(1);
    conf_file = fopen(mimefile, "rb");

    while((fgets(buff, LINE_LEN, conf_file)) != NULL) i++;

    mime_maps = (struct nv_pair**) malloc( sizeof(struct nv_pair) * i + 1);

    rewind(conf_file);

    i = 0;
    while((fgets(buff, LINE_LEN, conf_file)) != NULL) {
        mime_maps[i] = (struct nv_pair *) malloc(sizeof(struct nv_pair));;
        mime_maps[i]->name = (char*) malloc(sizeof(char) * OPT_LEN);
        mime_maps[i]->value = (char*) malloc(sizeof(char) * VLU_LEN);
        sscanf(buff, "%[^ ]%*[ ]%[^ \n]", mime_maps[i]->name, mime_maps[i]->value);//TODO:Not Safe
        i++;
    }
    mime_maps[i] = NULL;
    fclose(conf_file);
}

char* mimebyext(const char *ext)
{
    int i = 0;
    char *mime = (char*) malloc(sizeof(char) * MIME_LEN);   
    while(mime_maps[i] != NULL) {
        if(strcmp(mime_maps[i]->value, ext) == 0) {
            strcpy(mime, mime_maps[i]->name);
            return mime;
        }
        i++;
    }
    strcpy(mime, uws_config.http.default_type);
    return mime;
}

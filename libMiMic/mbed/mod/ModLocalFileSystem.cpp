#include "ModLocalFileSystem.h"
#include "HttpdConnection.h"
#include "UrlReader.h"
#include "Http.h"
#include "Httpd.h"
#include "NyLPC_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include "mbed.h"
#include "FATDirHandle.h"


using namespace MiMic;

static void write_path(HttpdConnection& i_connection,const char* i_buf,const char* i_root_alias,const char* i_f_path)
{
    if(i_root_alias==NULL){
        i_connection.sendBodyF("%s",i_buf);
    }else{
        i_connection.sendBodyF("%.*s%s",strlen(i_root_alias)-1,i_root_alias,i_buf+strlen(i_f_path)+1);
    }
}

void ModLocalFileSystem::retDirJson(const char* i_buf,HttpdConnection& i_connection,unsigned char i_fs_type)
{
   //assert(HEAD or GET)
   //directory-list json
    if(!NyLPC_cHttpdUtils_sendJsonHeader((i_connection._ref_inst))){
        return;
    }
    if(!i_connection.isMethodType(Http::MT_GET)){
        return;
    }
      
    DIR* d=opendir(i_buf);
    if ( d == NULL )
    {
        i_connection.sendBodyF("{\"dir\":\"");
        write_path(i_connection,i_buf,this->_root_alias,this->_path);
        i_connection.sendBodyF("\",\"status\":404,\"list\":[]}");
        return;
    }
    if(!i_connection.isMethodType(Http::MT_GET)){
        //nothing to do
    }else{
        i_connection.sendBodyF("{\"dir\":\"");
        write_path(i_connection,i_buf,this->_root_alias,this->_path);
        i_connection.sendBodyF("\",\"status\":200,\"list\":[");
        switch(i_fs_type){
        case ModLocalFileSystem::FST_DEFAULT:
            for(struct dirent *p= readdir(d);;)
            {
                i_connection.sendBodyF("{\"name\":\"%s\",\"mtype\":\"%s\",\"size\":undefined}",
                p->d_name,NyLPC_cMiMeType_getFileName2MimeType(p->d_name));
                p = readdir(d);
                if(p==NULL){
                    break;
                }
                i_connection.sendBodyF(",");                        
            }
            break;
        case ModLocalFileSystem::FST_SDFATFS:
            for(struct dirent *p= readdir(d);;)
            {
                bool isdir=(((struct direntFAT*)(p))->fattrib & AM_DIR)!=0;
                i_connection.sendBodyF("{\"name\":\"%s\",\"mtype\":\"%s\",\"size\":%u}",
                p->d_name,isdir?"directory":NyLPC_cMiMeType_getFileName2MimeType(p->d_name),
                isdir?0:((struct direntFAT*)(p))->fsize);
                p = readdir(d);
                if(p==NULL){
                    break;
                }
                i_connection.sendBodyF(",");                        
            }
            break;
        default:
            break;
        }
        i_connection.sendBodyF("]}");
    }
    closedir(d);
}
void ModLocalFileSystem::retDirHtml(const char* i_buf,HttpdConnection& i_connection,unsigned char i_fs_type)
{
    //assert(HEAD or GET)
    DIR* d=opendir(i_buf);
    if(d==NULL){
        i_connection.sendError(403);
        return;
    }        
    if(!i_connection.sendHeader(200,"text/html",NULL)){
        //nothing to do
    }else{
        if(!i_connection.isMethodType(Http::MT_GET)){
            //nothing to do.
        }else{
            i_connection.sendBodyF(
                "<!DOCTYPE html><html><body><h1>Index of ");
            write_path(i_connection,i_buf,this->_root_alias,this->_path);
            i_connection.sendBodyF("</h1><hr/>\n"
                "<ul>\n");
            switch(i_fs_type){
            case ModLocalFileSystem::FST_DEFAULT:
                for(struct dirent *p = readdir(d);p!=NULL;p = readdir(d))
                {
                    i_connection.sendBodyF("<li><a href=\"./%s\">%s</a></li>\n",
                    p->d_name,p->d_name);
                }
                break;
            case ModLocalFileSystem::FST_SDFATFS:
                for(struct dirent *p = readdir(d);p!=NULL;p = readdir(d))
                {
                    if((((struct direntFAT*)(p))->fattrib & AM_DIR)!=0){
                        //dir
                        i_connection.sendBodyF("<li><a href=\"./%s/\">[DIR]%s</a></li>\n",p->d_name,p->d_name);
                    }else{
                        //file
                        i_connection.sendBodyF("<li><a href=\"./%s\">%s</a></li>\n",p->d_name,p->d_name);
                    }
                }
                break;
            default:
                break;
            }
            i_connection.sendBodyF("</ul></body></html>");
        }
    }
    closedir(d);
}
/**
 * @param i_path
 * i_pathにはnull終端ファイルパスを入れてください。
 */
void ModLocalFileSystem::retFile(char* i_buf,HttpdConnection& i_connection)
{
    //return content
    FILE *fp = fopen(i_buf, "r"); 
    if(fp==NULL){
        i_connection.sendError(404);
        return;
    }
    
    fseek(fp, 0, SEEK_END); // seek to end of file
    size_t sz = ftell(fp);       // get current file pointer
    fseek(fp, 0, SEEK_SET); // seek back to beginning of file
    if(i_connection.sendHeader(200,NyLPC_cMiMeType_getFileName2MimeType(i_buf),NULL,sz)){
        if(!i_connection.isMethodType(Http::MT_GET)){
            //nothing to do
        }else{
            Timer t;
            t.start();
            for(;;){
                sz=fread(i_buf,1,Httpd::SIZE_OF_HTTP_BUF,fp);
                if(sz<1){
                    break;
                }
                if(!i_connection.sendBody(i_buf,sz)){
                    break;
                }
                //switch other session
                if(t.read_ms()>500){
                    //switch transport thread
                    i_connection.unlockHttpd();
                    NyLPC_cThread_sleep(50);
                    i_connection.lockHttpd();
                    t.reset();
                }
            }
        }
    }
    fclose(fp);
}    

NyLPC_TBool flip_url_prefix(char* buf, int buf_len, const char* url_prefix, const char* file_path)
{
    size_t bl = strlen(url_prefix);
    size_t al = strlen(file_path)+2;
    if (al - bl + strlen(buf) + 1 > buf_len){
        return NyLPC_TBool_FALSE;
    }
    if (strncmp(buf, url_prefix, bl) == 0){
        memmove(buf + al, buf + bl, strlen(buf) - bl + 1);
        memcpy(buf + 1, file_path, al - 1);
        *(buf + al - 1) = '/';
    }
    return NyLPC_TBool_TRUE;
}

namespace MiMic
{
    ModLocalFileSystem::ModLocalFileSystem(const char* i_path,const char* i_root_alias,unsigned char i_fs_type):ModBaseClass()
    {
        this->setParam(i_path,i_root_alias,i_fs_type);
    }
    ModLocalFileSystem::ModLocalFileSystem(const char* i_path,unsigned char i_fs_type):ModBaseClass()
    {
        this->setParam(i_path,NULL,i_fs_type);
    }
    ModLocalFileSystem::ModLocalFileSystem():ModBaseClass()
    {
        this->_root_alias=NULL;
    }
    ModLocalFileSystem::~ModLocalFileSystem()
    {
    }
    void ModLocalFileSystem::setParam(const char* i_path,const char* i_root_alias,unsigned char i_fs_type)
    {
        NyLPC_Assert(strlen(i_root_alias)>0);
        ModBaseClass::setParam(i_path);
        this->_root_alias=i_root_alias;
        this->_fs_type=i_fs_type;
    }
    void ModLocalFileSystem::setParam(const char* i_path,unsigned char i_fs_type)
    {
        this->setParam(i_path,NULL,i_fs_type);
    }
    bool ModLocalFileSystem::canHandle(HttpdConnection& i_connection)
    {
        if(this->_root_alias==NULL){
            return ModBaseClass::canHandle(i_connection);
        }
        //root alias指定がある場合
        if(!NyLPC_cHttpdConnection_getReqStatus(i_connection._ref_inst)==NyLPC_cHttpdConnection_ReqStatus_REQPARSE)
        {
            return NyLPC_TBool_FALSE;
        }        
        const NyLPC_TChar* in_url;
        in_url=NyLPC_cHttpdConnection_getUrlPrefix(i_connection._ref_inst);
        size_t base_url_len=strlen(this->_root_alias);
        if(strncmp(this->_root_alias,in_url,base_url_len)!=0){
            return false;
        }
        return true;
    }  
    bool ModLocalFileSystem::execute(HttpdConnection& i_connection)
    {
        //check platform
        //<write here! />
        
        //check prefix
        if(!this->canHandle(i_connection)){
            return false;
        }
        
        //check Method type
        {
            int mt=i_connection.getMethodType();
            if(mt!=Http::MT_GET && mt!=Http::MT_HEAD){
                //method not allowed.
                i_connection.sendError(405);
                return true;
            }
        }
        //Httpd lock
        i_connection.lockHttpd();
        char* buf=Httpd::_shared_buf;
        
        //set file path
        {
            //call ModUrl
            NyLPC_TcModUrl_t mod;
            NyLPC_cModUrl_initialize(&mod);
            if(!NyLPC_cModUrl_execute2(&mod,i_connection._ref_inst,buf,Httpd::SIZE_OF_HTTP_BUF,0,NyLPC_cModUrl_ParseMode_ALL)){
                NyLPC_cModUrl_finalize(&mod);
                i_connection.unlockHttpd();
                return true;
            }
            NyLPC_cModUrl_finalize(&mod);
        }
        //パスのエイリアスがあるときはここで編集
        if(this->_root_alias!=NULL){
            flip_url_prefix(buf,Httpd::SIZE_OF_HTTP_BUF,this->_root_alias,this->_path);
        }
        UrlReader url(buf);
        if(url.hasQueryKey("list")){
            // if path has '/?list' query key,return directory information
            const char* t;
            int l;
            url.getPath(t,l);
            buf[l]='\0';//split path
            //remove '/'
            if(buf[l-1]=='/'){
                buf[l-1]='\0';
            }
            retDirJson(buf,i_connection,this->_fs_type);
        }else if(strchr(buf,'?')==NULL && strchr(buf,'#')==NULL && buf[strlen(buf)-1]=='/'){
            //return directory html when URL has not bookmark and URL query and terminated by '/'.
            buf[strlen(buf)-1]='\0';//convert to dir path
            retDirHtml(buf,i_connection,this->_fs_type);
        }else{
            //split URL path and query
            const char* t;
            int l;
            url.getPath(t,l);
            buf[l]='\0';
            retFile(buf,i_connection);
        }
        //Httpd unlock
        i_connection.unlockHttpd();
        return true;
        
    }
}

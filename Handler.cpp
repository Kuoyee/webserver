#include"Handler.h"

Handler::Handler(){ 
}


int Handler::getclientfd(){
    return client_fd;
}

Handler::~Handler(){
        
}

void Handler::setHandler(int client_fd,int tree_fd,threadpool<Handler> *pool){
    this->client_fd=client_fd; 
    this->tree_fd=tree_fd;  
    this->pool=pool;
}
void Handler::dealwithread(){
    pool->add_task(this,0);
}
void Handler::dealwithwrite(){
    pool->add_task(this,1);
}
void Handler::handle_read(){
    get_line(client_fd,buf,sizeof(buf));
    process();
}

void Handler::process(){
    char method[64];
    char URL[256];
    char Protocol[64];
    int len;
    std::smatch results;
    std::string first_line=buf;
    std::string s1,s2,s3;
    if(regex_match(first_line,results,std::regex("(.*)\\s{1}(.*)\\s{1}(.*)"))){
        s1=results[1];
        s2=results[2];
        s3=results[3];
    }
    strcpy(method,s1.c_str());
    strcpy(URL,s2.c_str());
    strcpy(Protocol,s3.c_str());

    do{
        len=get_line(client_fd,buf,sizeof(buf)); ///返回值类型 设为读取一行的长度，方便判断
    }while(len!=0);


        //处理GET请求
    char param[1024];
    if(strcmp(method,"GET")==0){
        //定位本地html文件
        char *pos=strchr(URL,'?');    //返回？的位置
        if(pos){
            *pos='\0';
            strcpy(param,pos+1);       ///赋值
        }
        //执行HTTP响应
        //判断文件是否存在，存在 则状态码为200 ok 否则为404 
        struct stat st;
        char path[256]="./html_docs";
        strcat(path,URL);
        if(stat(path,&st)==0){//存在，成功
            //假设url对应的是文件夹，自动补上index.html
            if(S_ISDIR(st.st_mode)){
                strcat(path,"/index.html");
            }
            response_prepare(client_fd,path);

        }else{   //不存在或出错
            std::cout<<" not found"<<std::endl;
            not_found(client_fd);
        }
    }
    epoll_event event;
    event.data.fd = client_fd;
    event.events=EPOLLOUT|EPOLLET;
    epoll_ctl(tree_fd,EPOLL_CTL_MOD,client_fd,&event);
}

void Handler::dowrite(const char *buf,size_t data_size){
    int nwrite;
    while (rest_have_tosend > 0){
        nwrite = write(client_fd, buf + data_size - rest_have_tosend, rest_have_tosend);
        std::cout<<client_fd<<" "<<rest_have_tosend<<std::endl;
        if (nwrite<rest_have_tosend)
        {
            if (nwrite == -1 && errno != EAGAIN)
            {
                perror("write error");
                break;
            }
        }
        rest_have_tosend -= nwrite;
    }
}

void Handler::handle_write(){
    // //响应头部
    int len=write(client_fd,response_header,strlen(response_header));
    if(File_type=="png"||File_type=="jpg"){
        //write(client_fd,(const char*)ImgBuffer,rest_have_tosend);
        dowrite((const char*)ImgBuffer,rest_have_tosend);
    }else{
        //write(client_fd,response_text,rest_have_tosend);
        dowrite(response_text,rest_have_tosend);
    }
    

    epoll_ctl(tree_fd,EPOLL_CTL_DEL,client_fd,NULL); 
    close(client_fd);
}

//读取一行
int Handler::get_line(int clnt_sock,char *buf,int size){
    int count=0;
    char ch='\0';
    int len=0;
    //std::cout<<"线程: "<<pthread_self()<<"  读取cfd   "<<clnt_sock<<endl;
    while((count<size-1)&&ch!='\n'){
        len=read(clnt_sock,&ch,1);
        if(len==1){
        if(ch=='\r'){       //回车符
            continue;
        }else if(ch=='\n'){//换行符
            buf[count]='\0';
        }else{             //正常字符
            //std::cout<<ch;
            buf[count++]=ch;
        }
        }else if(len==-1){
            perror("fail to read!!!");
            break;
        }else if(len==0){
            perror("client is closed");
            break;
        }
    }
    //std::cout<<std::endl;
    return count;
}


void Handler::response_prepare(int clnt_sock,const char *path){  //若找到html文件
    strcpy(response_text,"");
    std::ifstream f(path,std::ios::in | std::ios::binary);
    if(!f){       //自动补全后未找到
        not_found(clnt_sock);
        return ;
    }

    strcpy(response_header,"HTTP/1.1 200 OK\r\n");
    strcat(response_header,"Server: kuoy's httpserver\r\n");
    strcat(response_header,"Connection: close\r\n");

    std::smatch results;
    std::string file_path=path;

    if(regex_match(file_path,results,std::regex(".*\\.(.*)"))){
        File_type=results[1];
    }

    //C++ 中 switch case 语句不识别字符串。
    if(File_type=="html"){
        strcat(response_header,"Content-Type: text/html\r\n");
    }
    if(File_type=="css"){
        strcat(response_header,"Content-Type: text/css\r\n");
    }
    if(File_type=="js"){
        strcat(response_header,"Content-Type: text/js\r\n");
    }
    if(File_type=="png"){
        strcat(response_header,"Content-Type: image/png\r\n");
    }
    if(File_type=="jpg"){
        strcat(response_header,"Content-Type: image/jpg\r\n");
    }

    struct stat st;
    if(stat(path,&st)==-1){
        std::cout<<"Internal error 501"<<std::endl;
    }else{
        char content_len[64];
        int flen=st.st_size;
        rest_have_tosend=flen;
        sprintf(content_len, "%d", flen);
        strcat(response_header,"Content-Lenth: ");
        strcat(response_header,content_len);
        strcat(response_header,"\r\n");
    }
    strcat(response_header,"\r\n");
    
    //响应
    if(File_type=="png"||File_type=="jpg"){
            f.seekg(0, std::ios::end);   // 将文件指针移动到文件末尾
            imglength = f.tellg();   // 返回文件指针的位置
            rest_have_tosend=imglength;
            f.seekg(0, std::ios::beg);
            //根据图像数据长度分配内存buffer
            f.read((char *)ImgBuffer, imglength * sizeof(char));

    }else{
        std::string line;
        std::cout<<"text"<<std::endl;
        while (getline (f, line)){
            const char *p;
            int len;
            p = line.c_str();
            strcat(response_text,p);

        }

    }
    f.close();
}



void Handler::not_found(int clnt_sock){
    strcpy(response_header,"");
    strcpy(response_text,"");
    const char *reply_header="HTTP/1.1 404 NOT FOUND\r\n\
Content-Type:text/html\r\n\
\r\n";
    strcat(response_header,reply_header);
    const char *reply_body="<!DOCTYPE html>\r\n\
    <html lang=\"en\">\r\n\
    <head>\r\n\
        <meta charset=\"UTF-8\">\r\n\
        <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\
        <title>Document</title>\r\n\
    </head>\r\n\
    <body>\r\n\
        <p>not found</p>\r\n\
    </body>\r\n\
    </html>";
    strcat(response_text,reply_body);
}
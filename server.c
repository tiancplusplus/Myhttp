#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<strings.h>
#include<pthread.h>
#include<signal.h>
//#include<pthread>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
//sockaddr_in
#include<arpa/inet.h>
//inet_pton
#define buf_size 1024

char in_path[100];

void print_err(const char* error){
	printf("Error---%s---\n",error);
}

int read_head_line(int connect_fd,char* buf,int size){	
	int i = 0;
	char ch = '\0';
	int n = 0;
	for(i = 0; i < size - 1; i++){
		n = recv(connect_fd,&ch,1,0);
		if(n > 0){
			buf[i] = ch;	
			if(ch == '\n'){
				i++;
				break;
			}
		}else{
			return -1;	
		}
	
	}
	buf[i] = '\0';
	return i;
}

void response_err(){
	return ;
}
void bad_request(int connect_fd)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(connect_fd, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(connect_fd, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(connect_fd, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(connect_fd, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(connect_fd, buf, sizeof(buf), 0);
}

void get_url(char*buf,char*url,int*con,int size){
	(*con)++;
	int i = 0;
	while(buf[*con] != ' ' && buf[*con] != '?' && i < size - 1)
	{
		url[i] = buf[*con];
		(*con)++;
		i++;
	}
	url[i] = '\0';
}

void response_head(int connect_fd){
	char buf[1024];
	strcpy(buf,"HTTP/1.0 200 OK\r\n");
	send(connect_fd,buf,strlen(buf),0);
	strcpy(buf,"Tian`s server\r\n");
	send(connect_fd,buf,strlen(buf),0);
	strcpy(buf,"Content-Type: text/html\r\n");
	send(connect_fd,buf,strlen(buf),0);
	strcpy(buf,"\r\n");
	send(connect_fd,buf,strlen(buf),0);
} 
void response_not_found(int connect_fd){
	//printf("no file \n");
	char buf[1024];
	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(connect_fd, buf, strlen(buf), 0);
	sprintf(buf, "Tian`s server\r\n");
	send(connect_fd, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(connect_fd, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(connect_fd, buf, strlen(buf), 0);
	sprintf(buf, "Not found resources");
	send(connect_fd, buf, strlen(buf), 0);
	return ;
}
void send_file(int connect_fd,char* path){
	FILE* file = NULL;
	int n = 1;
	char buf[buf_size];
	file = fopen(path,"r"); //common file not binary file
	if(file == NULL){
		response_not_found(connect_fd);	
	}
	else{
		response_head(connect_fd);
		while(1){
			fgets(buf,buf_size,file);
			send(connect_fd,buf,strlen(buf),0);
			if(feof(file)){
				break;	
			}
		}
		fclose(file);
	}
}
void response_msg(int connect_fd,char* url){
	char path[200] ;
	strcpy(path,in_path);

	if(!strcmp(url,"/"))
		strcat(path,"/index.html");
	else
		strcat(path,url);	
	//printf("path = %s\n",path);
	send_file(connect_fd,path);

	return ;
}

void process_request(void* sockfd){
	int connect_fd = (intptr_t)sockfd;
	//printf("test connection\n");
	char *buf = (char*)malloc(sizeof(char)*buf_size);
	char method[20];
	char url[100];
	char argu[100];
	char path[100];
	int i = 0;
	int n = 0;
	int con = 0;
	int post_data_lenth = -1;
	//char note[100] = "your input parameter is : ";
	if(!buf){
		print_err("malloc error");	
		close(connect_fd);
		return ;
	}
	n = read_head_line(connect_fd,buf,buf_size);
	if( n != -1){
		//printf("---%s---%d",buf,n);
		//read a line, parse 
		while( buf[i] != ' ' && i < sizeof(method) - 1){
			method[i] = buf[i];
			i++;	
		}
		con = i;
		method[i] = '\0';
		//printf("\nmethod = %s\n",method);
	}
	else{
		//sockek have something data need handle.
		free(buf);
		//print_err("unknow error");
		close(connect_fd);
		return ; //find a error
	}

	if(!strcasecmp(method,"GET")){
		get_url(buf,url,&con,100);
		printf("url = %s \n",url);
		if(buf[con] == ' '){
			while(1){
				read_head_line(connect_fd,buf,buf_size);
				if(buf[0] == '\r'){
					break;	
				}	
			}
			response_msg(connect_fd,url);	
		}
		else if(buf[con] == '?')
		{ 	//get method url have argument	
			i = 0;
			con++;
			while(buf[con] != ' ' && i < 100 - 1){
				argu[i] = buf[con];
				con++;
				i++;	
			}
			argu[i] = '\0';
			//printf("argu = %s\n",argu);
			while(1){ //destory head 
				read_head_line(connect_fd,buf,buf_size);
				if(buf[0] == '\r'){
					break;	
				}	
			}
			response_head(connect_fd);
			sprintf(buf,"this is your argu is : ");
			send(connect_fd,buf,strlen(buf),0);
			send(connect_fd,argu,strlen(argu),0);//return argu to client then close .
			sprintf(buf," , that can`t be handle ");
			send(connect_fd,buf,strlen(buf),0);
		}
	
	}
	else if(!strcasecmp(method,"head")){
		get_url(buf,url,&con,100);
		while(1){ //destory head 
			read_head_line(connect_fd,buf,buf_size);
			if(buf[0] == '\r'){
				break;	
			}	
		}
		strcpy(path,in_path);
		strcat(path,url);
		
		//printf("head method \n");
		//return ;
	
	}
	else if(!strcasecmp(method,"post")){
		get_url(buf,url,&con,100);
		while(1){
			read_head_line(connect_fd,buf,buf_size);
			if(buf[0] == '\r'){
				break;	
			}
			buf[15]='\0';
			if(!strcasecmp(buf,"Content-Length:")){
				post_data_lenth = atoi(&(buf[16]));	
				//printf("post data lenth = %d\n",post_data_lenth);
			}
		}
		read(connect_fd,argu,post_data_lenth);
		argu[post_data_lenth] = '\0';
		//printf("post data = %s\n",argu);
		response_head(connect_fd);
		sprintf(buf,"post argu is : ");
		send(connect_fd,buf,strlen(buf),0);
		send(connect_fd,argu,strlen(argu),0);
	}
	else {
		print_err("http request parse is error\n");	
		response_err();
	}

	free(buf);
	close(connect_fd);
	return ;
}

void sig_fun(int signo){
	printf("--Process is end by the signal SIGINT\n--");
	exit(0);
}


int main(int argc,char*argv[]){
	int listen_fd;
	int connect_fd;
	struct sockaddr_in srv_sock, cli_sock;
	socklen_t cli_len;
	pthread_t thread_id;
	char cli_ip_addr[20];

	if(argc != 2){
		print_err("argc is error,please input 2 argc");
		return -1;	
	}
	signal(SIGINT,sig_fun);

	strcpy(in_path,argv[1]);
	memset(&srv_sock,0,sizeof(srv_sock));
	srv_sock.sin_family = AF_INET;
	srv_sock.sin_port = htons(80);
	srv_sock.sin_addr.s_addr = htonl(INADDR_ANY);

	if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		print_err("socket init bad");
		return -1;
	}

	if(bind(listen_fd,(struct sockaddr *)&srv_sock,sizeof(srv_sock)) < 0){
		print_err("bind bad");
		close(listen_fd);
		return -1;
	}

	if(listen(listen_fd,5) < 0){
		print_err("listen error");
		close(listen_fd);
		return -1;	
	}

	for(;;){
		memset(&cli_sock,0,sizeof(cli_sock));
		connect_fd = accept(listen_fd,(struct sockaddr *)&cli_sock,&cli_len);
		if(connect_fd < 0){
			print_err("accept id bad");	
		}
		//inet_ntop(cli_sock.sin_family,&cli_sock.sin_addr,cli_ip_addr,40);
		printf("client ip address : %s \n",inet_ntoa(cli_sock.sin_addr));
		//handle cli_sock output information
		//process_request((void*)(intptr_t)connect_fd);
		//close(connect_fd); //server complish and close socket.
		if(pthread_create(&thread_id,NULL,(void*)process_request,(void*)(intptr_t)connect_fd)!= 0){
			print_err("pthread_create is error");	
		}	
	}
	return 0;
}

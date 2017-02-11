#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>
#include<sys/select.h>
#include<sys/time.h>
#include<signal.h>

#define PORT "8888"

struct info{
	int client_no;
	char name[30];
};

struct getlist{
	int size;
	int cno[20];
	char name1[20][30];
};	

struct message{
	int fun;
	char data[256];
	struct info src;
	struct info dest;
	struct getlist list;
};

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family==AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int main(int argc,char *argv[]){


/* *********************** Normal work to prepare socket for server **************************************************/
	struct addrinfo hints, *res,*p;
	socklen_t client_addrlen;
	int status,yes=1,sockfd;
	char name[30];
	if(argc!=2){
		fprintf(stderr,"Give ip address\n");
		exit(0);
	}
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;	
	hints.ai_flags=AI_PASSIVE;

	if((status=getaddrinfo(argv[1],PORT,&hints,&res))==-1){
		fprintf(stderr,"getaddrinfo error %s\n",gai_strerror(status));
		return 1;
	}
	
	for(p=res;p!=NULL;p=p->ai_next){
		if((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
			perror("client:socket");
			continue;
		}
		if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
			perror("client:setsockopt");
			exit(0);
		}
		if(connect(sockfd,p->ai_addr,p->ai_addrlen)==-1){
			perror("client:bind");
			continue;
		}
		break;
	}

	if(p==NULL){
		fprintf(stderr,"client can't connect to server\n");
		exit(0);
	}
	
/********************************************************************************************************/
	struct message *msg;

	struct getlist *list;
	char name2[30],buf[256],buf1[30];
	int client_no,choice;
	msg=(struct message*)malloc(sizeof(struct message));
	printf("Enter your name.\n");
	read(0,name2,sizeof(name2));
	int zzz=strlen(name2);
	name2[zzz]='\0';
	strcpy(msg->src.name,name2);
	strcpy(msg->dest.name,"server");
	strcpy(msg->data,"Hello world");
	msg->fun=0;
	read(sockfd,&client_no,sizeof(client_no));
	msg->src.client_no=client_no;
	msg->dest.client_no=0;
	printf("server:your client no is %d\n",client_no);
	write(sockfd,(char*)msg,sizeof(struct message));
	free(msg);
	while(1){
		printf("\n***************************************************\n");
		printf("1:online clients\n2:send data\n4:quit\n");
		printf("***************************************************\n");
		read(0,buf1,sizeof(buf1));
		choice=atoi(buf1);
		if(choice==1){//online clients
			msg=(struct message*)malloc(sizeof(struct message));
			msg->fun=3;//get list
			msg->src.client_no=client_no;
			msg->dest.client_no=0;
			write(sockfd,(char*)msg,sizeof(struct message));
			read(sockfd,(char*)msg,sizeof(struct message));
			int i=0,s1,size=msg->list.size;
			if(size==0){
				printf("No other client online\n");
			}
			else{
				printf("Online clients......\n");
				while(i<size){
					s1=strlen(msg->list.name1[i]);
					msg->list.name1[i][s1-1]='\0';
					printf("%s:(%d)\n",msg->list.name1[i],msg->list.cno[i]);
					i++;
				}
			}
			free(msg);sleep(1);
		}else if(choice==2){//private message
			printf("Enter exit if you want to quit sending data\n");
			printf("Whom you want to send, enter Client No....\n");
			int cc;
			scanf("%d",&cc);
			printf("client no is %d\n",cc);
			printf("start sending data...\n");
			
			while(1){
				struct timeval tv;
				tv.tv_sec=10;
				tv.tv_usec=0;
				fd_set fds;
				FD_ZERO(&fds);
				FD_SET(sockfd,&fds);
				printf("Enter data\n");
				memset(buf,0,sizeof(buf));
				msg=(struct message*)malloc(sizeof(struct message));
				read(0,buf,sizeof(buf));
				int sd=strlen(buf);
				buf[sd-1]='\0';
				
				msg->fun=1;
				msg->dest.client_no=cc;
				msg->src.client_no=client_no;
				strcpy(msg->src.name,name2);
				if(strcmp(buf,"exit")==0){
					strcpy(msg->data,"connection ended sender");
					printf("connection ended with receiver\n");
					write(sockfd,(char*)msg,sizeof(struct message));						
					break;
				}
				strcpy(msg->data,buf);
				write(sockfd,(char*)msg,sizeof(struct message));
				
				if(select(sockfd+1,&fds,NULL,NULL,&tv)==0){
					printf("reciever did not response. send same data again.......\n");
				}else{
					
					read(sockfd,(char*)msg,sizeof(struct message));
					printf("receiver: %s\n",msg->data);
				}	
				FD_CLR(sockfd,&fds);		
				free(msg);
			}
		}else if(choice==4){
			msg=(struct message*)malloc(sizeof(struct message));
			strcpy(msg->data,"bye");
			write(sockfd,(char*)msg,sizeof(struct message));
			close(sockfd);
			exit(0);
		}else{
			printf("Please enter right choice\n");
		}
	}
	close(sockfd);
	
return 0;
}

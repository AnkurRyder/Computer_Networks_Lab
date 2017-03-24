#include<stdio.h>
#include<stdlib.h>
#include<math.h>
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

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family==AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct info{
	int client_no;
	char mac[4];
};

struct message{
	int fun;
	char data[256];
	struct info src;
	struct info dest;
};


int main(int argc,char *argv[]){


/* *********************** Normal work to prepare socket for server **************************************************/
	struct addrinfo hints,*res,*p;
	struct sockaddr_storage client_addr;
	socklen_t client_addrlen;
	int status,yes=1,sockfd,clientfd,n=1;
	char ip[INET6_ADDRSTRLEN];
	

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
			perror("server:socket");
			continue;
		}
		if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
			perror("server:setsockopt");
			exit(0);
		}
		if(connect(sockfd,p->ai_addr,p->ai_addrlen)==-1){
			perror("server:bind");
			continue;
		}
		break;
	}

	if(p==NULL){
		fprintf(stderr,"server can't bind\n");
		exit(0);
	}
	freeaddrinfo(res);
/********************************************************************************************************/
	struct message *msg,*msg1;
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
	char my_mac[4],d_mac[4];
	printf("Enter your MAC addrs\n");
	scanf("%s",my_mac);
	//printf("Enter exit if you want to quit sending data\n");
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
			//int count=1;
			while(1){
				msg=(struct message*)malloc(sizeof(struct message));
				printf("Enter destination MAC addrs\n");
				scanf("%s",d_mac);
				msg->fun=5;
				strcpy(msg->src.mac,my_mac);
				strcpy(msg->dest.mac,d_mac);
				while(1){
					printf("Enter the data\n");
					memset(buf,0,sizeof(buf));
					read(0,buf,sizeof(buf));
					int sd=strlen(buf);
					buf[sd-1]='\0';
					if(strcmp(buf,"exit")==0){
						break;
				}
				strcpy(msg->data,buf);
				write(sockfd,(char*)msg,sizeof(struct message));
				//read(sockfd,(char*)msg1,sizeof(struct message));
				
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

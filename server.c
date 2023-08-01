#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_HOSTNAME "login.mcs.suffolk.edu"
#define SERVER_PORT "2000"
#define ARGSIZE 100
#define TRUE 1

char checkandgetuid(char* buf,char* uid);
char checkbye(char* buf,int cookie);
void gethostandport(char *s,char *hostname,char* port);


int main(int argc, char *argv[]){
  int sock,length1;
  struct sockaddr_in server,client;
  struct hostent *hp,*gethostbyaddr();
  int msgsock;
  char address[40];
  unsigned long address1;
  char buf[1024],hostname[ARGSIZE],port[ARGSIZE],msg[ARGSIZE*2],uid[ARGSIZE];
  int rval,rval1,i,cookie;
  char check;
  int logfile;
  time_t t;

  if ((argc!=1)&&(argc!=3)){
    fprintf(stderr,"wrong numbers of arguments\n");
    exit(1);
  }

  for(i=1;i<argc;i++){
    if (strlen(argv[i])>ARGSIZE-1){
      fprintf(stderr,"one of the argument is too long!\n");
      exit(1);
    }
  }

  if (argc==3){
    if (!strcmp(argv[1],"-h"))
      gethostandport(argv[2],hostname,port);
    else{
      fprintf(stderr,"second argument should be the host name\n");
      exit(1);
    }
  }else{
    strcpy(hostname,SERVER_HOSTNAME);
    strcpy(port,SERVER_PORT);
  }

  if (port[0]=='\0')  strcpy(port,SERVER_PORT);
    
  logfile= open("serverlog", O_TRUNC | O_CREAT | O_WRONLY ,0777);
  if (logfile==-1){         /* here I quit if the log file could not be
				 opened . */
    fprintf(stderr,"could not open the log file\n");
    exit(1);
  }
      
  sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock<0){
    perror("openning stream socket");
    exit(1);
  }

  server.sin_addr.s_addr =INADDR_ANY;
  server.sin_family = AF_INET;
  server.sin_port=ntohs(atoi(port));
  if (bind(sock,(struct sockaddr *) &server,sizeof(server))){
    perror("binding stream socket");
    exit(1);
    }


  printf("machine:%s  port:#%d\n",hostname,ntohs(server.sin_port));

  srand(time(NULL));
  listen(sock,5);
  do{
    length1=sizeof(client);
    msgsock = accept(sock,(struct sockaddr *)&client,(unsigned int *) &length1);
    if (msgsock == -1)
      perror("accept");
    else {
      bzero(address,sizeof(address));
      sprintf(address,"%s",inet_ntoa(client.sin_addr));
      address1=inet_addr(address);
      hp = gethostbyaddr((char*)&address1,sizeof(address1),AF_INET);
      if (hp==0){
	fprintf(stderr,"unknown host\n");
      }else{
	printf("--------------\n");
	printf("address: %s \n",address);
	printf("Name: %s \n",hp->h_name);
	printf("Cookie: %d\n",cookie=rand());
	printf("--------------\n");
      }
      do{
	bzero(buf,sizeof(buf));
	if ((rval =read(msgsock ,buf ,1024))<0)
	  perror("reading stream message");
	else if (rval>0){
	  t=time(NULL);
	  write(logfile,"--------\n",9);
	  write(logfile,ctime(&t),strlen(ctime(&t)));
	  write(logfile,"\n",1);
	  write(logfile,buf,strlen(buf));
	  write(logfile,"\n",1);
	  bzero(uid,sizeof(uid));
	  check=checkandgetuid(buf,uid);	
	  if (!check) {    /*not well formatted message */
	    write(logfile,"NOT accepted\n",13);
	    fprintf(stderr,"Ending connection.illegal message \n");
	    break;
	  }
	  write(logfile,"accepted\n",9);
	  bzero(msg,sizeof(msg));
	  sprintf(msg,"%s_%s fall-2002-671 %d status\n",hostname,uid,cookie);
	  if ((rval1 =write(msgsock ,msg ,sizeof(msg)))<0)
	    perror("writing stream message");
	  bzero(buf,sizeof(buf));
	  if ((rval =read(msgsock ,buf ,1024))<0)
	    perror("reading stream message");
	  t=time(NULL);
          write(logfile,"--------\n",9);
          write(logfile,ctime(&t),strlen(ctime(&t)));
          write(logfile,"\n",1);
          write(logfile,buf,strlen(buf));
          write(logfile,"\n",1);
	  if (!checkbye(buf,cookie)){
	     write(logfile,"NOT accepted\n",13);
             fprintf(stderr,"Ending connection.illegal message \n");
             break;
	  }
	  write(logfile,"accepted\n",9);
	  bzero(msg,sizeof(msg));
          sprintf(msg,"%s_%s %d bye\n",hostname,uid,cookie);
	  if ((rval1 =write(msgsock ,msg ,sizeof(msg)))<0)
            perror("writing stream message");
	}
	i=0;
	if (rval ==0)
	  printf("Ending connection \n");	
      }while(rval>0);
      close(msgsock);
    }
  }while(TRUE); 
}


char checkandgetuid(char *buf,char  *uid){
  int counter1,counter;
  counter=0;
  counter1=0;
  if (strncmp(buf,"fall-2002-671",13)){
    return 0;
  }

  if (buf[13]!=' ' || !isalnum(buf[14])){
    return 0;
  }

  counter=14;
  
  while(isalnum(buf[counter])){   /* uid */
    uid[counter1]=buf[counter];
    counter1++;counter++;
  }
  uid[counter1]='\0';
    
  if (buf[counter++]!=' ')
    return 0;
    
  if (!isalpha(buf[counter]))   /*first name */
    return 0;
    
  while(isalpha(buf[counter])){
      counter++;
  }
    
  if (buf[counter++]!=' ')
    return 0;
      
  if (!isalpha(buf[counter]))   /*last name */
    return 0;
    
  while(isalpha(buf[counter])){
      counter++;
  }
    
  if (buf[counter++]!=' ')
    return 0;
      
  if (strcmp(&buf[counter],"hello\n"))   /*hello */
    return 0;

  if (buf[counter+6]!='\0')
    return 0;
  return 1;

}
 
      
      
char checkbye(char* buf,int cookie){
  int counter1,counter;
  char cooke[50];
  counter=0;
  counter1=0;
  if (strncmp(buf,"fall-2002-671",13)){
    return 0;
  }

  if (buf[13]!=' ' || !isalnum(buf[14])){
    return 0;
  }

  counter=14;
  while(isdigit(buf[counter])){
    cooke[counter1]=buf[counter];
    counter++;counter1++;
    if (counter1>20) return 0;
  }

  if (buf[counter]!=' ') return 0;
  if (cookie!= atoi(cooke)) return 0;

  counter++;

  if (strncmp(&buf[counter],"bye\n",4))
    return 0;
  if (buf[counter+4]!='\0') return 0;
  return 1;
   
}

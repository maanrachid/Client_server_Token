#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <string.h>

#define SERVER_HOSTNAME "f431-02"
#define SERVER_PORT "2000"
#define ARGSIZE 100

void gethostandport(char *s,char *hostname,char* port);
char checkandextractcooke(char *msg,char *cookie,char *hostname,char *uid);

int main(int argc,char* argv[]){
  int sock,rval;
  struct sockaddr_in server;
  struct hostent *hp,*gethostbyname();
  char buf[1024],cooke[20],check;
  short op,i;
  char hostname[ARGSIZE],port[ARGSIZE],uid[ARGSIZE],first[ARGSIZE],
    last[ARGSIZE],msg[ARGSIZE*5];
  int logfile;
  time_t t;



  if ((argc!=9) && (argc!=7)){
    fprintf(stderr,"wrong number of arguments\n");
    exit(1);
  }

  for(i=1;i<argc;i++){
    if (strlen(argv[i])>ARGSIZE-1){
      fprintf(stderr,"one of the argument is too long!\n");
      exit(1);
    }
  }

  if (argc==7) {
    op=0;
    strcpy(hostname,SERVER_HOSTNAME);
    strcpy(port,SERVER_PORT);
  }else op=2;


  if (argc==9){
    if (!strcmp(argv[1],"-h"))
      gethostandport(argv[2],hostname,port);
    else{
      fprintf(stderr,"use the host name option first\n");
      exit(1);
    }
  }

  if (strcmp(argv[1+op],"-u") || strcmp(argv[3+op],"-f") ||
      strcmp(argv[5+op],"-l")){
    fprintf(stderr,"some information are missing or not in order\n");
    exit(1);
  }else{
    strcpy(uid,argv[2+op]);
    strcpy(first,argv[4+op]);
    strcpy(last,argv[6+op]);
  }
  

  logfile= open("clientlog", O_CREAT | O_TRUNC | O_WRONLY ,0777);
  if (logfile==-1){         /* here I quit if the log file could not be
			       opened . */
    fprintf(stderr,"could not open the log file\n");
    exit(1);
  }


  if (port[0]=='\0') strcpy(port,SERVER_PORT);
  sprintf(msg,"fall-2002-671 %s %s %s hello\n",uid,first,last);

  sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock<0){
    perror("openning stream socket");
    exit(1);
  }

  server.sin_family = AF_INET;
  hp = gethostbyname(hostname);
  if (hp==0){
    fprintf(stderr,"%s : unknown host\n",hostname);
    exit(2);
  }

  bcopy(hp->h_addr,&server.sin_addr,hp->h_length);
  server.sin_port = htons(atoi(port));
   
  if (connect(sock,(struct sockaddr * ) &server,sizeof(server))<0){
    perror("connecting stream socket");
    exit(1);
  }


  bzero(buf,sizeof(buf));
  if (write(sock,msg ,sizeof(msg))<0){
    perror("writing in stream socket");
    exit(1);
  }
  t=time(NULL);
  write(logfile,"--------\n",9);
  write(logfile,ctime(&t),strlen(ctime(&t)));
  write(logfile,"\n",1);
  write(logfile,msg,strlen(msg));
  write(logfile,"\n",1);

  if  ((rval =read(sock ,buf ,1024))<0){
    perror("reading stream message");
    exit(1);
  }

  if (rval>0){
    printf("Message from the Server: %s\n",buf);
    write(logfile,"accepted\n",9);
    if (!(check=checkandextractcooke(buf,cooke,hostname,uid))){
      fprintf(stderr,"problem! bad message from the server\n");
      close(sock);
      exit(1);
    }
    bzero(msg,sizeof(msg));
    sprintf(msg,"fall-2002-671 %s bye\n",cooke);
    if (write(sock,msg ,sizeof(msg))<0){
      perror("writing in stream socket");
      exit(1);
    }
    t=time(NULL);
    write(logfile,"--------\n",9);
    write(logfile,ctime(&t),strlen(ctime(&t)));
    write(logfile,"\n",1);
    write(logfile,msg,strlen(msg));
    write(logfile,"\n",1);
    bzero(buf,sizeof(buf));
    if  ((rval =read(sock ,buf ,1024))<0){
      perror("reading stream message");
      exit(1);
    }
    else if (rval>0){
      printf("Message 2 from the Server :%s\n",buf);
      write(logfile,"accepted\n",9);
    }else {
      printf("rejected!\n");
      write(logfile,"NOT accepted\n",13);
    }
  }else{ 
    printf("rejected!\n");
    write(logfile,"NOT accepted\n",13);
  }
  
  close(sock);
  close(logfile);
  return 0;
  }


char checkandextractcooke(char *msg,char *cookie,char *hostname,char *uid){
  int counter,counter1;
  counter=counter1=0;
  /*  if (strncmp(msg,hostname,counter=strlen(hostname)))
    return 0;

  if (msg[counter++]!='_')
    return 0;

  if (strncmp(&msg[counter],uid,strlen(uid)))
  return 0; */

  counter+=strlen(uid)+strlen(hostname) + 1;
  if (msg[counter++]!=' ') {printf("%s\n",hostname);return 0;}

  if (strncmp(&msg[counter],"fall-2002-671 ",14)) return 0;

  counter+=14;

  while(msg[counter]!=' '){
    // add some limit to the size of the msg 
    if (!isdigit(msg[counter]) || (counter > 200)) return 0;
    cookie[counter1]=msg[counter];
    counter++,counter1++;
  }
  cookie[counter1]='\0';
  counter++;
  if (strncmp(&msg[counter],"status\n",7)) return 0;

  return 1;

}

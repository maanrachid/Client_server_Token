#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> 

void gethostandport(char *s,char *hostname,char* port){
  int counter,counter1;
  counter=0;
  counter1=0;
  if (s[0]==':'){
    printf("no such host name called :\n");
    exit(1);
  }else {
    while (s[counter]!= ':'){
      if (s[counter]=='\0'){
        printf("host name should be followed by :\n");
        exit(1);
      }
      hostname[counter]= s[counter];
      counter+=1;
    }
    hostname[counter]='\0';
    counter++;
    while (s[counter]!='\0'){
      if (isdigit(s[counter])){
        port[counter1]=s[counter];
        counter1++;counter++;
      }else {
        printf("port should be a number\n");
        exit(1);
      }
    }
    port[counter1]='\0';
  }
}

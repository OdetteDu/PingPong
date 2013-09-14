#include "server.h"

char *message = "HTTP/1.1 200 OK \r\nContent-type: text/html \r\n\r\n";
char *message200 = "HTTP/1.1 200 OK \r\n";
char *message400 ="HTTP/1.1 400 Bad Request \r\n";
char *message404 ="HTTP/1.1 404 Not Found \r\n";
char *message500 ="HTTP/1.1 500 Internal Server Error \r\n";
char *message501 ="HTTP/1.1 501 Not Implemented \r\n";

char* getPath(char* str)
{
	int pos;
	char *temp;

	//Check if the first three character is GET
	temp = malloc(sizeof(char) * 4);
	strncpy(temp,str,3);
	temp[4]='\0';
	if(!(strcmp(temp,"GET")==0))
	{
		printf("Violate HTTP Protocol, the first three character is not GET\n");
		return 0;
	}
	free(temp);

	//Get the path by getting the string after GET and before a space
	str+=4;
	pos=strcspn(str," ");
	temp=malloc(sizeof(char) * (pos+1));
	strncpy(temp,str,pos);
	temp[pos+1]='\0';
	return temp;
}

char* getFile(char* root,char* path)
{
	int fd;
	int count;
	char *temp;
	char *file;

	temp=malloc(sizeof(char)*(strlen(root)+strlen(path)));
	strcat(temp,root);
	strcat(temp,path);
	fd = open(temp, O_RDONLY);
	count = read(fd, file, 1000);
	free(temp);
	if(count<0)
	{
		temp=malloc(sizeof(char)*(strlen(message404)));
		temp=message404;
	}
	else
	{
		temp=malloc(sizeof(char)*count+strlen(message));
		strcat(temp,message);
		strcat(temp,file);
	}
	return temp;

}

void SVMode(int sock)
{
}

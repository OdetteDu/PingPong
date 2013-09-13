#include<stdio.h>
#include<malloc.h>
#include<fcntl.h>

#define BUF_LEN 1000

int main()
{
	int fd;	
	char *s="GET /index.html HTTP/1.1 \r\n language Chinese \r\n\r\n";
	char *use;
	int firstRN, count;
	//char *rn = "\r\n";
	char *temp;

	use = malloc(sizeof(char) * 1000);
	temp = malloc(sizeof(char) * 1000);

	printf("%s\n", s);


	firstRN=strcspn(s, "\r\n");
	strncpy(use,s,firstRN+1);
	use[firstRN+1] = '\0';
	printf("Result: %s\n", use);	


	strncpy(temp,use,3);
	temp[4]='\0';
	if(strcmp(temp,"GET")==0)
	{
		  printf("The first three character is GET\n");
	}
	else
	{
		  printf("Violate HTTP Protocol, the first three character is not GET\n");
		  return 0;
	}

	use+=4;

	printf("The new string is %s\n", use);
			  
	firstRN=strcspn(use," ");
	strncpy(temp,use,firstRN);
	temp[firstRN+1]='\0';

	printf("The path is %s\n", temp);

	fd = open("./index.html", O_RDONLY);

	count = read(fd, use, BUF_LEN);

	printf("%d\n", count);
	use[count] = '\0';
	printf("file content:\n%s\n", use);
}

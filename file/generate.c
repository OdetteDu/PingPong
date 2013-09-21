#include <stdio.h>

void main(int argc, char *argv[])
{
	int i, count;
	FILE *fp = fopen("page.html", "w+");

	if (argc < 2)
		count = 100000;
	else
		count = atoi(argv[1]);

	fprintf(fp, "<html>\r\n\t<p>");
	for (i = 0; i < count; i++) {
		if (i % 100 == 0)
			fprintf(fp, "\r\n");
		fprintf(fp, "%c", 'A' + i % 26);
	}
	fprintf(fp, "</p>\r\n\t<p>Finish!!!</p>\r\n</html>");

	fclose(fp);
}

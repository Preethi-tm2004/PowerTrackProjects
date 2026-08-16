
#include <stdio.h>
#include "s2html_event.h"
#include "s2html_conv.h"

/********** main **********/

int main (int argc, char *argv[])
{
	FILE * sfp, *dfp; // source and destination file descriptors 
	pevent_t *event;
	char dest_file[100];

	if(argc < 2)
	{
		printf("\n\033[31mError ! please enter file name and mode\033[0m\n");
		printf("\033[32mUsage: <executable> <file name> \033[0m\n");
		printf("\033[32mExample : ./a.out abc.txt\033[0m\n\n");
		return 1;
	}

/* Conditional compilation for debugging */
#ifdef DEBUG
	printf("File to be opened : %s\n", argv[1]);
#endif

	/* open the file */
	if(NULL == (sfp = fopen(argv[1], "r")))
	{
		printf("\033[31mError! File %s could not be opened\033[0m\n", argv[1]);
		return 2;
	}
	/* Check for output file */
	if (argc > 2)
	{
		sprintf(dest_file, "%s.html", argv[2]);
	}
	else
	{
		sprintf(dest_file, "%s.html", argv[1]);
	}
	
	/* open dest file */
	if (NULL == (dfp = fopen(dest_file, "w")))
	{
		printf("\033[31mError! could not create %s output file\033[0m\n", dest_file);
		return 3;
	}

	/* write HTML starting Tags */
	html_begin(dfp, HTML_OPEN);

	/* Read from src file convert into html and write to dest file */
	do
	{
		event = get_parser_event(sfp);
		/* call sourc_to_html */
		source_to_html(dfp, event);
	//printf("In main : Event = %d\n", event);
	} while (event->type != PEVENT_EOF);

	/* Call start_or_end_conv function for ending the convertation */
	html_end(dfp, HTML_CLOSE);
	
	printf("\n\033[32mOutput file %s generated\033[0m\n", dest_file);

	printf("\n\033[32mSource to html conversion is done successfully\033[0m\n");
/* close file */
	fclose(sfp);
	fclose(dfp);

	return 0;
}

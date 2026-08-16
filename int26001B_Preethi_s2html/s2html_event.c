
#include <stdio.h>
#include <string.h>
#include "s2html_event.h"

#define SIZE_OF_SYMBOLS (sizeof(symbols))
#define SIZE_OF_OPERATORS (sizeof(operators))
#define WORD_BUFF_SIZE	100

/********** Internal states and event of parser **********/
typedef enum
{
	PSTATE_IDLE,
	PSTATE_PREPROCESSOR_DIRECTIVE,
	PSTATE_SUB_PREPROCESSOR_MAIN,
	PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD,
	PSTATE_SUB_PREPROCESSOR_ASCII_CHAR,
	PSTATE_HEADER_FILE,
	PSTATE_RESERVE_KEYWORD,
	PSTATE_NUMERIC_CONSTANT,
	PSTATE_STRING,
	PSTATE_SINGLE_LINE_COMMENT,
	PSTATE_MULTI_LINE_COMMENT,
	PSTATE_ASCII_CHAR
}pstate_e;

/********** global variables **********/

/* parser state variable */
static pstate_e state = PSTATE_IDLE;

/* sub state is used only in preprocessor state */
static pstate_e state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;

/* event variable to store event and related properties */
static pevent_t pevent_data;
static int event_data_idx=0;

static char word[WORD_BUFF_SIZE];
static int word_idx=0;

static int escape;

static char* res_kwords_data[] = {"const", "volatile", "extern", "auto", "register",
   						   "static", "signed", "unsigned", "short", "long", 
						   "double", "char", "int", "float", "struct", 
						   "union", "enum", "void", "typedef", ""
						  };

static char* res_kwords_non_data[] = {"goto", "return", "continue", "break", 
							   "if", "else", "for", "while", "do", 
							   "switch", "case", "default","sizeof", ""
							  };

static char operators[] = {'/', '+', '*', '-', '%', '=', '<', '>', '~', '&', ',', '!', '^', '|'};
static char symbols[] = {'(', ')', '{', '}', '[', ']', ':', ';'};

/********** state handlers **********/
pevent_t * pstate_idle_handler(FILE *fd, int ch);
pevent_t * pstate_single_line_comment_handler(FILE *fd, int ch);
pevent_t * pstate_multi_line_comment_handler(FILE *fd, int ch);
pevent_t * pstate_numeric_constant_handler(FILE *fd, int ch);
pevent_t * pstate_string_handler(FILE *fd, int ch);
pevent_t * pstate_header_file_handler(FILE *fd, int ch);
pevent_t * pstate_ascii_char_handler(FILE *fd, int ch);
pevent_t * pstate_reserve_keyword_handler(FILE *fd, int ch);
pevent_t * pstate_preprocessor_directive_handler(FILE *fd, int ch);
pevent_t * pstate_sub_preprocessor_main_handler(FILE *fd, int ch);

/********** Utility functions **********/

/* function to check if given word is reserved key word */
static int is_reserved_keyword(char *word)
{
	int idx = 0;

	/* search for data type reserved keyword */
	while(*res_kwords_data[idx])
	{
		if(strcmp(res_kwords_data[idx++], word) == 0)
			return RES_KEYWORD_DATA;
	}

	idx = 0; // reset index
	/* search for non data type reserved key word */
	while(*res_kwords_non_data[idx])
	{
		if(strcmp(res_kwords_non_data[idx++], word) == 0)
			return RES_KEYWORD_NON_DATA;
	}

	return 0; // word did not match, return false
}

/* function to check symbols */
static int is_symbol(char c)
{
	size_t idx;
	for(idx = 0; idx < SIZE_OF_SYMBOLS; idx++)
	{
		if(symbols[idx] == c)
			return 1;
	}

	return 0;
}

/* function to check operator */
static int is_operator(char c)
{
	size_t idx;
	for(idx = 0; idx < SIZE_OF_OPERATORS; idx++)
	{
		if(operators[idx] == c)
			return 1;
	}

	return 0;
}

/* to set parser event */
static void set_parser_event(pstate_e s, pevent_e e)
{
	pevent_data.data[event_data_idx] = '\0';
	pevent_data.length = event_data_idx;
	event_data_idx = 0;
	state = s;
	pevent_data.type = e;
	escape = 0;
}


/************ Event functions **********/

/* This function parses the source file and generate 
 * event based on parsed characters and string
 */
pevent_t *get_parser_event(FILE *fd)
{
	int ch;
	pevent_t *evptr = NULL;
	/* Read char by char */
	while((ch = fgetc(fd)) != EOF)
	{
		switch(state)
		{
			case PSTATE_IDLE :
				if((evptr = pstate_idle_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_SINGLE_LINE_COMMENT :
				if((evptr = pstate_single_line_comment_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_MULTI_LINE_COMMENT :
				if((evptr = pstate_multi_line_comment_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_PREPROCESSOR_DIRECTIVE :
				if((evptr = pstate_preprocessor_directive_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_RESERVE_KEYWORD :
				if((evptr = pstate_reserve_keyword_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_NUMERIC_CONSTANT :
				if((evptr = pstate_numeric_constant_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_STRING :
				if((evptr = pstate_string_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_HEADER_FILE :
				if((evptr = pstate_header_file_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_ASCII_CHAR :
				if((evptr = pstate_ascii_char_handler(fd, ch)) != NULL)
					return evptr;
				break;
			default : 
				printf("unknown state\n");
				state = PSTATE_IDLE;
				break;
		}
	}

	/* Check is EOF is reached while processing a single line comment */
	if(state == PSTATE_SINGLE_LINE_COMMENT && event_data_idx)
	{
		/* Single line comment ends at EOF */
		set_parser_event(PSTATE_IDLE, PEVENT_SINGLE_LINE_COMMENT);

		return &pevent_data;
	}

	/* end of file is reached, move back to idle state and set EOF event */
	set_parser_event(PSTATE_IDLE, PEVENT_EOF);

	return &pevent_data; // return final event
}


/********** IDLE state Handler **********
 * Idle state handler identifies
 ****************************************/

pevent_t * pstate_idle_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch(ch)
	{
		case '\'' : // begining of ASCII char 
		{
			if(event_data_idx)
			{
				//return the regular expression if already it was there in the buffer
				fseek(fd, -1L, SEEK_CUR);

				set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);

				return &pevent_data;
			}
			
			pevent_data.data[event_data_idx++] = ch;
			state = PSTATE_ASCII_CHAR;
			break;
		}

		case '/' :
			pre_ch = ch;
			if((ch = fgetc(fd)) == '*') // multi line comment
			{
				if(event_data_idx) // we have regular exp in buffer first process that
				{
					fseek(fd, -2L, SEEK_CUR); // unget chars
					set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
					return &pevent_data;
				}
				else //	multi line comment begin 
				{
					state = PSTATE_MULTI_LINE_COMMENT;
					pevent_data.data[event_data_idx++] = pre_ch;
					pevent_data.data[event_data_idx++] = ch;
				}
			}
			else if(ch == '/') // single line comment
			{
				if(event_data_idx) // we have regular exp in buffer first process that
				{
					fseek(fd, -2L, SEEK_CUR); // unget chars
					set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
					return &pevent_data;
				}
				else //	single line comment begin
				{
					state = PSTATE_SINGLE_LINE_COMMENT;
					pevent_data.data[event_data_idx++] = pre_ch;
					pevent_data.data[event_data_idx++] = ch;
				}
			}
			else // it is regular exp
			{
				pevent_data.data[event_data_idx++] = pre_ch;
				pevent_data.data[event_data_idx++] = ch;
			}
			break;

		case '#' :
		{
			if(event_data_idx)
			{
				fseek(fd, -1L, SEEK_CUR);

				set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);

				return &pevent_data;
			}

			pevent_data.data[event_data_idx++] = ch;

			/* Start processing a new processor directive */
			state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;

			state = PSTATE_PREPROCESSOR_DIRECTIVE;
			break;
		}

		case '"':
		{
			if(event_data_idx)
			{
				//return the regular expression if already it was there in the buffer
				fseek(fd, -1L, SEEK_CUR);

				set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);

				return &pevent_data;
			}

			pevent_data.data[event_data_idx++] = ch;
			state  = PSTATE_STRING;
			break;
		}

		case '0' ... '9' : // detect numeric constant
		{
			if(event_data_idx)
			{
				fseek(fd, -1L, SEEK_CUR);

				set_parser_event(PSTATE_NUMERIC_CONSTANT, PEVENT_REGULAR_EXP);

				return &pevent_data;
			}

			pevent_data.data[event_data_idx++] = ch;
			state = PSTATE_NUMERIC_CONSTANT;
			break;
		}

		case 'a' ... 'z' : // could be reserved key word
		case 'A' ... 'Z' :
		{
			if(event_data_idx)
			{
				fseek(fd, -1L, SEEK_CUR);
				set_parser_event(PSTATE_RESERVE_KEYWORD, PEVENT_REGULAR_EXP);
				return &pevent_data;
			}

			word[word_idx++] = ch;
			state = PSTATE_RESERVE_KEYWORD;
			break;
		}

		default : // Assuming common text starts by default.
			if(is_operator(ch))
			{
				if(event_data_idx)
				{
					fseek(fd, -1L, SEEK_CUR);
					set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
					return &pevent_data;
				}

				pevent_data.data[event_data_idx++] = ch;
				set_parser_event(PSTATE_IDLE, PEVENT_OPERATOR);
				return &pevent_data;
			}

			if(is_symbol(ch))
			{
				if(event_data_idx)
				{
					fseek(fd, -1L, SEEK_CUR);
					set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
					return &pevent_data;
				}

				pevent_data.data[event_data_idx++] = ch;
				set_parser_event(PSTATE_IDLE, PEVENT_SYMBOL);
				return &pevent_data; 
			}

			pevent_data.data[event_data_idx++] = ch;
			break;
	}

	return NULL;
}

pevent_t * pstate_preprocessor_directive_handler(FILE *fd, int ch)
{
	switch(state_sub)
	{
		case PSTATE_SUB_PREPROCESSOR_MAIN :
			return pstate_sub_preprocessor_main_handler(fd, ch);

		case PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD :
			return pstate_reserve_keyword_handler(fd, ch);

		case PSTATE_SUB_PREPROCESSOR_ASCII_CHAR :
			return pstate_ascii_char_handler(fd, ch);

		default :
				printf("unknown state\n");
				state = PSTATE_IDLE;
	}

	return NULL;
}

pevent_t * pstate_sub_preprocessor_main_handler(FILE *fd, int ch)
{
	/* write a switch case here to detect several events here
	 * This state is similar to Idle state with slight difference
	 * in state transition.
	 * return event data at the end of event
	 * else return NULL
	 */
	(void)fd;

	 switch(ch)
	 {
		case 'a' ... 'z':
		case 'A' ... 'Z':
			word[word_idx++] = ch;
			state_sub = PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD;
			break;

		case '<':
			/* Set property for system header */
			pevent_data.property = STD_HEADER_FILE;

			/* Start collecting header file name */
			state = PSTATE_HEADER_FILE;
			break;

		case '"':
			/* Store the opening double quote */
			pevent_data.data[event_data_idx++] = ch;

			/* Set property for user-defined header */
			pevent_data.property = USER_HEADER_FILE;

			/* Start collecting header file name */
			state = PSTATE_HEADER_FILE;
			break;

		case ' ':
		case '\t':
			/* Ignore spaces between dirsctive and header */
			break;

		default:
			pevent_data.data[event_data_idx++] = ch;
			break;
	 }

	 return NULL;
}

pevent_t * pstate_header_file_handler(FILE *fd, int ch)
{
	/* write code here to store header file name
	 * return event data at the end of event
	 * else return NULL
	*/

	(void)fd;

	 /* Check for the closing character*/
	if((pevent_data.property == STD_HEADER_FILE && ch == '>') || (pevent_data.property == USER_HEADER_FILE&& ch == '"'))
	{
		/* Store the closing character for user header */
		if(pevent_data.property == USER_HEADER_FILE)
		{
			pevent_data.data[event_data_idx++] = ch;
		}

		/* Header file event is complete */
		set_parser_event(PSTATE_IDLE, PEVENT_HEADER_FILE);

		/* Return the completed header file event */
		return &pevent_data;
	}

	/*Store the header file name */
	pevent_data.data[event_data_idx++] = ch;
	
	/* Continue collecting the header file */
	return NULL;
}

pevent_t * pstate_reserve_keyword_handler(FILE *fd, int ch)
{
	/* write code here to store words
	 * return event data at the end of event
	 * else return NULL
	*/

	int ret;

	if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
	{
		word[word_idx++] = ch;
	}
	else
	{
		/* End of word */
		word[word_idx] = '\0';

		/* check whether word is a reserved keyword */
		ret = is_reserved_keyword(word);

		if(state == PSTATE_PREPROCESSOR_DIRECTIVE && state_sub == PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD && strcmp(word, "include") == 0)
		{
			/* Append the preprocessor keyword to '#' */
			strcpy(&pevent_data.data[event_data_idx], word);
			event_data_idx += strlen(word);

			/* Rest wor buffer */
			word_idx = 0;

			/* Put '<' back into the input stream */
			fseek(fd, -1L, SEEK_CUR);

			/* Change sub-state to header file processing */
			state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;

			/* Gnerate preprocessor directive event */
			set_parser_event(PSTATE_PREPROCESSOR_DIRECTIVE, PEVENT_PREPROCESSOR_DIRECTIVE);

			return &pevent_data;
		}

		if(ret)
		{
			/* Copy keyword into event data */
			strcpy(pevent_data.data, word);

			 /* Tell set_parser_event() how much data is present */
            event_data_idx = strlen(word);

			/* Set keyword property */
			pevent_data.property = ret;

			/* Reset word buffer */
			word_idx = 0;

			/* Put delemiter back into iput stream */
			fseek(fd, -1L, SEEK_CUR);

			/* Generate reserved keyword event */
			set_parser_event(PSTATE_IDLE, PEVENT_RESERVE_KEYWORD);

			return &pevent_data;
		}
		else
		{
			/* Not a keyword -> regular expression */
			strcpy(pevent_data.data, word);

			/* Tell set_parser_event() how much data is present */
            event_data_idx = strlen(word);

			/* Reset word buffer */
			word_idx = 0;

			/* Put delimeter back */
			fseek(fd, -1L, SEEK_CUR);

			/* Generate regular expression event */
			set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);

			return &pevent_data;
		}
	}
	return NULL;
}

pevent_t * pstate_numeric_constant_handler(FILE *fd, int ch)
{
	/* write code here to store digits
	 * return event data at the end of event
	 * else return NULL
	 */

	 if((ch >= '0' && ch <= '9') || ch == '.')
	 {
		/* Stores the character in the event data */
		pevent_data.data[event_data_idx++] = ch;
	 }
	 else
	 {
		/* Numeric constant has ended */
		fseek(fd, -1L, SEEK_CUR);

		/* Numeric constant event is complete */
		set_parser_event(PSTATE_IDLE, PEVENT_NUMERIC_CONSTANT);

		return &pevent_data;
	 }

	 //numeric constant is not yet complete[keep collecting until event is done]
	 return NULL;
}

pevent_t * pstate_string_handler(FILE *fd, int ch)
{
	/* write code here to store string
	 * return event data at the end of event
	 * else return NULL
	*/

	(void)fd;

	/* Stores the character in the event data */
	pevent_data.data[event_data_idx++] = ch;

	if(ch == '\\')
	{
		escape = 1;
		return NULL;
	}

	/* check whether the closing double quote is reached */
	if(ch == '"' && escape == 0)
	{
		/* string event is complete */
		set_parser_event(PSTATE_IDLE, PEVENT_STRING);

		/* return the completed string event */
		return &pevent_data;
	}

	/* Reset escape flag after processing escaped character */
    escape = 0;

	/* string is not completed yet[keep collecting until event is done ] */
	return NULL;
}

pevent_t * pstate_single_line_comment_handler(FILE *fd, int ch)
{
	(void)fd;

	switch(ch)
	{
		case '\n' : /* single line comment ends here */
			pevent_data.data[event_data_idx++] = ch;
			set_parser_event(PSTATE_IDLE, PEVENT_SINGLE_LINE_COMMENT);
			return &pevent_data;
		default :  // collect single line comment chars
			pevent_data.data[event_data_idx++] = ch;
			break;
	}

	return NULL;
}

pevent_t * pstate_multi_line_comment_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch(ch)
	{
		case '*' : /* comment might end here */
			pre_ch = ch;
			pevent_data.data[event_data_idx++] = ch;
			if((ch = fgetc(fd)) == '/')
			{
				pre_ch = ch;
				pevent_data.data[event_data_idx++] = ch;
				set_parser_event(PSTATE_IDLE, PEVENT_MULTI_LINE_COMMENT);
				return &pevent_data;
			}
			else // multi line comment string still continued
			{
				pevent_data.data[event_data_idx++] = ch;
			}
			break;
		case '/' :
			/* go back by two steps and read previous char */
			fseek(fd, -2L, SEEK_CUR); // move two steps back
			pre_ch = fgetc(fd); // read a char
			fgetc(fd); // to come back to current offset

			pevent_data.data[event_data_idx++] = ch;
			if(pre_ch == '*')
			{
				set_parser_event(PSTATE_IDLE, PEVENT_MULTI_LINE_COMMENT);
				return &pevent_data;
			}
			break;
		default :  // collect multi-line comment chars
			pevent_data.data[event_data_idx++] = ch;
			break;
	}

	return NULL;
}

pevent_t * pstate_ascii_char_handler(FILE *fd, int ch)
{
/* write code here to store ASCII chars
	* return event data at the end of event
	* else return NULL
	*/

	(void)fd;

	/* Stores the current ASCII character */
	pevent_data.data[event_data_idx++] = ch;

	/* Check whether the character is escaped */
	if(ch == '\\')
	{
		/* Toggle escape state for consecutive backslash */
		escape = !escape;
		return NULL;
	}

	/* Check for closing single quote */
	if(ch == '\'' && escape == 0)
	{
		/* ASCII character event is complete */
		set_parser_event(PSTATE_IDLE, PEVENT_ASCII_CHAR);

		return &pevent_data;
	}

	/* Non-backslash character is complete */
	escape = 0;

	/* ASCII character is not complete yet[keep collecting until the event is done] */
	return NULL;
}

/**** End of file ****/

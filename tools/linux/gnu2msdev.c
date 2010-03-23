#define _MAX_LINE_LEN 300
#define _SEPS ":"
#define _FROM " from "

#include <stdio.h>
#include <string.h>

static char _line[_MAX_LINE_LEN];
static char _tmp[_MAX_LINE_LEN];

int _isNum (char *x_str)
{
	size_t l_i;

	// ~FOR 
	for (l_i=0;l_i<strlen(x_str);l_i++)
	{
		//  ~IF 
		if ( !((x_str[l_i] >= '0') && (x_str[l_i] <= '9')) )
		{ //  ~THEN 
			return 0;
		} // ~ENDIF 
	} // ~ENDFOR 

	return 1;
}

int main (int x_argc, char *x_argv[ ] )
{
	FILE *l_in,*l_out;
	char *l_file;
	char *l_num;
	char *l_end;
	char *l_from;
	int l_ret;

	if (x_argc > 1)
	{
		fprintf (stderr, "no arguments it uses stdin, stdout\n");
		return 1;
	}

	l_in = stdin;
	l_out = stdout;

//	fputs("coucou\n",l_out);
	l_ret = 0;
	while( fgets(_line,_MAX_LINE_LEN,l_in) )
	{
//		printf ("*%s", _line);
		strcpy (_tmp, _line);
		l_file = strtok( _tmp, _SEPS);
		//  ~IF 
		if ( l_file != NULL )
		{ //  ~THEN 
//			printf ("*file: %s\n", l_file);
			l_num = strtok( NULL, _SEPS);
			//  ~IF 
			if ( l_num != NULL )
			{ //  ~THEN 
				l_end = strtok(NULL, "");
//				printf ("*num: %s\n", l_num);
				l_num = strtok(l_num, " ");
				//  ~IF number
				if (_isNum (l_num) == 1)
				{ //  ~THEN 
//					printf ("*is num\n");
					l_from = strstr (l_file, _FROM);
					//  ~IF 
					if ( l_from == NULL )
					{ //  ~THEN 
						strcpy (_line, l_file);
						strcat (_line, "(");
						strcat (_line, l_num);
						strcat (_line, "): ");
						//  ~IF 
						if (l_end != NULL)
						{ //  ~THEN 
							strcat (_line, l_end);
						} // ~ENDIF 
						//  ~IF 
						if ( strstr (_line, "warning:") == NULL )
						{ //  ~THEN 
							l_ret = 1;
						} // ~ENDIF 
					} // ~ENDIF 
				} // ~ENDIF 
			} // ~ENDIF 
		} // ~ENDIF 

		fputs(_line,l_out);
	}

	return l_ret;
}


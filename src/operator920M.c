// Elliott 900 Operator - Andrew herbert 30/04/2021

// Provides an operator control facility for use of an Elliott 920M computer.

// Usage: 900op [-r reader-file] [-p punch-file]
//
// At start up resets the 920M to begin executing initial instructions. paper
// tape input is taken from reader-file in binary as requested by the 920M.
// An eero indication is given and the program aborts if there is no reader-
// file or anattempt is made to read beyond the end of reader-file.

// Similarly paper tape output is written in binary to punch-file.  If there
// is no punch-file or a file writing error occurs the program aborts.

// Teletype input/output is not yet supported and will be treated as paper tape
// input/output.


/*******************************************/
/*               HEADER FILES              */
/*******************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <popt.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>


/*******************************************/
/*                  DEFINES                */
/*******************************************/


#define TRUE  1
#define FALSE 0

#define EXIT_SUCCESS           0
#define EXIT_FAILURE_NO_PTS    1
#define EXIT_FAILURE_LOST_PTS  2
#define EXIT_FAILURE_NO_INPUT  3
#define EXIT_FAILURE_NO_OUTPUT 4
#define EXIT_FAILURE_INTERRUPT 5

#define PTS_OPEN_FAIL        "Failed to open connection to paper tape station"
#define PTS_CONNECTION_LOST  "Lost connection to paper tape station\n"
#define READER_OPEN_FAIL     "Cannot open input file %s: "
#define PUNCH_OPEN_FAIL      "Cannot open output file %s: "
#define INTERRUPT            "Program terminated by interrupt\n"

#define OPTSTR "i:a:"
#define USAGE_FMT  "%s [-i inputfile] [-a asciifile]"


/*******************************************/
/*                  GLOBALS                */
/*******************************************/



FILE *reader_file  = NULL;
FILE *punch_file   = NULL;
char *reader_name  =  NULL;
char *punch_name   = NULL;

extern int errno;


/*******************************************/
/*                 FUNCTIONS               */
/*******************************************/

int  main(char **argv, int argc));
int  get_cmd_ch(void);
void put_cmd_ch(int);
void decode_args(char **argv, int argc);


/*******************************************/
/*                   MAIN                  */
/*******************************************/



int main ()
{
  FILE *cmds = NULL; // for connection to PTS

  signal(SIGINT, catchInt); // allow control-C to end cleanly
 
  puts("920M Console Starting\n");
  
  reset_pts();

  /* open connection to paper tape station */
  if ( !(cmds = open("/dev/XXX", O_RW)) )
    {
      perror(PTS_OPEN_FAIL);
      tidy_exit(FAIL_NO_PTS);
      /* NOT REACHED */
    }
  
    
  /* Loop reading request from paper tape station */
  while ( TRUE )
    {
      int ch;
      ch1 = get_cmd_ch();
      switch ( ch )
        {
           case 'R': // read from input file
	     get_cmd_ch(); // absorb newline
	     put_cmd_ch(cmds, get_reader_ch());
	     put_cmd_ch('\n'); // acknowledge
	     break;

	   case'W': // write to output file
	     put_punch_ch(get_cmd_ch()); // character to output
	     get_cmd_ch(); // absorb newline
	     put_cmd_ch('\n'); // acknowledge
	     break;

	   case 'S': // read from teletype
	     get_cmd_ch(); // absorb newline
	     putc(cmds, get_tty_ch());
	     put_cmd_ch('\n'); // acknowledge
	     break;

	   case'T': // write to output file
	     put_tty_ch(get_cmd_ch()); // character to output
	     get_cmd_ch(); // absorb newline
	     put_cmd_ch('\n'); // acknowledge
	     break:

           default:
	     fprintf(stderr, "Protocol failure - received %d (%c)\n");
	     tidy_exit(EXIT_FAILURE_PROTOCOL);
	     /* NOT REACHED */
	}
    }
}


/*******************************************/
/*       PTS COMMAND INTERFACE             */
/*******************************************/


int get_cmd_ch ()
{
  int ch;
  if ( (ch = getc(cmds)) == EOF )
    {
      puts(PTS_CONNECTION_LOST);
      tidy_exit(EXIT_FAILURE_LOST_PTS);
      /* NOT REACHED */
    }
  return ch;
}

  void put_cmd_ch (const int ch)
  {
    if ( putc(ch, cmds) == EOF )
    {
      puts(PTS_CONNECTION_LOST);
      tidy_exit(EXIT_FAILURE_LOST_PTS);
      /* NOT REACHED */
    }
}


/*******************************************/
/*          READ / WRITE FILES             */
/*******************************************/


  int get_reader_ch()
  {
    if ( reader_file == NULL )
      {
	if ( (reader_file = fopen(reader_name, "rb")) != 0 )
	  {
	    fprintf(stderr, READER_OPEN_FAIL, reader_name);
	    perror();	    
	    exit(EXIT_FAILURE__NO_INPUT);
	    /* NOT REACHED */
	  }
      }
    return getc(reader_file);
  }

  void put_punch_ch()
  {
    if ( punch_file == NULL )
      {
	if ( (punch_file = fopen(punch_name, "ab")) != 0 )
	  {
	    fprintf(stderr, PUNCH_OPEN_FAIL, punch_name);
	    perror();
	    exit(EXIT_FAILURE_NO_OUTPUT);
	    /* NOT REACHED */
	  }
      }
  }


/*******************************************/
/*               RESET PTS                 */
/*******************************************/
	
void reset_pts()
{
  gpio_put(NOPOWER_PIN, 1);
  sleep(1.0);
  gpio_put(NOPOWER_PIN, 0);
}




/*******************************************/
/*            ARGUMENT DECODING            */
/*******************************************/



void decode_args(const char *argv, int arg)
{
  poptContext optCon;
  
  struct poptOption optionsTable[] = {
      {"reader",  'r', POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,
       &ptrPath, 0, "paper tape reader input", "file"},
      {"punch",   'p', POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,
       &punPath, 0, "paper tape punch output", "file"},
      POPT_AUTOHELP
      POPT_TABLEEND
    };

  optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);

  while ( (c = poptGetNextOpt(optCon)) > 0 ) ;

  if ( c < -1 ) // bombed out on an error
    {
      fprintf(stderr, "%s: %s\n", poptBadOption(optCon, 0), poptStrerror(c));
      exit(EXIT_FAILURE);
      /* NOT REACHED */
    }
  
  if ( (buffer = (char *) poptGetArg(optCon)) != NULL ) // check for extra arguments
       usage(optCon, EXIT_FAILURE, "unexpected argument", buffer);

  poptFreeContext(optCon); // release context
}

void usage (poptContext optCon, INT32int exitcode, char *error, char *addl)
{
  poptPrintUsage(optCon, stderr, 0);
  if (error) fprintf(stderr, "%s: %s\n", error, addl);
  exit(exitcode);
}


/*******************************************/
/*                TIDY EXIT                */
/*******************************************/


  void tidy_exit(int reason)
  {
    close input;
    close output;
    close tty;
    close cmds; // can PTS detect this and restart?
    exit(reason);
    /* NOT REACHED */
  }

void catchInt(INT32 sig, void (*handler)(int)) {
  fputsf(INTERRUPT, stderr);
  tidyExit(EXIT_FAILURE_INTERRUPT);
}



  
    
					       
	  

	     
           








       
       
      
    


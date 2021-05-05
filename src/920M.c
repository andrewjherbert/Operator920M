// Elliott 900 Operator - Andrew herbert 05/05/2021

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
#include <sys/types.h>
#include <popt.h>
#include <signal.h>
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


int  usb           = 0;

FILE *reader_file  = NULL;
FILE *punch_file   = NULL;
char *reader_name  = NULL;
char *punch_name   = NULL;

char buf[1];

extern int errno;


/*******************************************/
/*                 FUNCTIONS               */
/*******************************************/

int  main(char **argv, int argc));
int  get_usb_ch(void);              // read from USB
void put_usb_ch(int ch);            // write to usb
void decode_args(char **argv, int argc);
void catchInt(INT32 sig, void (*handler)(int));


/*******************************************/
/*                   MAIN                  */
/*******************************************/


int main ()
{
    signal(SIGINT, catchInt); // allow control-C to end cleanly
 
  puts("920M Console Starting\n");

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
      ch1 = get_usb_ch();
      switch ( ch )
        {
           case 'R': // read from input file
	     put_usb_ch(get_reader_ch());
	     break;

	   case'W': // write to output file
	     put_punch_ch(get_usb_ch()); // character to output
	     break;

	     /*	   case 'S': // read from teletype
	     put_usb_ch(get_tty_ch());
	     break;

	   case'T': // write to output file
	     put_tty_ch(get_usb_ch()); // character to output
	     break: */

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


int get_usb_ch ()
{
  if ( read(usb, buf, 1) == EOF )
    {
      perror(PTS_CONNECTION_LOST);
      tidy_exit(EXIT_FAILURE_LOST_PTS);
      /* NOT REACHED */
    }
  return buf[0];
}

  void put_usb_ch (const int ch)
  {
    buf[0] = ch;
    if ( write(cmd, buf, 1) == EOF )
    {
      perror(PTS_CONNECTION_LOST);
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
	if ( (reader_file = fopen(reader_name, "r")) != 0 )
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
	if ( (punch_file = fopen(punch_name, "w")) != 0 )
	  {
	    fprintf(stderr, PUNCH_OPEN_FAIL, punch_name);
	    perror();
	    exit(EXIT_FAILURE_NO_OUTPUT);
	    /* NOT REACHED */
	  }
      }
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
    if ( input  != NULL ) fclose(input);
    if ( output != NULL ) fclose(output);
    if ( usb)             close(usb);
    exit(reason);
    /* NOT REACHED */
  }

void catchInt(INT32 sig, void (*handler)(int)) {
  fputsf(INTERRUPT, stderr);
  tidyExit(EXIT_FAILURE_INTERRUPT);
}



  
    
					       
	  

	     
           








       
       
      
    


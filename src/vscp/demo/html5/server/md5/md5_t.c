/*
 * A little test program for md5 code which does a md5 on all the
 * bytes sent to stdin.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"

static	char	*rcs_id =
  "$Id: md5_t.c,v 1.3 2010-05-07 13:58:18 gray Exp $";

typedef struct {
  char		*ss_string;			/* test string */
  char		*ss_sig;			/* result signature */
} str_sig_t;

/* MD5 Test Suite from RFC1321 */
static	str_sig_t	tests[] = {
  { "", "d41d8cd98f00b204e9800998ecf8427e" },
  { "a", "0cc175b9c0f1b6a831c399e269772661" },
  { "abc", "900150983cd24fb0d6963f7d28e17f72" },
  { "message digest", "f96b697d7cb7938d525a2f31aaf161d0" },
  { "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" },
  { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "d174ab98d277d9f5a5611c2c9f419d9f" },
  { "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a" },
  
  { "This string is precisely 56 characters long for a reason", "93d268e9bef6608ff1a6a96adbeee106" },
  { "This string is exactly 64 characters long for a very good reason", "655c37c2c8451a60306d09f2971e49ff" },
  { "This string is also a specific length.  It is exactly 128 characters long for a very good reason as well. We are testing bounds.", "2ac62baa5be7fa36587c55691c026b35" },
  { NULL }
};

/*
 * Print a signature
 */
static	void	print_sig(const unsigned char *sig)
{
  const unsigned char	*sig_p;
  
  for (sig_p = sig; sig_p < sig + MD5_SIZE; sig_p++) {
    (void)printf("%02x", *sig_p);
  }
}

/*
 * Read in from stdin and run MD5 on the input
 */
static	void	read_file(const char *filename)
{
  unsigned char	sig[MD5_SIZE];
  char		buffer[4096];
  md5_t		md5;
  int		ret;
  FILE		*stream;
  
  if (strcmp(filename, "-") == 0) {
    stream = stdin;
  }
  else {
    stream = fopen(filename, "r");
    if (stream == NULL) {
      perror(filename);
      exit(1);
    }
  }
  
  md5_init(&md5);
  
  /* iterate over file */
  while (1) {
    
    /* read in from our file */
    ret = fread(buffer, sizeof(char), sizeof(buffer), stream);
    if (ret <= 0)
      break;
    
    /* process our buffer buffer */
    md5_process(&md5, buffer, ret);
  }
  
  md5_finish(&md5, sig); 
  
  if (stream != stdin) {
    (void)fclose(stream);
  }
  
  (void)printf("%25s '", "Resulting signature:");
  print_sig(sig);
  (void)printf("'\n");

  /* convert to string to print */
  md5_sig_to_string(sig, buffer, sizeof(buffer));
  (void)printf("%25s '%s'\n", "Results of md5_to_string:", buffer);
  
  /* now we convert it from string back into the sig */
  md5_sig_from_string(sig, buffer);
  
  (void)printf("%25s '", "After md5_from_string:");
  print_sig(sig);
  (void)printf("'\n");
}

static	void	run_tests(void)
{
  unsigned char	sig[MD5_SIZE], sig2[MD5_SIZE];
  char		str[33];
  str_sig_t	*test_p;
  
  /* run our tests */
  for (test_p = tests; test_p->ss_string != NULL; test_p++)
  {
    /* calculate the sig for our test string */
    md5_buffer(test_p->ss_string, strlen(test_p->ss_string), sig);
    
    /* convert from the sig to a string rep */
    md5_sig_to_string(sig, str, sizeof(str));
    if (strcmp(str, test_p->ss_sig) == 0)
	{
      (void)printf("Sig for '%s' matches '%s'\n",
		   test_p->ss_string, test_p->ss_sig);
    }
    else 
	{
      (void)printf("ERROR: Sig for '%s' is '%s' not '%s'\n",
		   test_p->ss_string, test_p->ss_sig, str);
    }
    
    /* convert from the string back into a MD5 signature */
    md5_sig_from_string(sig2, str);
    if (memcmp(sig, sig2, MD5_SIZE) == 0)
	{
      (void)printf("  String conversion also matches\n");
    }
    else {
      (void)printf("  ERROR: String conversion for '%s' failed\n",
		   test_p->ss_sig);
    }
  }
}

/*
 * print the usage message
 */
static	void	usage(void)
{
  (void)fprintf(stderr, "Usage: md5_t [-r file]\n");
  exit(1);      
}

int	main(int argc, char **argv)
{
  char	do_read = 0;
  char	*infile = NULL;
  
  argc--, argv++;
  
  /* process the args */
  for (; *argv != NULL; argc--, argv++) {
    
    if (**argv != '-') {
      continue;
    }
    
    switch (*(*argv + 1)) {
      
    case 'r':
      do_read = 1;
      argc--;
      argv++;
      if (argc == 0) {
	usage();
      }
      infile = *argv;
      break;
      
    default:
      usage();
      break;
    }
  }
  
  if (argc > 0) {
    usage();
  }
  
  /* do we need to read in from stdin */
  if (do_read) {
    read_file(infile);
  }
  else {
    run_tests();
  }
  
  return 0;
}

/*
oakpark
version 0.2
(module file)

by:
Iman S. H. Suyoto

Filename: oakpark.c.
Compiler: any ISO C89 (or later) compiler.

Copyright (C) 2005 Iman S. H. Suyoto
 
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the
following conditions:
 
The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the
Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include "oakpark.h"

#ifndef OAKPARK_BUF_SIZE
#define OAKPARK_BUF_SIZE 1024
#endif

/*
function: oakpark_get_line
parameter: FILE *: input stream
return: NULL: on failure
        pointer to string on success. This pointer should be
        free()'d later.
*/
char * oakpark_get_line(FILE * ins, size_t * len)
{ char buf[OAKPARK_BUF_SIZE + 1];
  char * result;
  char * curr;
  size_t num_of_bytes_read = 0;
  size_t num_of_bytes_remaining = OAKPARK_BUF_SIZE;
  size_t mult = 1;

  assert(ins != NULL);
  result = malloc(OAKPARK_BUF_SIZE + 1);
  if(!result)
  { return NULL;
    }
  curr = result;
  while(fgets(buf, sizeof buf, ins))
  { size_t buf_len = strlen(buf);
    size_t n;

    if(num_of_bytes_remaining < buf_len)
    { char * temp =
             realloc(result,
                     (++mult) * OAKPARK_BUF_SIZE + 1);

      if(temp)
      { result = temp;
        }
      else
      { free(result);
        return NULL;
        }
      curr = result + num_of_bytes_read;
      num_of_bytes_remaining += OAKPARK_BUF_SIZE;
      }
    assert(num_of_bytes_remaining >= buf_len);
    strcpy(curr, buf);
    num_of_bytes_remaining -= buf_len;
    curr += buf_len;
    /* check for overflow */
    if((n = num_of_bytes_read + buf_len) < num_of_bytes_read)
    { /* yes, we have an overflow */
      free(result);
      return NULL;
      }
    num_of_bytes_read += buf_len;
    if(buf[buf_len - 1] == '\n')
    { break;
      }
    }
  if(len)
  { *len = num_of_bytes_read;
    }
  if(num_of_bytes_read == 0)
  { free(result);
    result = NULL;
    }
  return result;
  }

/*
function: oakpark_get_para
parameter: ins: input stream
           len: pointer to the object to store the length of
                of the line (can be NULL if not needed)
return: NULL: on failure
        pointer to string on success. This pointer should be
        free()'d later.
*/
char * oakpark_get_para(FILE * ins, size_t * len)
{ char buf[OAKPARK_BUF_SIZE + 1];
  char * result;
  char * curr;
  size_t num_of_bytes_read = 0;
  size_t num_of_bytes_remaining = OAKPARK_BUF_SIZE;
  size_t mult = 1;

  assert(ins != NULL);
  result = malloc(OAKPARK_BUF_SIZE + 1);
  if(!result)
  { return NULL;
    }
  curr = result;
  while(fgets(buf, sizeof buf, ins) && buf[0] != '\n')
  { size_t buf_len = strlen(buf);
    size_t n;

    /* enough space? */
    if(num_of_bytes_remaining < buf_len)
    { char * temp = result;

      temp = realloc(result, ++mult * OAKPARK_BUF_SIZE + 1);
      if(temp)
      { result = temp;
        }
      else
      { free(result);
        return NULL;
        }
      curr = result + num_of_bytes_read;
      num_of_bytes_remaining += buf_len;
      }
    assert(num_of_bytes_remaining >= buf_len);
    strcpy(curr, buf);
    num_of_bytes_remaining -= buf_len;
    curr += buf_len;
    /* check for overflow */
    if((n = num_of_bytes_read + buf_len) < num_of_bytes_read)
    { /* yes, we have an overflow */
      free(result);
      return NULL;
      }
    num_of_bytes_read += buf_len;
    }
  if(len)
  { *len = num_of_bytes_read;
    }
  if(num_of_bytes_read == 0)
  { free(result);
    result = NULL;
    }
  return result;
  }

/*
function: oakpark_trim_end
parameter: s: string to trim
return: pointer to the end of the new string
change from 0.2a:
cast to unsigned char added for isspace() argument
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_trim_end(char * s)
{ size_t s_len;
  char * p; 

  if(!s)
  { return NULL;
    }
  s_len = strlen(s);
  p = &s[s_len - 1];
  while((p - s) && isspace((unsigned char)*p))
  { p--;
    }
  ++p;
  *p = '\0';
  return p;
  }

/*
function: oakpark_trim_start
parameter: s: string to trim
return: pointer to start of new string
change from 0.2a:
cast to unsigned char added for isspace() argument
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_trim_start(char * s)
{ if(!s)
  { return NULL;
    }
  while(*s && isspace((unsigned char)*s))
  { s++;
    }
  return s;
  }

/*
function: oakpark_get_ascii_val
parameter: c: character
return: on success: the ASCII value
        on failure: INT_MIN
*/
int oakpark_get_ascii_val(char c)
{ /* ASCII predefines 128 characters */
  char ascii_charset[128 + 1] =
       "\0\x01\x02\x03\x04\x05\x06\a\b\t\n\x0b\f\r\x0e\x0f"
       "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19"
       "\x1a\x1b\x1c\x1d\x1e\x1f"
       " !\"#$%&'()*+,-./"
       "0123456789"
       ":;<=>?@"
       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
       "[\\]^_`"
       "abcdefghijklmnopqrstuvwxyz"
       "{|}~\x7f";
  int ascii_val = 0;

  if(c != '\0')
  { char * pos;

    /* must add "+ 1" because ascii_charset[0] == '\0' */
    pos = strchr(ascii_charset + 1, c);
    if(pos)
    { ascii_val = pos - ascii_charset;
      /* must add "- 1" to discard the ending '\0' */
      assert(ascii_val < (sizeof ascii_charset - 1));
      }
    else
    /* if the character can't be found in the charset, it's not
       ASCII */
    { ascii_val = INT_MIN;
      }
    }
  return ascii_val;
  }

/*
function: oakpark_tokenize_str
parameter: s: string to tokenize, must be modifiable
           delim_str: delimiter string
return: pointer to the next token, NULL if no more token is
        found
notes:
Unlike the library function strtok(), this function uses the
whole delim_str as the delimiter. Also unlike strtok(), this
function doesn't accept NULL as s. To tokenize the next token,
use the return value from the previous invocation of this
function.
change from 0.1:
s may be NULL. If it is, NULL is returned.
delim_str may be NULL. If it is, s is returned.
*/
char * oakpark_tokenize_str(char * s, char * delim_str)
{ char * next;
  size_t delim_str_len;

  if(!s)
  { return NULL;
    }
  if(!delim_str)
  { return s;
    }
  delim_str_len = strlen(delim_str);
  /* find delimiter */
  next = strstr(s, delim_str);
  if(next)
  { *next = '\0';
    /* get the next token */
    next += delim_str_len;
    }
  return next;
  }

/*
An ISO C string hashing function.
Modified by Iman S. H. Suyoto from code by Justin Zobel,
available at:

http://www.cs.rmit.edu.au/~hugh/software/zwh-ipl/bitwisehash.c

function: oakpark_hash_str
parameters: s: string to be hashed
            u_bound: the number of slots - 1 (e.g. if the
                     intended number of slots is 8, then
                     u_bound is 7)
            seed: seed value
return: size_t: hash value 
change from 0.1:
assertion (s != NULL) is added.
*/
size_t oakpark_hash_str
       (char * s, size_t u_bound, unsigned long seed)
{ char c;
  unsigned long h = seed;

  assert(s != NULL);
  for(; (c = *s) != '\0'; s++)
  { h ^= (h << 5) + c + (h >> 2);
    }
  if(u_bound == (size_t)-1)
  { return h & (size_t)-1;
    }
  else
  { return h % (u_bound + 1UL);
    }
  }

/*
nonpublic function: case_conv_str
parameter: s: string to be converted, must be modifiable
           conv_func: character conversion function
return: pointer to the string
*/
static char * case_conv_str(char * s, int (*conv_func)(int))
{ char * start = s;

  if(!s)
  { return NULL;
    }
  for(; *s != '\0'; s++)
  { *s = conv_func((unsigned char)*s);
    }
  return start;
  }

/*
function: oakpark_lowerize_str
parameter: s: string to be converted, must be modifiable
return: pointer to the string
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_lowerize_str(char * s)
{ return case_conv_str(s, tolower);
  }

/*
function: oakpark_upperize_str
parameter: s: string to be converted, must be modifiable
return: pointer to the string
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_upperize_str(char * s)
{ return case_conv_str(s, toupper);
  }

/*
nonpublic function: no_case_conv
parameter: ch: character
return: the character itself
note:
This function is written to be passed as a parameter to
dup_str().
references: dup_str(), oakpark_dup_str().
*/
static int no_case_conv(int ch)
{ return ch;
  }

/*
nonpublic function: dup_str
parameter: s: string to be duplicated
           conv_func: character conversion function
return: NULL on failure
        pointer to the duplicate of s with conv_func applied on
        each character on success. This pointer should be
        free()'d later.
*/
static char * dup_str(char * s, int (*conv_func)(int))
{ size_t len;
  char * dup;

  if(!s)
  { return NULL;
    }
  len = strlen(s);
  dup = malloc(len + 1);
  if(dup)
  { char * d = dup;

    while(*s != '\0')
    { *d++ = conv_func((unsigned char)*s++);
      }
    *d = '\0';
    }
  return dup;
  }

/*
function: oakpark_dup_str
parameter: s: string to be duplicated
return: NULL on failure
        pointer to the duplicate of s on success. This pointer
        should be free()'d later.
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_dup_str(char * s)
{ return dup_str(s, no_case_conv);
  }

/*
function: oakpark_lowerize_dup_str
parameter: s: string to be converted
return: NULL on failure
        pointer to the lower-cased duplicate string on success.
        This pointer should be free()'d later.
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_lowerize_dup_str(char * s)
{ return dup_str(s, tolower);
  }

/*
function: oakpark_upperize_dup_str
parameter: s: string to be converted
return: NULL on failure
        pointer to the upper-cased duplicate string on success.
        This pointer should be free()'d later.
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_upperize_dup_str(char * s)
{ return dup_str(s, toupper);
  }

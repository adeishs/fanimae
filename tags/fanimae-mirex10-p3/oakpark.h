/*
oakpark
version 0.2
(header file)

by:
Iman S. H. Suyoto

Filename: oakpark.h.
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

#ifndef H__OAKPARK_
#define H__OAKPARK_

#include <stdlib.h>
#include <stdio.h>

/*
function: oakpark_get_line
parameter: ins: input stream
           len: pointer to the object to store the length of
                of the line (can be NULL if not needed)
return: NULL: on failure
        pointer to the string on success. This pointer should
        be free()'d later.
*/
char * oakpark_get_line(FILE * ins, size_t * len);

/*
function: oakpark_get_para 
parameter: ins: input stream
           len: pointer to the object to store the length of
                of the line (can be NULL if not needed)
return: NULL: on failure
        pointer to the string on success. This pointer should
        be free()'d later.
*/
char * oakpark_get_para(FILE * ins, size_t * len);

/*
function: oakpark_trim_end
parameter: s: string to trim
return: pointer to the end of the new string
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_trim_end(char * s);

/*
function: oakpark_trim_start
parameter: s: string to trim
return: pointer to start of new string
change from 0.1:
s may be NULL. If it is, NULL is returned.
*/
char * oakpark_trim_start(char * s);

/*
function: oakpark_get_ascii_val
parameter: c: character
return: on success: the ASCII value
        on failure: INT_MIN
*/
int oakpark_get_ascii_val(char c);

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
change from 0.1:
assertion (s != NULL) is added.
*/
char * oakpark_tokenize_str(char * s, char * delim_str);

/*
function: oakpark_hash_str
parameters: s: string to be hashed
            u_bound: the number of slots - 1 (e.g. if the
                     intended number of slots is 8, then
                     u_bound is 7)
            seed: seed value
return: size_t: hash value 
*/
size_t oakpark_hash_str
       (char * s, size_t u_bound, unsigned long seed);

/*
function: oakpark_lowerize_str
parameter: s: string to be converted, must be modifiable
return: pointer to the string
*/
char * oakpark_lowerize_str(char * s);

/*
function: oakpark_upperize_str
parameter: s: string to be converted, must be modifiable
return: pointer to the string
*/
char * oakpark_upperize_str(char * s);

/*
function: oakpark_dup_str
parameter: s: string to be duplicated
return: NULL on failure
        pointer to duplicate string on success. This pointer
        should be free()'d later.
*/
char * oakpark_dup_str(char * s);

/*
function: oakpark_lowerize_dup_str
parameter: s: string to be converted
return: NULL on failure
        pointer to lower-cased duplicate string on success.
        This pointer should be free()'d later.
*/
char * oakpark_lowerize_dup_str(char * s);

/*
function: oakpark_upperize_dup_str
parameter: s: string to be converted
return: NULL on failure
        pointer to upper-cased duplicate string on success.
        This pointer should be free()'d later.
*/
char * oakpark_upperize_dup_str(char * s);

#endif

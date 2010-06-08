RMIT MIRT
Fanimae MIREX 2005 Edition

Developer:
Iman S. H. Suyoto

based on research by:
Alexandra L. Uitdenbogerd
Justin Zobel

http://mirt.cs.rmit.edu.au/fanimae/

* OVERVIEW *
There are two main programs in Fanimae MIREX 2005 Edition,
fnmib and fnms. Prior to searching, the collection must be
indexed first. This is the task that fnmib does. The generated
index is broken into three files: .filp, .fipp, and .fdl. fnms
makes use of this index to search for similar melodies. A
utility fnmmp can be used to convert a set of MIDI files into a
file of directed modulo-12 sequences.

* INSTALLATION *
fnmib is written in C and should be able to be compiled by any
ISO C-compliant (to the 1989 standard) compiler. Three
Makefiles have been provided: Makefile.gnu is to be used in
conjunction with GNU Make and GCC; Makefile.suncc is to be
used under the Solaris environment with Sun C; and if you
don't have any of them, you may try Makefile.def. If it
doesn't work, you should refer to your compiler documentation.
Note that we have only tested this thoroughly under Linux
(x86). fnmib requires oakpark (included in the distribution).
oakpark (see http://oakpark.sourceforge.net) is not part of
the RMIT MIRT Project.

fnms is written in Perl and has been tested with perl version
5.8.3. Similarly for fnmmp. fnmmp requires the MIDI package
from CPAN.

You may wish to adjust the path to perl in the she-bang line in
fnmmp.pl and fnms.pl.

* HOW TO USE *
1. Run fnmmp.pl to convert a directory containing a collection
of MIDI files to a Fanimae sequence file, e.g.

% ./fnmmp.pl /directory/containing/collection my-seq

my-seq will be produced in your current working directory.

2. Run fnmmp.pl to convert a directory containing a query set
of MIDI files to a Fanimae sequence file, e.g.

% ./fnmmp.pl /directory/containing/queries my-query q

Note the ending "q". It's mandatory if you wish to generate a
query sequence.

3. Run fnmib to generate an index of sequences, e.g.

% ./fnmib my-idx my-seq

The following files will be produced: my-idx.fdl, my-idx.filp,
and my-idx.fipp. If you wish to rename the index, you must
rename all of them, e.g. if you wish to rename my-idx to
your-idx, you should rename my-idx.fdl, my-idx.filp, and
my-idx.fipp to your-idx.fdp, your-idx.filp, and your-idx.fipp
respectively.

4. Run fnms.pl to start searching. fnms.pl expects queries to
be fed from the standard input, e.g.

% ./fnms.pl my-idx < my-seq > my-answers

my-answers contain the answers to queries in my-seq.

* LICENSE *
See LICENSE.

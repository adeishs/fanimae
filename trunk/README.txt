Fanimae MIREX 2010 Edition

http://fanimae.sourceforge.net/

S1. Overview

Fanimae MIREX 2010 Edition is a desktop music information
retrieval system. In this version, two algorithms are
implemented: ngr5 and pioi.

ngr5 is the 5-gram coordinate matching implemented in Fanimae
MIREX 2005 Edition. This is based on Uitdenbogerd and Zobel
(2002).

pioi is the dynamic programming matching described in Section
7 of Suyoto and Uitdenbogerd (2008).

Like Fanimae MIREX 2005 Edition, this version also ships with
a MIDI parser to convert MIDI files into sequence files to
facilitate searching.


S2. Installation

fnmib and fnmspioi are written in C and should be able to
be compiled by any ISO C-compliant (to the 1990 standard)
compiler. Consult your C implementation documentation on how
to build the programs. If you are using GCC and GNU Make,
you can use Makefile.gnu.

Both fnmib and fnmspioi require oakpark (included in the
distribution).  oakpark (see http://oakpark.sourceforge.net)
is not part of the RMIT MIRT Project.

fnms2 and fnmmp are written in Perl and have been tested
with perl version 5.10.1. fnmmp requires the MIDI package
from CPAN.

Depending on your environment, you may wish to adjust the
path to perl in the she-bang lines in the Perl scripts.


S3. How to use

NOTE: if you want to use Fanimae in a fully MIREX-compliant
way, follow the instructions in Section 4 instead.

1. Run fnmmp.pl to convert a directory containing a collection
of MIDI files to a Fanimae sequence file, e.g.

% ./fnmmp.pl /directory/containing/collection my-seq

my-seq will be produced in your current working directory.

2. Run fnmmp.pl to convert a directory containing a query set
of MIDI files to a Fanimae sequence file, e.g.

% ./fnmmp.pl /directory/containing/queries my-query

3. [This is only mandatory if you want to search using
the ngr5 algorithm.] Run fnmib to generate an index of
sequences, e.g.

% ./fnmib my-idx my-seq

The following files will be produced: my-idx.fdl, my-idx.filp,
and my-idx.fipp. If you wish to rename the index, you must
rename all of them, e.g. if you wish to rename my-idx to
your-idx, you should rename my-idx.fdl, my-idx.filp, and
my-idx.fipp to your-idx.fdp, your-idx.filp, and your-idx.fipp
respectively.

4. To search using the ngr5 algorithm, perform 4a. To search
using the pioi algorithm, perform 4b.

4a. Run fnms2.pl to search. fnms.pl expects queries to be
fed from the standard input, e.g.

% ./fnms.pl my-idx < my-query

The answers will be output to the standard output.

4b. Run fnmspioi to search. fnmspioi expectes queries to be
fed from the standard input, e.g.

% ./fnmspioi my-seq < my-query


S4. Producing MIREX-compliant results

NOTE: this has only been tested against the official MIREX 2010
Symbolic Melodic Similarity dataset.

Make sure that the following CPAN modules are installed:

Fcntl
File::Spec
File::Copy
File::Path
FileHandle
IO::File
IPC::Run
MIDI
MIDI::Simple

The fnmmirex.pl wrapper script has been written for the
purpose of MIREX 2010. Usage:

% ./fnmmirex.pl alg /path/to/coll/files/dir/ /path/to/query.mid

alg is either (without quotes) "ngr5" or "pioi".

The first invocation takes a bit longer than subsequent
invocations as the first one will also build the index and
sequence files and cache them for subsequent uses.

Usage examples:

./fnmmirex.pl ngr5 essen/midi/ query.mid

The output will be:

query.mid ans1.mid ans2.mid ans3.mid ... ans10.mid


S5. License

See LICENSE.txt.

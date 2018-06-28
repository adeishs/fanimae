# Fanimae MIREX 2010 Edition (patch 3)

## Overview

Fanimae MIREX 2010 Edition is a desktop music information
retrieval system. In this version, two algorithms are
implemented: `ngr5` and `pioi`. Refer to the [paper describing
the algorithms](http://music-ir.org/mirex/abstracts/2010/SU1.pdf).

`ngr5` is the 5-gram coordinate matching implemented in Fanimae
MIREX 2005 Edition. This is based on Uitdenbogerd and Zobel
(2002).

`pioi` is the dynamic programming matching described in §7 of
Suyoto and Uitdenbogerd (2008).

Like Fanimae MIREX 2005 Edition, this version also ships with
a MIDI parser to convert MIDI files into sequence files to
facilitate searching.

## Installation

`fnmib` and `fnmspioi` are written in C and should be able to
be compiled by any ISO C-compliant (to the 1990 standard)
compiler. Consult your C implementation documentation on how
to build the programs. If you are using GCC and GNU Make,
you can use `Makefile.gnu`.

Both `fnmib` and `fnmspioi` require oakpark (included in the
distribution). oakpark (see <https://github.com/adeishs/oakpark>)
is not part of the RMIT MIRT Project.

`fnmsngr5` and `fnmmp` are written in Perl and have been tested
with perl version 5.10.1. `fnmmp` requires the `MIDI` package
from CPAN.

Depending on your environment, you may wish to adjust the
path to perl in the she-bang line in the Perl scripts.

## How to use

**Note**: if you want to use Fanimae in a fully MIREX-compliant
way, follow the instructions in Section 4 instead.

1. Run `fnmmp.pl` to convert a directory containing a collection
   of MIDI files to a Fanimae sequence file, e.g.
   ```
   % ./fnmmp.pl /directory/containing/collection my-seq
   ```
   `my-seq` will be produced in your current working directory.
1. Run `fnmmp.pl` to convert a directory containing a query set
   of MIDI files to a Fanimae sequence file, e.g.
   ```
   % ./fnmmp.pl /directory/containing/queries my-query
   ```
1. [This is only mandatory if you want to search using
   the `ngr5` algorithm.] Run `fnmib` to generate an index of
   sequences, e.g.
   ```
   % ./fnmib my-idx my-seq
   ```
   The following files will be produced: `my-idx.fdl`, `my-idx.filp`,
   and `my-idx.fipp`. If you wish to rename the index, you must
   rename all of them, e.g. if you wish to rename `my-idx` to
   `your-idx`, you should rename `my-idx.fdl`, `my-idx.filp`, and
   `my-idx.fipp` to `your-idx.fdp`, `your-idx.filp`, and `your-idx.fipp`
   respectively.
1. To search:
   * Using the `ngr5` algorithm: Run `fnmsngr5.pl` to search. `fnms.pl`
     expects queries to be fed from the standard input, e.g.
     ```
     % ./fnmsngr5.pl my-idx < my-query
     ```
   * Using the `pioi` algorithm: Run `fnmspioi` to search. `fnmspioi`
     expects queries to be fed from the standard input, e.g.
     ```
     % ./fnmspioi my-seq < my-query
     ```

The answers will be output to the standard output.

## Producing MIREX-compliant results

**Note**: this has only been tested against the official MIREX 2010
Symbolic Melodic Similarity dataset.

Make sure that the following CPAN modules are installed:

* `Fcntl`
* `File::Spec`
* `File::Copy`
* `File::Path`
* `FileHandle`
* `IO::File`
* `IPC::Run`
* `MIDI`
* `MIDI::Simple`

The `fnmmirex.pl` wrapper script has been written for the
purpose of MIREX 2010. Usage:
```
% ./fnmmirex.pl alg /coll/files/dir/ query.mid
```
`alg` is either `ngr5` or `pioi`.

The first invocation takes a bit longer than subsequent
invocations as the first one will also build the index and
sequence files and cache them for subsequent uses.

Usage examples:
```
./fnmmirex.pl ngr5 essen/midi/ query.mid
```
The output will be:
```
query.mid ans1.mid ans2.mid ans3.mid ... ans10.mid
```
Separate indexing and searching invocations can be achieved if
needed.

To only index:
```
% ./fnmmirex.pl - /coll/files/dir/
```
To search, simply use the script by specifying the algorithm
as specified above, e.g.:
```
./fnmmirex.pl ngr5 essen/midi/ query.mid

./fnmmirex.pl pioi essen/midi/ query.mid
```

## License

The use of Fanimae is governed by the MIT Licence.

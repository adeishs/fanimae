#!/usr/bin/perl

#
# RMIT MIRT Project
# Fanimae Search MIREX 2005 Edition
#
# Copyright 2004--2005 by RMIT MIRT Project.
#
# Developer:
# Iman S. H. Suyoto
#
# Filename: fnms.pl
#

require IO::File;
require Fcntl;

my $NUM_OF_GRAMS = 5;
my $POS_SIZE = 4;
my $ARGC = @ARGV;
if($ARGC < 1)
{ show_usage();
  exit(1);
  }
# check command line arguments
my $idx_fn = $ARGV[0];
my $use_qid = ($ARGV[1] eq 'q');
my $ilp_fn = "$idx_fn.fipp";
my $il_fn = "$idx_fn.filp";
my $dl_fn = "$idx_fn.fdl";
sysopen ILP_FH, $ilp_fn, (O_RDONLY | O_BINARY) or
die "Can't open $ilp_fn\n";
sysopen IL_FH, $il_fn, (O_RDONLY | O_BINARY) or
die "Can't open $il_fn\n";
open DL_FH, "<$dl_fn" or
die "Can't open $dl_fn\n";
my @titles = <DL_FH>;
chomp(@titles);
close DL_FH;
my $symbols = "abcdefghijklmnopqrstuvwxy";
QUERY_PROMPT:
while(my $query = prompt())  # capture queries
{ my %H = ();
  my @ngrams = ();
  my $num_of_grams;
  if($use_qid)
  { my $qid;
    ($qid, $query) = split /\*\*\*/, $query;
    print "$qid";
    }
  # get n-grams
  for(my $ql = 0; $ql <= length($query) - $NUM_OF_GRAMS; ++$ql)
  { my $t = substr($query, $ql, $NUM_OF_GRAMS);
    push @ngrams, $t;
    }
  # remove duplicates
  @ngrams = sort @ngrams;
  $num_of_grams = @ngrams;
  for(my $n = 1; $n < $num_of_grams; ++$n)
  { if($ngrams[$n] eq $ngrams[$n - 1])
    { splice @ngrams, $n, 1;
      --$num_of_grams;
      }
    }
  # query
  for(my $ql = 0; $ql < @ngrams; ++$ql)
  { my $t = $ngrams[$ql];
    my $tune_num = 0;
    # check query validity
    for(my $n = 0; $n < $NUM_OF_GRAMS; ++$n)
    { my $pos = index($symbols, substr($t, $n, 1));
      if($pos < 0)
      { print "Erratic query at position $t\n";
        next QUERY_PROMPT;
        }
      $tune_num = 25 * $tune_num + $pos;
      }
    # fetch index entries
    seek ILP_FH, $tune_num * $POS_SIZE, SEEK_SET;
    my $pos_bytes = "";
    read ILP_FH, $pos_bytes, $POS_SIZE;
    my $pos = 0;
    for(my $pc = 0; $pc < $POS_SIZE; ++$pc)
    { $pos |= ord(substr($pos_bytes, $pc, 1)) <<
              ($pc << 3);  # it actually means $pc * 8
      }
    seek IL_FH, $pos, SEEK_SET;
    # grab tune ids
    my $nod = read_uint(*IL_FH, 0);
    for(my $dc = 0; $dc < $nod; $dc++)
    { $tune_num = read_uint(*IL_FH, 0);
      $H{$tune_num}++;
      }
    }
  # rank answers
  my @answers = sort { $H{$b} <=> $H{$a} } keys %H;
  # show only top 30 answers
  my $num_of_answers = (@answers > 30)? 30 : @answers;
  for(my $r = 0; $r < $num_of_answers; $r++)
  { print "\t$titles[$answers[$r]]";
    }
  print "\n";
  undef @ngrams;
  undef %H;
  undef @answers;
  }
close(IL_FH);
close(ILP_FH);

#
# sub: show_usage
# param: none
# return: none
# purpose show program usage
# author: ISHS
#
sub show_usage
{ print STDERR "Usage:\n" .
               "fnms.pl idx [q]\n\n" .
	       "Use \"q\" to include query-ID\n\n";
  }

#
# sub: prompt
# param: none
# return: query
# purpose: get a query from user
# author: ISHS
#
sub prompt
{ print STDERR "p>\n";
  my $line = <STDIN>;
  chomp($line);
  return $line;
  }

#
# sub: read_uint
# param: $fh: file handle
#        $nob: number of bytes;
#        0 for bitwise compressed integer
# purpose: read (and optionally decompress) an integer from $fh
# author: ISHS
#
sub read_uint
{ my ($fh, $nob) = @_;
  my $x = 0;
  if($nob)  # uncompressed integer of nob bytes
  { my $buf;
    my $r;

    if(read $fh, $buf, $nob)
    { my $b;

      for($b = 0; $b < $nob; $b++)
      { $x |= ord(substr($buf, $b, 1)) << (8 * $b);
        }
      return $x;
      }
    }
  else  # variable-nibble compressed integer
  { my $buf;
    my $is_finished = 0;
    my $b = 0;

    while(!$is_finished)
    { # pool next byte */
      if(read($fh, $buf, 1) == 1)
      { my $o = ord($buf);

        $x |= ($o & 0x07) << (6 * $b);  # ?... */
        if($o & 0x08)  # 1... */
        { # ?... 1... */
          $x |= (($o & 0x70) >> 4) << (6 * $b + 3);
          if($o & 0x80)  # 1... 1... */
          { ++$b;  # read next byte */
            }
          else  # 0... 1... */
          { $is_finished = 1;  # stop reading */
            }
          }
        else  # 0... */
        { $is_finished = 1;  # stop reading */
          }
        }
      else  # inconsistent data; a byte must exist */
      { return 0;  # return failure status */
        }
      }
    }
  return $x;
  }

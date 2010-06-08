#!/usr/bin/perl

#
# RMIT MIRT Project
# Fanimae MIDI Parser MIREX 2005 Edition
#
# Copyright 2004--2005 by RMIT MIRT Project.
#
# Developer:
# Iman S. H. Suyoto
# 
# Filename: fnmmp.pl.
#

use strict;
use MIDI;
use MIDI::Simple;
use FileHandle;

my $START_TICK = 1;
my $DURATION = 2;
my $PITCH = 4;
my $dir = $ARGV[0];
my $out_filename = $ARGV[1];
my $query_mode = ($ARGV[2] eq "q");
if($dir && $out_filename)
{ if(-d $dir)
  { my $out_fh = FileHandle->new();
    open($out_fh, ">$out_filename") or
    die "Failed creating handle: $!";
    process_dir($dir, $out_fh, $query_mode);
    close($out_fh);
    }
  }
else
{ print STDERR "RMIT MIRT Project\n" .
               "Fanimae MIREX 2005 Edition\n" .
               "MIDI Parser\n\n" .
               "Developer:\n" .
               "Iman S. H. Suyoto\n\n" .
               "based on research by:\n" .
               "Alexandra L. Uitdenbogerd\n" .
               "Justin Zobel\n\n" .
               "Usage: fnmmp.pl directory output [q]\n\n" .
               "Specify \"q\" if the output is intended to " .
               "be a set of query sequences\n\n";
  }

sub process_file
{ my ($filename, $short_filename, $out_fh, $query_mode) = @_;
  die "Filename not supplied: $!" unless $filename;
  die "Invalid filehandle: $!" unless $out_fh;
  eval
  { my $opus = MIDI::Opus->new({'from_file' => $filename});
    my $track_num = 0;
    foreach my $track($opus->tracks())
    { my $score_r =
      MIDI::Score::events_r_to_score_r($track->events_r);
      my $sorted_score_r =
      MIDI::Score::sort_score_r($score_r);
      my @notes = ();
      my @durs = ();
      my @prev = ();
      foreach my $notes_r(@$score_r)
      { my @n = @$notes_r;
        if(($n[0] eq "note") &&
           ($n[$DURATION] != -$n[$START_TICK]) &&
           ($n[$START_TICK] > 0))
        { if(@prev)
	  { # compare with previous note
            # if overlapping
            if($n[$START_TICK] == $prev[$START_TICK])
            { # take the highest note
              if($n[$PITCH] > $prev[$PITCH])
              { @prev = @n;
                }
              }
            else
            { # if partially overlapping (next note starts before
              # previous note stops)
              if($prev[$DURATION] >
                 $n[$START_TICK] - $prev[$START_TICK])
              { $prev[$DURATION] =
                $n[$START_TICK] - $prev[$START_TICK];
                $durs[$#durs] = $prev[$DURATION];
                }
              if($n[$DURATION] > 0)
              { push @notes, $n[$PITCH];
                push @durs, $n[$DURATION];
                }
              }
            }
          else
          { if($n[$DURATION] > 0)
            { push @notes, $n[$PITCH];
              push @durs, $n[$DURATION];
              }
            }
	  @prev = @n;
	  }
        }
      print $out_fh "$short_filename|$track_num***"
      unless($query_mode);
      print $out_fh directed_mod_12(@notes);
      print $out_fh "\n";
      $track_num++;
      }
    }
  }

sub directed_mod_12
{ my @notes = @_;
  my $c;
  my $result = "";
  my @symbols = ('a', 'b', 'c', 'd', 'e', 'f',
                 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                 'n', 'o', 'p', 'q', 'r', 's',
                 't', 'u', 'v', 'w', 'x', 'y');
  for($c = 0; $c < $#notes; $c++)
  { my $now = $notes[$c + 1];
    my $prev = $notes[$c];
    my $interval = $now - $prev;
    my $d = $now <=> $prev;
    $interval *= $d;
    $result .= $symbols[12 + $d * (1 + ($interval - 1) % 12)];
    }
  return $result;
  }

sub dur_ext_contour
{ my @durs = @_;
  my $c;
  my $result = "";
  for($c = 0; $c < $#durs; $c++)
  { my $now = $durs[$c + 1];
    my $prev = $durs[$c];
    my $ratio;
    if($prev > 0)
    { $ratio = $now / $prev;
      my $e =
         ($ratio <= 0.25)?
         'S' : ($ratio <= 0.5)?
               's' : ($ratio < 2)?
                     'R' : ($ratio < 4)?
                           'l' : 'L';
      $result .= $e;
      }
    }
  return $result;
  }

sub process_dir
{ my ($dir_name, $out_fh, $query_mode) = @_;
  die "$dir_name is not a directory: $!" unless -d $dir_name;
  die "Invalid filehandle: $!" unless $out_fh;
  opendir DH, $dir_name;
  my @filenames = readdir(DH);
  foreach my $filename(@filenames)
  { my $full_filename = "$dir_name/$filename";
    if(($filename ne ".") && ($filename ne "..") &&
       ($filename ne "TRANS.TBL"))
    { if(-d $full_filename)
      { process_dir($full_filename, $out_fh, $query_mode);
	}
      else
      { my $s = rindex $full_filename, "/";
        if($s >= 0)
        { ++$s;
          }
        my $short_filename = substr $full_filename, $s;
        print "$short_filename\n";
        process_file($full_filename, $short_filename, $out_fh,
                     $query_mode);
        }
      }
    }
  }

sub midi_num_to_name
{ my $midi_num = $_[0];
  my @pitch_names =
     ("C", "C#", "D", "D#", "E", "F",
      "F#", "G", "G#", "A", "A#", "B");
  my $pitch = $midi_num % 12;
  return ($pitch_names[$pitch] .
          (int($midi_num / 12) - 1));
  }

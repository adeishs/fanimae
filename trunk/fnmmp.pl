#!/usr/bin/perl

#
# RMIT MIRT Project
# Fanimae MIDI Parser MIREX 2005 Edition
#
# Copyright 2004--2006 by RMIT MIRT Project.
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
my $PITCH_IDX = 0;
my $ONSET_IDX = 1;
my $dir = $ARGV[0];
my $out_filename = $ARGV[1];
if ($dir && $out_filename) {
    if (-d $dir) {
        my $out_fh = FileHandle->new();
        open($out_fh, ">$out_filename") or
        die "Failed creating handle: $!";
        process_dir($dir, $out_fh);
        close($out_fh);
    }
} else {
    print STDERR "RMIT MIRT Project\n" .
                 "Fanimae MIREX 2005 Edition\n" .
                 "MIDI Parser\n\n" .
                 "Developer:\n" .
                 "Iman S. H. Suyoto\n\n" .
                 "based on research by:\n" .
                 "Iman S. H. Suyoto\n" .
                 "Alexandra L. Uitdenbogerd\n" .
                 "Justin Zobel\n\n" .
                 "Usage: fnmmp.pl directory output [q]\n\n" .
                 "Specify \"q\" if the output is intended to " .
                 "be a set of query sequences\n\n";
}

#
# sub: process_file
# parameter: $filename: MIDI filename (with full path)
#            $short_filename: shortened MIDI filename
#            $out_fh: output filehandle
#            $query_mode: whether the output is a query
# 
sub process_file {
    my ($filename, $short_filename, $out_fh, $query_mode) = @_;
    die "Filename not supplied: $!" unless $filename;
    die "Invalid filehandle: $!" unless $out_fh;
    eval {
        my $opus = MIDI::Opus->new({'from_file' => $filename});
        my @tracks = $opus->tracks();
        my $track_num = 0;
        foreach my $track(@tracks) {
            my $score_r =
            MIDI::Score::events_r_to_score_r($track->events_r);
            # a note is expressed as a tuple of
            # <pitch-num, onset-time>
            my @note_tuples = ();
            # collect pitches
            foreach my $n_r(@$score_r) {
                my @n = @$n_r;
                if (($n[0] eq "note") &&
                    ($n[$DURATION] != -$n[$START_TICK]) &&
                    ($n[$START_TICK] >= 0) &&
                    ($n[$DURATION] > 0)) {
                    my @n_tuple = ($n[$PITCH], $n[$START_TICK]);
                    my $n_tuple = \@n_tuple;
                    push @note_tuples, $n_tuple;
                }
            }
            # sort note
            @note_tuples = sort note_cmp @note_tuples;
            # get "melody line"
            my @pitches = ();
            my @curr = ();
            my @next = ();
            my $curr_r;
            my $next_r;
            for (my $n = 0; $n < $#note_tuples; ++$n) {
                $curr_r = $note_tuples[$n];
                @curr = @$curr_r;
                $next_r = $note_tuples[$n + 1];
                @next = @$next_r;
                # this ensures only the lowest pitch at a specific
                # onset time will be pushed
                if ($curr[$ONSET_IDX] != $next[$ONSET_IDX]) {
                    push @pitches, $curr[$PITCH_IDX];
                }
            }
            push @pitches, $next[$PITCH_IDX];
            # standardize the pitches
            print $out_fh "$short_filename|$track_num***"
            unless($query_mode);
            print $out_fh directed_mod_12(@pitches);
            print $out_fh "\n";
            $track_num++;
        }
    }
}

#
# sub: note_cmp
# a helper sub for sort
# sort ascending on onset-time, ascending on pitch-num
#
sub note_cmp {
    my @note_a = @$a;
    my @note_b = @$b;

    if ($note_a[$ONSET_IDX] == $note_b[$ONSET_IDX]) {
        return($note_a[$PITCH_IDX] <=> $note_b[$PITCH_IDX]);
    } else {
        return($note_a[$ONSET_IDX] <=> $note_b[$ONSET_IDX]);
    }
}

#
# sub: directed_mod_12
# parameter: @pitches: melody
# return: standardized melody
#
sub directed_mod_12 {
    my @pitches = @_;
    my $c;
    my $result = "";
    my @symbols = ('a', 'b', 'c', 'd', 'e', 'f',
                   'g', 'h', 'i', 'j', 'k', 'l', 'm',
                   'n', 'o', 'p', 'q', 'r', 's',
                   't', 'u', 'v', 'w', 'x', 'y');

    for ($c = 0; $c < $#pitches; $c++) {
        my $now = $pitches[$c + 1];
        my $prev = $pitches[$c];
        my $interval = $now - $prev;
        my $d = $now <=> $prev;
        $interval *= $d;
        $result .= $symbols[12 + $d * (1 + ($interval - 1) % 12)];
    }
    return $result;
}

#
# sub: process_dir
# parameter: $dir_name: directory name containing MIDI files
#            $out_fh: output filehandle
#            $query_mode: whether the output is a query
# 
sub process_dir {
    my ($dir_name, $out_fh, $query_mode) = @_;
    die "$dir_name is not a directory: $!" unless -d $dir_name;
    die "Invalid filehandle: $!" unless $out_fh;
    opendir DH, $dir_name;
    my @filenames = readdir(DH);
    foreach my $filename(@filenames) {
        my $full_filename = "$dir_name/$filename";
        if (($filename ne ".") && ($filename ne "..") &&
            ($filename ne "TRANS.TBL")) {
            if (-d $full_filename) {
                process_dir($full_filename, $out_fh, $query_mode);
            } else {
                my $s = rindex $full_filename, "/";
                if ($s >= 0)
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

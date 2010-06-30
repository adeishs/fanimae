#!/usr/bin/perl

# $Id$
#
# Fanimae MIREX 2010 Edition
# MIDI Parser
#
# Copyright (C) 2004--2010 by RMIT MIRT Project.
# Copyright (C) 2010 by Iman S. H. Suyoto
#
# Filename: fnmmp.pl.
#

use strict;
use MIDI;
use MIDI::Simple;
use FileHandle;

my $START_TICK = 1;
my $DURATION = 2;
my $CHANNEL = 3;
my $PITCH = 4;
my $PITCH_IDX = 0;
my $ONSET_IDX = 1;
my $LN_2 = log(2);
my $dir = $ARGV[0];
my $out_filename = $ARGV[1];
if ($dir && $out_filename) {
    if (-d $dir) {
        my $out_fh = FileHandle->new();
        open($out_fh, ">$out_filename") or
        die "Failed creating handle: $!\n";
        process_dir($dir, $out_fh);
        close($out_fh);
    }
} else {
    print STDERR <<EOT;
Fanimae MIREX 2010 Edition
MIDI Parser

Usage: fnmmp.pl directory output
EOT
    exit 1;
}

#
# sub: process_file
# parameter: $filename: MIDI filename (with full path)
#            $short_filename: shortened MIDI filename
#            $out_fh: output filehandle
# 
sub process_file {
    my ($filename, $short_filename, $out_fh) = @_;
    die "Filename not supplied: $!\n" unless $filename;
    die "Invalid filehandle: $!\n" unless $out_fh;
    eval {
        my $opus = MIDI::Opus->new({'from_file' => $filename});
        my @tracks = $opus->tracks();
        my $track_num = 0;
        foreach my $track(@tracks) {
            my $score_r =
               MIDI::Score::events_r_to_score_r($track->events_r);
            my $sorted_score_r =
               MIDI::Score::sort_score_r($score_r);
            my @pitches = ();
            my @iois = ();
            my @prev = ();

            # collect pitches
            foreach my $n_r(@$score_r) {
                my @n = @$n_r;
                if (($n[$CHANNEL] != 9) &&
                    ($n[0] eq "note") &&
                    ($n[$DURATION] != -$n[$START_TICK]) &&
                    ($n[$START_TICK] >= 0)) {

                    if (@prev) {
                        # compare with previous note
                        if ($n[$START_TICK] == $prev[$START_TICK]) {
                            # if overlapping
                            # take the highest note
                            if ($n[$PITCH] > $prev[$PITCH]) {
                                @prev = @n;
                            }
                        } else {
                            $prev[$DURATION] =
                            $n[$START_TICK] -
                            $prev[$START_TICK];
                            $iois[$#iois] = $prev[$DURATION];

                            if ($n[$DURATION] > 0) {
                                push @pitches, $n[$PITCH];
                                push @iois, $n[$DURATION];
                            }
                        }
                    } else {
                        if ($n[$DURATION] > 0) {
                            push @pitches, $n[$PITCH];
                            push @iois, $n[$DURATION];
                        }
                    }
                    @prev = @n;
                }
            }
            # standardize the pitches
            print $out_fh "pi:$short_filename|$track_num***";
            print $out_fh directed_mod_12(@pitches);
            # standardize the IOIs 
            print $out_fh '***';
            print $out_fh ioi_ext_contour(@iois);
            print $out_fh "\n";
            $track_num++;
        }
    }
}

#
# sub: directed_mod_12
# parameter: @pitches: pitches
# return: standardized pitches
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
# sub: ioi_ext_contour
# parameter: @durs: durations
# return: standardized melody
#
sub ioi_ext_contour {
    my @iois = @_;
    my $c;
    my $result = "";
    for ($c = 0; $c < $#iois; $c++) {
        my $now = $iois[$c + 1];
        my $prev = $iois[$c];
        my $ratio;
        if ($prev > 0) {
            $ratio = log($now / $prev) / $LN_2;
            my $e = (abs($ratio) < 1) ? 'R' :
                    (1 <= $ratio && $ratio < 2) ? 'l' :
                    (2 <= $ratio) ? 'L' :
                    (-2 < $ratio && $ratio <= -1) ? 's' : 'S';
            $result .= $e;
        } else {
            $result .= "S";
        }
    }
    return $result;
}

#
# sub: process_dir
# parameter: $dir_name: directory name containing MIDI files
#            $out_fh: output filehandle
# 
sub process_dir {
    my ($dir_name, $out_fh) = @_;
    die "$dir_name is not a directory: $!\n" unless -d $dir_name;
    die "Invalid filehandle: $!\n" unless $out_fh;
    opendir DH, $dir_name;
    my @filenames = readdir(DH);
    foreach my $filename(@filenames) {
        my $full_filename = "$dir_name/$filename";
        if (($filename ne ".") && ($filename ne "..") &&
            ($filename ne "TRANS.TBL")) {
            if (-d $full_filename) {
                process_dir($full_filename, $out_fh);
            } else {
                my $s = rindex $full_filename, "/";
                if ($s >= 0) {
                    ++$s;
                }
                my $short_filename = substr $full_filename, $s;
                print STDERR "$short_filename\n";
                process_file($full_filename, $short_filename,
                             $out_fh);
            }
        }
    }
}

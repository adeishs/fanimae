#!/usr/bin/perl

# $Id$

use strict;

my $EXPECTED_ARGC = 2;
my $FNM_DIR = ".fnm";
my $MAX_NUM_OF_ANSWERS = 10;

main();

sub query($$) {
    my $query_fn = shift;
    my @answers = ();

    # parse query and generate sequence file

    # query

    # construct answer

    return @answers;
}

sub create_index($) {
    my $coll_dir = shift;
    my $index_dir = "$coll_dir/$FNM_DIR";
    my @cmd;
    my $result;

    # create index dir if not existing
    unless (-d $index_dir) {
        mkdir $index_dir or die "Can't create $index_dir\n";
    }

    # parse collection files and generate sequence file
    my $temp_seq_fn = "$index_dir/.fnmmp.seq.$$";
    @cmd = ('./fnmmp.pl', $coll_dir, $temp_seq_fn);
    print "@cmd\n";
    $result = system @cmd;

    if ($result != 0 || !(-f $temp_seq_fn)) {
        return 0;
    }
    # index the sequences in the sequence file
    my $index_name = "$index_dir/index";
    @cmd = ('./fnmib', $index_name, $temp_seq_fn);
    $result = system @cmd;

    if ($result != 0 ||
        !(-f "$index_name.fdl") ||
        !(-f "$index_name.filp") ||
        !(-f "$index_name.fipp")) {
        return 0;
    }
    unlink $temp_seq_fn;

    return 1;
}

sub index_found($) {
    my $coll_dir = shift;

    unless (-d "$coll_dir/$FNM_DIR") {
        return 0;
    }
    return 1;
}

sub print_usage {
    print <<EOT;
fnmmirex
Fanimae wrapper for MIREX 2010

Usage:
fnmmirex.pl /path/to/coll/files/dir/ /path/to/query.mid

The collection files must be in MIDI format.
EOT

    exit 1;
}

sub main {
    if (@ARGV != $EXPECTED_ARGC) {
        print_usage();
        exit 1;
    }

    my $coll_dir = $ARGV[0];
    my $query_fn = $ARGV[1];

    # validate parameters
    unless (-d $coll_dir) {
        die "Directory not found: $coll_dir";
    }
    unless (-e $query_fn) {
        die "File not found: $query_fn";
    }

    if (!index_found($coll_dir)) {
        create_index($coll_dir);
    }

    my @answers = query($query_fn, $coll_dir);

    # output answer according to MIREX requirement
    print $query_fn;
    if (defined @answers) {
        print ' ';
        print join(@answers, ' ');
    }
    print "\n";
}

#!/usr/bin/perl

# $Id$

use strict;
use File::Spec;
use File::Copy;
use File::Path;
use IPC::Run qw(run);

my $EXPECTED_ARGC = 3;
my $FNM_DIR = '.fnm';
my $SEQUENCE_FN = 'sequence';
my $INDEX_FN = 'index';
my $MAX_NUM_OF_ANSWERS = 10;
my $CURR_DIR = File::Spec->curdir();
my $FNMMP_PATH = File::Spec->catfile($CURR_DIR, 'fnmmp.pl');

main();

sub query($$$) {
    my $algo = shift;
    my $query_fn = shift;
    my $coll_dir = shift;
    my $index_dir = File::Spec->catdir($coll_dir, $FNM_DIR);
    my $tmp_dir = File::Spec->catdir($index_dir, "query");
    my $answer;
    my @cmd;
    my $result;

    # copy the query MIDI file to Fanimae working directory
    # to prevent all the files in the directory containing the
    # MIDI file to be used as queries
    my $query_vol;
    my $query_dir;
    my $query_base_fn;

    ($query_vol,
     $query_dir,
     $query_base_fn) = File::Spec->splitpath($query_fn);

    my $query_path = File::Spec->catpath($query_vol,
                                         $query_dir, '');
    my $tmp_query_fn = File::Spec->catfile($tmp_dir,
                                           $query_base_fn);

    unless (-d $tmp_dir) {
        mkdir $tmp_dir or die "Can't create $tmp_dir\n";
    }
    copy($query_fn, $tmp_query_fn)
    or die "Copying $query_fn to $tmp_query_fn failed";

    # parse query and generate sequence file
    my $temp_seq_fn = ".fnmmp.qryseq.$$";
    my $output;

    @cmd = ($FNMMP_PATH, $query_path, $temp_seq_fn);
    $result = run \@cmd, undef, \$output;

    if (!$result || !(-f $temp_seq_fn)) {
        return undef;
    }

    # query
    my $n = 0;

    if ($algo eq 'ngr5') {
        @cmd = (File::Spec->catfile($CURR_DIR, 'fnms2.pl'),
                File::Spec->catfile($index_dir, $INDEX_FN),
                'q');
    } elsif ($algo eq 'pioi') {
        @cmd = (File::Spec->catfile($CURR_DIR, 'fnmspioi'),
                File::Spec->catfile($index_dir, $SEQUENCE_FN),
                'q');
    } else {
        die "Invalid algo\n";
    }
    open QFH, "<$temp_seq_fn"
    or die "Can't open $temp_seq_fn\n";
    $result = run \@cmd, \*QFH, \$answer;
    close QFH;

    if (!$result) {
        return undef;
    }
    chomp($answer);

    File::Path->remove_tree($tmp_dir);
    unlink $temp_seq_fn;
    return $answer;
}

sub create_index($) {
    my $coll_dir = shift;
    my $index_dir = File::Spec->catdir($coll_dir, $FNM_DIR);
    my @cmd;
    my $result;
    my $output;

    # create index dir if not existing
    unless (-d $index_dir) {
        mkdir $index_dir or die "Can't create $index_dir\n";
    }

    # parse collection files and generate sequence file
    my $temp_seq_fn = File::Spec->catfile($index_dir,
                                          $SEQUENCE_FN);
    @cmd = ($FNMMP_PATH, $coll_dir, $temp_seq_fn);
    $result = run \@cmd, undef, \$output;

    if (!$result || !(-f $temp_seq_fn)) {
        return 0;
    }

    # index the sequences in the sequence file
    my $index_name =
       File::Spec->catfile($index_dir, $INDEX_FN);
    @cmd = (File::Spec->catfile($CURR_DIR, 'fnmib'),
            $index_name, $temp_seq_fn);
    $result = run \@cmd, undef, \$output;

    if (!$result ||
        !(-f "$index_name.fdl") ||
        !(-f "$index_name.filp") ||
        !(-f "$index_name.fipp")) {
        return 0;
    }

    return 1;
}

sub index_up_to_date($) {
    my $coll_dir = shift;
    my $index_dir = File::Spec->catdir($coll_dir, $FNM_DIR);
    my $coll_dir_modif_time = (stat($coll_dir))[9];
    my $index_dir_modif_time = (stat($index_dir))[9];

    unless (-d $index_dir) {
        return 0;
    }
    if ($coll_dir_modif_time > $index_dir_modif_time) {
        return 0;
    }
    return 1;
}

sub print_usage {
    print <<EOT;
Fanimae wrapper for MIREX 2010

Usage:
fnmmirex.pl algo /path/to/coll/files/dir/ /path/to/query.mid

algo is either ngr5 or pioi

The collection files must be in MIDI format.
EOT

    exit 1;
}

sub main {
    if (@ARGV != $EXPECTED_ARGC) {
        print_usage();
        exit 1;
    }
    my $arg = 0;

    my $algo = $ARGV[$arg++];
    my $coll_dir = $ARGV[$arg++];
    my $query_fn = $ARGV[$arg++];

    # validate parameters
    for ($algo) {
        /^(ngr5|pioi)$/ && do {
            last;
        };
        die "Invalid algo.\n";
    }

    unless (-d $coll_dir) {
        die "Directory not found: $coll_dir\n";
    }
    unless (-e $query_fn) {
        die "File not found: $query_fn\n";
    }

    if (!index_up_to_date($coll_dir)) {
        create_index($coll_dir);
    }

    my $answers = query($algo, $query_fn, $coll_dir);

    # output answer according to MIREX requirement
    if (defined $answers) {
        my @a = split / /, $answers;
        @a = map {
            (my $s = $_) =~ s/\|0//;
            $s;
        } @a;
        print join(' ', @a);
    }
    print "\n";
}

#!/usr/bin/perl -w
#
# get_composite.pl
#
# This file is released under the terms of the Artistic License.  Please see
# the file LICENSE, included in this package, for details.
#
# Copyright (C) 2003 Open Source Development Lab, Inc.
#
#
use strict;
use English;
use Getopt::Long;
use Pod::Usage;
use FileHandle;
use Env qw(DBT3_INSTALL_PATH);

=head1 NAME

get_composite.pl

=head1 SYNOPSIS

call get_power.pl and get_throughput.pl and calculate composite metrics

=head1 ARGUMENTS

 -perf_run_number <perf_run_number>
 -scale_factor <scale factor >
 -num_of_streams < number of streams>
 -outfile < output file name >
 -file <config filename to read from> 
 -write <config filename to write to> 

=cut

my ( $perf_run_number, $num_of_streams, $hlp, $scale_factor, $configfile, 
	$writeme, $dbdriver_sapdb_path, $power, $throughput, $composite,
	$outfile );

GetOptions(
	"perf_run_number=i" => \$perf_run_number,
	"scale_factor=i" => \$scale_factor,
	"num_of_streams=i" => \$num_of_streams,
	"outfile=s" => \$outfile,
	"help"      => \$hlp,
	"file=s"    => \$configfile,
	"write=s"   => \$writeme
);


my $fcf = new FileHandle;
my ( $cline, %options );

if ($hlp) { pod2usage(1); }

if ( $configfile ) {
	unless ( $fcf->open( "< $configfile" ) ) {
		die "Missing config file $!";
	}
	while ( $cline = $fcf->getline ) {
		next if ( $cline =~ /^#/ );
		chomp $cline;
	my ( $var, $value ) = split /=/, $cline;
		$options{ $var } = $value;
	}
	$fcf->close;
}

if ( $perf_run_number ) { $options{ 'perf_run_number' } = $perf_run_number; }
elsif ( $options{ 'perf_run_number' } ) {
        $perf_run_number =  $options{ 'perf_run_number' };
}
else
{
        die "No perf_run_number $!";
}

if ( $scale_factor ) { $options{ 'scale_factor' } = $scale_factor; }
elsif ( $options{ 'scale_factor' } ) {
        $scale_factor =  $options{ 'scale_factor' };
}
else
{
        die "No scale_factor $!";
}

if ( $num_of_streams ) { $options{ 'num_of_streams' } = $num_of_streams; }
elsif ( $options{ 'num_of_streams' } ) {
        $num_of_streams =  $options{ 'num_of_streams' };
}
else
{
        die "No num_of_streams $!";
}

if ( $outfile ) { $options{ 'outfile' } = $outfile; }
elsif ( $options{ 'outfile' } ) {
        $outfile =  $options{ 'outfile' };
}
else
{
        die "No output file name $!";
}

if ( $writeme ) {
	my $ncf = new FileHandle;
	unless ( $ncf->open( "> $writeme" ) ) { die "can't open file $!"; }
	my $name;
	foreach $name ( keys( %options ) ) {
	    print $ncf $name, "=", $options{ $name }, "\n";
	}
	$ncf->close;
}

$dbdriver_sapdb_path = "$DBT3_INSTALL_PATH/dbdriver/scripts/sapdb";

print "calculate power\n";
system("$dbdriver_sapdb_path/get_power.pl -p $perf_run_number -s $scale_factor > composite.out ") && die "execute $dbdriver_sapdb_path/get_power.pl failed"; 

print "calculate throughput\n";
system("$dbdriver_sapdb_path/get_throughput.pl -p $perf_run_number -s $scale_factor -n $num_of_streams >> composite.out ") && die "execute $dbdriver_sapdb_path/get_throughput.pl failed"; 

my $fcomp = new FileHandle;
unless ( $fcomp->open( "< composite.out" ) ) { die "can't open file $!"; }
while ( <$fcomp> )
{
	chop;
	if ( /power/ )
	{
		my ( $var, $value ) = split /=/;
		$power = $value;
	}
	elsif ( /throughput/ )
	{
		my ( $var, $value ) = split /=/;
		$throughput = $value;
	}
}
close($fcomp);
system("rm", "composite.out");

print "calculate composite\n";
#calculate the query per hour * SF
$composite = sqrt( $power * $throughput);

unless ( $fcomp->open( "> $outfile" ) ) { die "can't open file $!"; }
printf $fcomp "power = %.2f\n", $power;
printf $fcomp "throughput = %.2f\n", $throughput;
printf $fcomp "composite = %.2f\n", $composite;
close($fcomp);

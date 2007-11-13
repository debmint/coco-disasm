#! /usr/bin/perl

# ######################################################### #
# perldis.pl - Perl version of os9 disassembler             #
#                                                           #
# This is a Perl version of os9disasm.  It functions        #
# similarly to the C version.                               #
#                                                           #
# ######################################################### #

# $Id$

# Include getopt functions

use strict qw(vars refs);
use warnings;
use integer;

use dtble;

use Data::Dumper;

use constant VERSION => "0.5.1";
# CPU TYPE DEFS
use constant M_09 => 0;
use constant M_03 => 1;

# Some control files, types, etc

#my $os9dflt = '@@@@@X&&D&DXL';
#my $cocodflt = '@@LLLL&&&&DLL';
#my $flexdflt = '@@LLLL&&&&DLL';
#my $motodflt = '@@@@@X&&D&DXL';
my %motodflt = (
    am_bytei => '@',
    am_dimm => '@',
    am_ximm => '@',
    am_yimm => '@',
    am_uimm => '@',
    am_simm => 'X',
    am_xidx => '&',
    am_yidx => '&',
    am_uidx => 'D',
    am_sidx => '&',
    am_drct => 'D',
    am_ext => 'X',
    am_rel => 'L',
);
my %os9dflt = (
    am_bytei => '@',
    am_dimm => '@',
    am_ximm => '@',
    am_yimm => '@',
    am_uimm => '@',
    am_simm => 'X',
    am_xidx => '&',
    am_yidx => '&',
    am_uidx => 'D',
    am_sidx => '&',
    am_drct => 'D',
    am_ext => 'X',
    am_rel => 'L',
);
my %cocodflt = (
    am_bytei => '@',
    am_dimm => '@',
    am_ximm => 'L',
    am_yimm => 'L',
    am_uimm => 'L',
    am_simm => 'L',
    am_xidx => '&',
    am_yidx => '&',
    am_uidx => '&',
    am_sidx => '&',
    am_drct => 'D',
    am_ext => 'L',
    am_rel => 'L',
);
my %flexdflt = (
    am_bytei => '@',
    am_dimm => '@',
    am_ximm => 'L',
    am_yimm => 'L',
    am_uimm => 'L',
    am_simm => 'L',
    am_xidx => '&',
    am_yidx => '&',
    am_uidx => '&',
    am_sidx => '&',
    am_drct => 'D',
    am_ext => 'L',
    am_rel => 'L',
);
my $dfltlbls = \%os9dflt;       # Note that $dfltlbls will be a REFERENCE

my $ostype = 'os9';

my $cputyp = M_09;
my $defdir = $ENV{HOME} . "/coco/defs/";

# print formatting

my $pseudcmd = "%5d  %04X %-14s %-10s %-6s %s\n";
my $realcmd = "%5d  %04X %-04s %-9s %-10s %-6s %s\n";
my $blankcmd = '%5d';

# filenames

my ($modfile, $asmfile, $cmdfilename);

# Register defines

my $regreg = ["d", "x", "y", "u", "s", "pc", "", "", "a", "b", "cc", "dp"];
my @regordr =("x", "y", "u", "s");
my $reg03 = ["d", "x", "y", "u", "s", "pc", "w", "v", "a", "b", "cc", "dp",
             "0", "", "e", "f"];
my $psh_puls = ["cc", "a", "b", "dp", "x", "y", "u", "pc"];
my $psh_pulu = ["cc", "a", "b", "dp", "x", "y", "s", "pc"];

my $ladds;  # array of data boundary trees;

my %am_vals = (
      "am_inh" => 0,
      "am_bytei" => 1,
      "am_dimm" => 2,
      "am_ximm" => 3,
      "am_yimm" => 4,
      "am_uimm" => 5,
      "am_simm" => 6,
      "am_xidx" => 7,
      "am_yidx" => 8,
      "am_uidx" => 9,
      "am_sidx" => 10,
      "am_drct" => 11,
      "am_ext" => 12,
      "am_rel" => 13,
      "am_reg" => 14,
      "am_psh" => 15,
      "am_wimm" => 16,
      "am_tfm" => 17,
      "am_bit" => 18
);

# Disassembly options

my ($pgwidth, $pgdepth, $pgnum, $pglin, $linnum) = (80, 66, 0, 0, 0);
my (@lblfnam, $shortlbl);
my $upcase;
my $wrtsrc = 0;
my $doingcmds = 0;
my $pass2 = 0;
my $useFCC = 1;

my $pc;     # File position pointer
my ($modsiz, $modnam, $modtyp, $modrev, $modexe, $moddata, $codend);

my ($cmdent, $prevent) = (0, 0);    # Entry address in previous command

# Try to get $nowclass to be not global????

my $nowclass;       # class to pass if not an amode
my ($modload, $dbounds);

# $symlst - a hash of hashes holding all symbols
# $symlst { "^" =>

my $symlst;     # Hash of hashes holding all the symbols
my $comments;   # Tree of comments

my $hdrlen;

sub errexit
{
    my ($rept,$errno) = @_;

    print STDERR shift;
    
    unless (defined $errno) {
        $errno = 1;
    }

    exit $errno;
}

# ################################################## #
# o9_fgetbyte() - reads two bytes from FILEHANDLE #
# Passed:  FILEHANDLE                                #
# Returns: two-byte integer value read               #
# ################################################## #

sub o9_fgetbyte
{
    my $fh = shift;

    return ord(getc($fh));
}

# ################################################# #
# o9_fgetword() - reads two bytes from FILEHANDLE #
# Passed:  FILEHANDLE                               #
# Returns: two-byte integer value read              #
# ################################################# #

sub o9_fgetword
{
    my $fh = shift;

    my $msb = o9_fgetbyte($fh);
    my $lsb = o9_fgetbyte($fh);

    return $msb*256 + $lsb;
}

# ######################################################## #
# clear_pbuf()  Sets all elements of hash to empty string  #
# Passed:  $hr - REFERENCE to hash                         #
# ######################################################## #

sub clear_pbuf {
    my $hr = shift;

    unless (ref $hr eq 'HASH') {
        print STDERR "clear_pbuf() error:  variable passed is NOT a HASH REF\n";
        return;
    }

    for ('instr', 'lbnm', 'mnem', 'operand', 'opcod') {
        $hr->{$_} = "";
    }
}

# ######################################################## #
# defaultclass() - returns the default class for the       #
#           specified addressing mode                      #
# ######################################################## #

sub defaultclass {
    return substr ($dfltlbls, shift, 1);
}

# ######################################################## #
# build_path() Locate a filename and build a full pathlist #
# Passed:  The filename                                    #
# Returns: The resulting pathlist                          #
# ######################################################## #

sub build_path {
    my $fname = shift;
    
    my $fh;

    if (substr($fname, 0, 2) eq "~/") {
        $fname =~ s#^~#$ENV{HOME}#;
    }

    # Try to open fh - if successful, we're done

    if (open $fh, $fname) {
        close $fh;
        return $fname;
    }
    elsif ($defdir) {      # Try one more shot
        if (substr($fname, 0, 2) eq "~/") {
            $defdir = ~ s#^~#$ENV{HOME}#;
        }

        $fname = $defdir . $fname;

        if (open $fh, $fname) {
            close $fh;
            return $fname;
        }
        else {          # Failure to access file !!!
            return undef;
        }
    }
}

sub do_ostype {
    if (uc (substr($_[1], 1)) eq "coco") {
        $ostype = 'coco';
        $dfltlbls = \%cocodflt;
    }
    else {
        die "Error! Unsupported OS $_[1]";
    }
}

# ####################################################### #
# addlbl() - Add a label to the list                      #
# Passed:   $loc=address of label                         #
#          $class name                                    #
# Returns: ptr to new entry if added, 0 if match found    #
# ####################################################### #

sub addlbl {
    my ($loc, $class) = @_;

    $loc &= 0xffff;

    # (for now, at least), don't add labels for class '@', '$', or '&'

    if ( index('@$&', $class) >= 0) {
        return undef;
    }

    unless ( index('!^$&@%ABCDEFGHIJKLMNOPQRSTUVWXYZ', $class) >= 0) {
        errexit( "Illegal label class" . $class);
    }

    ###### This may be a kludge - may need to fix later

    if ($class eq '^') {
        $loc &= 0x7f;

        if ($loc >= 0x20) { # don't allow labels for printables exc SPACE
            return undef;
        }
    }


    # If this address already exists, do nothing, return $loc undef

    if (exists ($symlst->{$class}->{$loc})) {
        #return undef;
        return $symlst->{$class}->{$loc};
    }

    # At this point, we don't have an existing entry, create one

    $symlst->{$class}->{$loc}->{sname} = sprintf("%s%04x", $class, $loc);
    
#    if ($upcase) {
#        $me->{sname} = uc($me->{sname});
#    }

    #$me->{myaddr} = $loc;

    # ??? Hope stdlbls don't pass through here
    $symlst->{$class}->{$loc}->{stdnam} = 0;

    return $symlst->{$class}->{$loc};
}

# ####################################################### #
# findlbl() - starting at $nl, search tree (or remainder) #
#       for a match.                                      #
# Passed: $nl = Class name to search                      #
#         $loc= address to match                          #
# Returned: ptr to correct entry if match,                #
#           0 if not found                                #
# ####################################################### #

sub findlbl {
    my ($nl, $loc) = @_;

    $loc &= 0xffff;

    if (exists $symlst->{$nl}) {
        if (exists $symlst->{$nl}->{$loc}) {
            return $symlst->{$nl}->{$loc};
        }
    }

    return undef;
}

# ####################################################### #
# clashere() - Determine if an addressing mode is         #
#              specified                                  #
# Passed: pointer to data boundaries                      #
#         $adrs - address                                 #
# ####################################################### #

sub clashere {
    my ($bp, $adrs) = @_;

    my $pt;

    unless ($pt = $bp) {
        return undef;
    }

    while (1) {
        my $h;

        if ($h < $pt->{b_lo}) {
            if ($pt->{dleft}) {
                $pt = $pt->{dleft};
            }
            else {
                return undef;
            }
        }
        elsif ($h > $pt->{b_hi}) {
            if ($pt->{dright}) {
                $pt = $pt->{dright};
            }
            else {
                return undef;
            }
        }
        else {      # Match found
            return $pt;
        }
    }
}

# ####################################################### #
# lblcalc() - Calculate the Label for a location          #
# Passed:  $adr - the address for the label               #
#          $amod - the Addressing Mode                    #
#          $bytesize - size of $adr in bytes              #
# Returns the string to insert into the source            #
# ####################################################### #

sub lblcalc {
    use integer;
    my ($adr, $amod, $bytesize) = @_;

    my $raw = $adr;     # Raw offset (postbyte) - was unsigned
    my $mainclass;
    my $label;
    my $kls;
    my $oclass;

    if ($amod eq 'am_rel') {
        $raw += $pc;
    }

    # If $amod is defined, we're doing a label class
    
    if (defined $amod) {

#        if ($kls = clashere ($ladds[$amod], $cmdent)) {
        if ($kls = clashere ($ladds->{$amod}, $cmdent)) {
            $mainclass = $kls->{b_typ};

            if ($kls->{dofst}) {
                $oclass = $kls->{dofst}->{oclas_maj};

                if ($kls->{dofst}->{add_to}) {
                    $raw -= $kls->{dofst}->{of_maj};
                }
                else {
                    $raw += $kls->{dofst}->{of_maj};
                }

                if ($kls->{dofst}->{incl_pc}) {
                    $raw += $pc;
                }
            }   # End if ($kls->{dofst}
        }       # End if (chashere)
        else {  # no class was found
            $mainclass = $dfltlbls->{$amod};
        }
    }
    else {      # Else we're not doing a label class
        if ($nowclass) {
#            $kls = clashere ($dbounds, $cmdent);
            $kls = clashere ($dbounds, $raw);
            $mainclass = $nowclass;

            if ($kls->{dofst}) {
                $oclass = $kls->{dofst}->{oclas_maj};

                if ($kls->{dofst}->{add_to}) {
                    $raw -= $kls->{dofst}->{of_maj};
                }
                else {
                    $raw += $kls->{dofst}->{of_maj};
                }

                if ($kls->{dofst}->{incl_pc}) {
                    $raw += $pc;
                }
            }
        }
         else {
             return undef;
        }
    }
 
    unless ($pass2) {

        if (index('@$&', $mainclass) >= 0) {

#            if ($mainclass eq '&' and $raw > 0) {
#                if ($bytesize == 1 and $raw > 0x7f) {
#                    $raw -= 256;
#                }
#                elsif ($raw > 0x7fff) {
#
#                    # The following should negate a 16-bit integer
#                    # on any system...
#                    # Note that $raw -= 65536 would fail on a 16-bit system
#
#                    $raw |= (-1) ^ 0xffff;
#                }
#            }

            return $raw;
        }
        else {
            $label = addlbl ($raw, $mainclass);     # Tmp
            
            # Get the label name from address entry
            $label = $label->{sname};
        }
    }
    else {
        my $mylabel;
 
        if ($mylabel = findlbl ($mainclass, $raw)) {
           
            printlbl (\$label, $mainclass, $raw, $mylabel, $bytesize);
        }
        else {
            if (index('^$@&%', $mainclass) >= 0) {
                printlbl (\$label, $mainclass, $raw, $mylabel, $bytesize);
            }
            else {
                printf STDERR "Lookup error on Pass 2 main)\n" .
                              "Cannot find %s%04x\n" .
                              "Cmd line thus far: %s\n",
                              $mainclass, $raw, $label;
                exit 1;
            }
        }

        # Now process offset, if any
        
        if ($kls && $kls->{dofst}) {
            my $cl = $kls->{dofst}->{oclas_maj};

            if ($kls->{dofst}->{add_to}) {
                $label .= "+";
            }
            else {
                $label .= "-";
            }

            if ($mylabel = findlbl ($cl, $kls->{dofst}->{of_maj})) {
                printlbl ( \$label, $cl, $kls->{dofst}->{of_maj}, $mylabel,
                                    $bytesize);
            }
            elsif (index ('^$@&%', $cl) >= 0) {
                printlbl (\$label, $cl, $kls->{dofst}->{of_maj}, $mylabel,
                                   $bytesize);
            }
            else {
                printf STDERR "Lookup error on Pass 2 main (offset)\n" .
                              "Cannot find %s%04x\n" .
                              "Cmd line thus far: %s\n",
                              $kls->{dofst}->{of_maj}, $raw, $label;
                exit 1;
            }
        }
    }

    return $label;
}

# ####################################################### #
# printlbl() - output formatted label name                #
# Passed:  $dest - POINTER to destination                 #
#          $clas = label class                            #
#          $adr  = address in program                     #
#          $dl   = label tree entry                       #
#          $pbytesize = size of $adr in bytes             #
# ####################################################### #

sub printlbl {
    my ($dest, $clas, $adr, $dl, $pbytsiz) = @_;

    #my $pbytsiz;

    # First, reassign class "@"

    if ($clas eq '@') {
        if (($adr < 9) || (($pbytsiz == 1) && ($adr > 244)) ||
            (($pbytsiz == 2) && ($adr > 65526))) {
            $clas = '&';
        }
        else {
            $clas = '$';
        }
    }

    if ($clas eq '$') {
        if ($pbytsiz == 1) {
            $adr &= hex 'ff';
            $$dest = sprintf '$%02x', $adr;
        }
        else {
            $adr &= hex 'ffff';
            $$dest = sprintf '$%04x', $adr;
        }
    }
    elsif ($clas eq '&') {
#        $$dest = sprintf '%d', $adr & hex 'ffff';
        $$dest = sprintf '%d', $adr;
    }
    elsif ($clas eq '^') {
        $$dest = "";

        if ($adr > hex 'ff') {
            movchr ($$dest, ($adr >> 8) & hex 'ff');
            $$dest .= '*256 +';
        }
        movchr ($$dest, $adr & hex 'ff');
    }
    elsif ($clas eq '%') {
        my $mask;

        $$dest = "%";

        if ($adr > hex 'ff') {
            $mask = hex '8000';
        }
        else {
            $mask = hex '80';
        }

        while ($mask) {
            $$dest .= $mask & $adr ? '1' : '0';
            $mask >>= 1;
        }
    }
    else {
        $$dest = $dl->{sname};
    }
}

# ####################################################### #
# rdlblfile() - Opens a label file and processes it       #
# Passed: name of label file, type of file (for error rept#
# Returns: 1 on success 0 on failure                      #
# ####################################################### #

sub rdlblfile {
    my $tmpname = build_path(shift);
    
    my $fh;

    unless (open $fh, $tmpname) {
        print STDERR "Error in opening " . shift() . "\"$tmpname\"\n\n";
        return 0;
    }

    # Good!  Now read the file

    while (<$fh>) {
        
        my ($label, $equ, $addr, $class) = split(/ |\t/, $_);
        next if ($label =~ /^[ |\t]*\*/);        # Comment
        next unless ($label && $equ && $addr && $class);

        # Class

        chomp $class;
        next unless (length ($class) == 1);
        
        $class = uc ($class);

        next unless (uc ($equ) eq "EQU");
        
        # Convert value to decimal if it's hex

        if ( $addr =~ /^\$/) {
            $addr =~ s/^.//;
            $addr = hex ($addr);
        }

        my $nl;

        if (($ostype eq 'os9') && ($class eq "D") && $addr <= $moddata) {
            $nl = findlbl ('D', $addr);
        }
        else {
            $nl = findlbl ($class, $addr);
        }

        if ($nl) {
            $nl->{sname} = $label;
            $nl->{stdname} = 1;

            if ($upcase) {
                $nl->{sname} = uc $nl->{sname};
            }
        }
    }

    close $fh;
    return 1;
}

sub getlabels {

    # First, do build-in ASCII definitions

    my $ctrlcodes = [   "nul", "soh", "stx", "etx",
                        "eot", "enq", "ack", "bel",
                        "bs", "ht", "lf", "vt",
                        "ff", "cr", "so", "si",
                        "dle", "dcl", "dc2", "dc3",
                        "dc4", "nak", "syn", "etb",
                        "can", "em", "sub", "esc",
                        "fs", "gs", "rs", "us",
                        "space"
    ];
    my ($nl, $stdlbl, $ctrl);
    
    for $ctrl (0 ... 32) {
        if ($nl = findlbl('^', $ctrl)) {
            $$nl->{sname} = $ctrlcodes->[$ctrl]
        }

        if ($upcase) { 
            $nl->{sname} = uc $nl->{sname} }
    }

    if ($ctrl = findlbl('^', 0x7f)) {
        $$nl->{sname} = "del";
        if ($upcase) { 
            $nl->{sname} = uc $nl->{sname} }
    }

    # Next read in the Standard systems names file

    if (($ostype eq 'os9') || ($ostype eq OS_Moto)) {
        rdlblfile("sysnames", "Sysnames file");
    }
    
    # Now read the standard label files

    my $rdbuf = $ENV{HOME};

    if ($ostype eq 'flex') {
        $rdbuf .= "/coco/defs/flex9lbl";
    }
    elsif ($ostype eq 'moto') {
        $rdbuf .= "/defs/donthavenone";
    }
    elsif ($ostype eq 'coco') {
        $stdlbl = "cocolbl";
    }
    else {      # Default, OS9
        $stdlbl = "dynalbl";
    }

    rdlblfile ($stdlbl, "Standard Label");
}

sub open_cmdfile {
    if ($cmdfilename) {
        print STDERR "Command file already specified\n" .
                     "Ignoring $_[1]";
    }
    else {
        unless ($cmdfilename = build_path $_[1]) {
            die "Error! Cannot access command file $_[1]";
        }
    }
}

# ----------------------------------------------------- #
# *hdr() - Does any preliminary module reads  such as   #
#       reading any headers                             #
# Passed: Filehandle for program module being read      #
# ----------------------------------------------------- #

sub rsdoshdr {
    errexit "RSDOS disassembly has not yet been implemented\nPlease be patient";}

# ########################################### #
# os9hdr() - Parses a Program module header   #
#     and populates certain variables         #
# Passed : filehandle for program module      #
# Returns: nothing                            #
# ########################################### #


sub os9hdr {
    my $fh = shift;

    # First test to see if it's a real OS9 module
    
    unless ( o9_fgetword($fh) eq 0x87cd ) {
        errexit "Not an OS9 program module";
    }

    my @sz = ("w", "w", "b", "b", "d", "w");

    for my $v ($modsiz, $modnam, $modtyp, $modrev, "hdr_crc", $modexe) {
        my $whatdo = shift @sz;
        if ($whatdo eq "w") {
            $v = o9_fgetword($fh);
            addlbl($v, "L");
        }
        elsif ($whatdo eq "b") {
            $v = o9_fgetbyte($fh);
        }
        else {      # "d" is for "discard"  - (the parity byte)
            o9_fgetbyte($fh);
        }
    }

    if (($modtyp < 3) || ($modtyp eq 0x0c) || ($modtyp gt 0x0b)) {
        $moddata = o9_fgetword($fh);
#        $pbytsiz = 2;    # Kludge???
        addlbl($moddata, "D");
        $hdrlen = 13;
    }
    else {
        $moddata = -1;  # Flag "Not Used"
        $hdrlen = 9
    }

    $codend = $modsiz - 3;  # Less CRC
}

use Getopt::Long;

# ##################################################### #
# do_opt() process command-line or command-file options #
# Passed: REFERENCE to array containing options         #
# ##################################################### #

sub do_opt {
    my $result = GetOptions ("o=s" => sub {open $wrtsrc, $_[1] or die "Cannot open output file\n";
                                          },
                             "x=s" => \&do_ostype,
                             "s=s" => \@lblfnam,
                             "c=s" => \&open_cmdfile,
                             "u"   => \$upcase,
                             "pw=i"=> \$pgwidth,
                             "pd=i"=> \$pgdepth,
                             "ls"  => sub {$shortlbl = 6},
                             "d=s" => \$defdir,
                             "3"   => sub { $cputyp = M_03}
                            );
}

my $inprog;


# #################################################### #
# startpage() - print headers, etc for page            #
# #################################################### #

sub startpage {

    if ($pglin) {
        while ($pglin++ < $pgdepth) {
            print "\n";
        }
    }

    ++$pgnum;
    $pglin = 0;

    my $now = localtime;
    printf "OS9 Cross Disassembler -Ver. %s %s Page %03d\n\n",
                VERSION, $now, $pglin;
    $pglin = 2;

    if ($pgnum == 1) {
        $linnum = 1;
        printf "%5d%20s * Disassembly by Os9disasm of %s\n", $linnum++,
                                                             "",
                                                             $modfile;
        blankline();
    }
}

# #################################################### #
# blankline() - prints a blank line to the listing     #
#           and/or the source listing                  #
# #################################################### #

sub blankline {
    if (!$pglin || ($pglin > ($pgdepth - 6))) {
        startpage();
    }

    printf "%5d\n", $linnum++;
    ++$pglin;

    if ($wrtsrc) {
        printf $wrtsrc, "\n";
    }
}

# #################################################### #
# listdata() - Routine to print "rmb"s for data defs   #
#              listdata recursively calls itself as    #
#              it passes over the entire tree          *
# Passed:  $me: pointer to base of tree                #
#          $upadr: Upper limit to data area            #
# #################################################### #

#sub listdata {
#    my ($me, $upadr) = @_;
#
#    # Process lower addresses before this
#
#    if ($me->{lnext}) {
#        listdata ( $me->{lnext}, $me->{myaddr});
#    }
#
#    # Now, we've come back... print this entry
#
#    my $pbf;
#
#    $pbf->{lbnm} = $me->{sname};
#
#    unless ($me->{myaddr} == $moddata) {
#        $pbf->{mnem} = "rmb";
#    }
#    else {
#        $pbf->{mnem} = "equ";
#    }
#
#    my $datasiz;
#
#    if ($me->{rnext}) {
#        my $srch = $me->{rnext};
#
#        while ($srch->{lnext}) {
#            $srch = $srch->{lnext};
#        }
#
#        $datasiz = ($srch->{myaddr}) - ($me->{myaddr});
#    }
#    else {
#        $datasiz = $upadr - $me->{myaddr};
#    }
#
#    # Don't print any class 'D' variables which are not in data area
#
#    if (($ostype eq 'os9') && ($me->{myaddr} > $moddata)) {
#        return
#    }
#
#    if ($me->{myaddr} != $moddata) {
#        $pbf->{operand} = sprintf "%d", $datasiz;
#    }
#    else {
#        $pbf->{operand} = ".";
#    }
#
#    $cmdent = $me->{myaddr};
#    #$prevent = $cmdent;
#    $prevent = $me->{myaddr};
#    printline ('real', $pbf, 'D', $me->{myaddr}, $me->{myaddr} + $datasiz);
#
#    if ($me->{rnext}) {
#        listdata ($me->{rnext}, $upadr);
#    }
#}

# #################################################### #
# os9dataprint() - Mainline routine to list data defs  #
# #################################################### #

sub os9dataprint {
    $inprog = 0;

    my $hdr = '* OS9 data area definitions';

    if (exists $symlst->{'D'}) {
        my $dta = $symlst->{'D'};
        blankline();
        printf "%5d %22s%s\n", $linnum++, "", $hdr;
        ++$pglin;
        blankline();

        if ($wrtsrc) {
            printf $wrtsrc, "%s\n", $hdr;
        }

        # First, if first entry is not D000, rmb bytes up to the first;

        my $pbf;
        my @d_list = sort {$a <=> $b} (keys %$dta);

        if  ($d_list[0]) {  # i.e., if not D000
            $pbf->{instr} = $pbf->{lbnm} = "";
            $pbf->{mnem} = "rmb";
            $cmdent = 0;
            $pbf->{operand} = $d_list[0];
            printline ('pseud', $pbf, 'D', 0, $d_list[0]);
        }
        
        my $d_low = shift @d_list;
        my $d_hi;

        $pbf->{mnem} = 'rmb';
        $pbf->{instr} = "";

        while ($d_hi = scalar @d_list) {
            $d_hi = shift @d_list;
            
            # Don't print any class 'D' variables which are not in data area
        
            if ($ostype eq 'os9' and $d_low >= $moddata) {
                last;
            }

            $cmdent = $d_low;
            $pbf->{lbnm} = $dta->{$d_low}->{sname};
            $pbf->{mnem} = "rmb";
            $pbf->{operand} = sprintf "%d", ($d_hi - $d_low);
            $cmdent = $prevent = $d_low;
            printline ('pseud', $pbf, 'D', $d_low, $d_hi);
            $d_low = $d_hi;
        }

#        printf $pseudcmd, $linnum, $cmdent, $pb->{instr}, $pb->{lbnm},
#                          $pb->{mnem}, $pb->{operand};
        if ($d_low == $moddata) {
            $pbf->{lbnm} = $dta->{$d_low}->{sname};
            $pbf->{mnem} = "equ";
            $pbf->{operand} = ".";
            $cmdent = $prevent = $d_low;
            printline ('pseud', $pbf, 'D', $d_low, $d_hi);
            blankline();
        }
    }
    else {
        return undef;
    }
}

# #################################################### #
# wrtequates() -Print out label defs                   #
# Passed: stdflag - 1 for std labels, 0 for externals  #
# #################################################### #

sub wrtequates {
    my $stdflg = shift;

    $inprog = 0;
    my @claspt = ('!', '^', 'A' ... 'K', 'M' ... 'Z');
    my $curnt;
    my $flg = $stdflg;

    unless ($stdflg) {
        shift @claspt; shift @claspt;
    }

    while ($nowclass = shift @claspt) {
        if (exists $symlst->{$nowclass}) {

            # For OS9, we only want external labels this pass

            if ( ($ostype eq 'os9') && ($nowclass eq 'D')) {
                if ($stdflg) {
                    return;
                }

                # Probably an error if this happens, but...
                # What we're doing is positioning $me to
                # last real data element

#                unless ($me = findlbl ($me, $moddata)) {
#                    return;
#                }
            }

            my $header = '%5d %21s';
            my $syshd = "* OS-9 system function equates\n",
            my $aschd = "* ASCII control character equates\n",
            my @genhd = ( "* class %s external label equates\n",
                          "* class %s standard named label equates\n"
                        );
            my $srchd;


            if ($nowclass eq '!') {
                #$header .= $syshd;
                $srchd = $syshd;
                $flg = -1;
            }
            elsif ($nowclass eq '^') {
                #$header .= $aschd;
                $srchd = $aschd;
                $flg = -1;
            }
            else {
                #$header .= $genhd[$stdflg];
                $srchd = $genhd[$stdflg];
                $flg = $stdflg;
            }

            tellabels ($nowclass, $flg, $srchd, 0);
        }
    }

    $inprog = 1;
}


# #################################################### #
# tellabels() - print out the labels for class in      #
#             "$me" tree.                              #
#                                                      #
# tellabels() is recursive, in that it calls itself    #
#   once for each entry in a class.  recursing through #
#   the class                                          #
# Passed: $me -    start point in label class to print #
#         $stdflg - flag if standard label             #
#         $class  - Class name                         #
#         $srchd  - The header to print                #
# #################################################### #

sub tellabels {
    my ($class,$stdflg, $srchd) = @_;

    my $hadwrote = 0;

    unless (exists $symlst->{$class}) {
        return undef;
    }

    my $me = $symlst->{$class};

    for my $adr (sort {$a <=> $b} keys %$me) {
        if (($stdflg < 0) or $stdflg == $me->{$adr}->{stdnam}) {
            my $pb;

            # Don't print real OS9 Data variables here

            unless (($ostype eq 'os9') && ($class eq 'D') &&
                                ($adr <= $moddata)) {
                unless ($hadwrote) {
                    blankline();
                    printf '%5d %21s' . $srchd, $linnum++, "", $class;
                    
                    if ($wrtsrc) {
                        printf $wrtsrc, $srchd, $class;
                    }
                    $hadwrote = 1;
                    blankline();
                }
    
                $cmdent = $adr;
                $pb->{instr} = "";
                $pb->{lbnm} = $me->{$adr}->{sname};
                $pb->{mnem} = 'equ';
                $pb->{operand} = sprintf '$%04x', $adr;
    
                $cmdent = $prevent = $adr;
                printline ('pseud', $pb, $class, $adr, $adr + 1);
            }
        }
    }
        
    return $hadwrote;
}

# #################################################### #
# movbytes() Move data bytes                           #
# Passed:  $db = data boundaries tree                  #
#          $bsiz = size of data unit (1 or 2 bytes)    #
# #################################################### #

sub movbytes {
    my ($db, $bsiz) = @_;

    my $pbuf;

    while ($pc < $db->{b_hi}) {
        my $tmps;
        my $valu = o9_fgetbyte (MODULE);

        # If Word-sized unit, get other byte from file

        if ($bsiz == 2) {
            $valu = ($valu << 8) + o9_fgetbyte (MODULE);
        }
        
        $tmps = lblcalc ($valu, undef, 2);

        if ($pass2) {
            if (length ($pbuf->{instr}) < 10) {
                $pbuf->{instr} .= sprintf ("%0" . chr (2 * $bsiz) . "x ",
                                            $valu);
            }

            if (length $pbuf->{operand}) {
                $pbuf->{operand} .= ",";
            }

            $pbuf->{operand} .= $tmps;

            if (length $pbuf->{operand} > 22) {
                $pbuf->{mnem} = ($bsiz == 1 ? "fcb" : "fdb");
                $cmdent = $pc;
                printline ('pseud', $pbuf, 'L', $pc);
                $cmdent = $pc + $bsiz;
                $pbuf = undef;
            }
        }

        $pc += $bsiz;
    }

    # Looping done.. now print any remaining data

    if (length $pbuf->{operand}) {
        $pbuf->{mnem} = ($bsiz == 1 ? "fcb" : "fdb");
        $cmdent = $pc;
        printline ('pseud', $pbuf, 'L', $pc);
    }
}

# #################################################### #
# add_delims() - Add delimiters to fcc/fcs operand -   #
#       checks string for nonexistant delimiter        #
#       and copies string witn delims to destination   #
# Passed:  The string to delimit                       #
# Returns: The delimited string                        #
#           exits program with error if no delim found #
# #################################################### #

sub add_delims {
    my $src = shift;

    for (ord '"', ord "'", ord '/', ord '#', ord '\\', ord '|', ord '$', ord '!', ord "%" ... ord "~") {
        if ( index ($src, $_) < 0) {
            return $_ . $src . $_;
        }
        else {
            errexit ( "Error! No delimiter found for $src");
        }
    }
}

# #################################################### #
# movasc() - Move $nb bytes into fcc (or fcs) statement#
# Passed: $nb: number of bytes to move                 #
# #################################################### #

sub movasc {
    my $nb = shift;

    my ($pbuf, $oper_tmp);

    $pbuf->{mnem} = "fcc";
    $cmdent = $pc;

    while ($nb--) {
        my $c = getc MODULE;

        if (($c =~ /[[:print:]]/) || (($c & 0x80) && $useFCC &&
                chr(ord($c) & 0x7f))) {
            if ($pass2) {
                if (length ($pbuf->{instr}) < 12) {
                    $pbuf->{instr} .= sprintf "%02x ", $c;
                }

                $oper_tmp .= sprintf "%s", $c & 0x7f;

                if ($c & 0x80) {
                    $pbuf->{mnem} = "fcs";
                    $pbuf->{operand} = add_delims (oper_tmp);
                    printline ('pseud', $pbuf, 'L', $pc);
                    $oper_tmp = "";
                    $cmdent = $pc + 1;
                    $pbuf->{mnem} = "fcc";
                }

                if ((length ($oper_tmp) > 24) ||
                            (length ($oper_tmp) && findlbl ('L', $pc + 1))) {
                    $pbuf->{operand} = add_delims (oper_tmp);
                    printline ('pseud', $pbuf, 'L', $pc);
                    $oper_tmp = "";
                    $cmdent = $pc + 1;
                    $pbuf->{mnem} = "fcc";
                }
            }       # end if (pass 2)
        }
        else {      # It's a control character
            if ($pass2 && (length ($oper_tmp))) {
                $pbuf->{operand} = add_delims (oper_tmp);
                printline ('pseud', $pbuf, 'L', $pc);
                $oper_tmp = "";
                $cmdent = $pc;
            }

            if (!pass2) {
                if (($c & 0x7f) < 33) {
                    addlbl ($c & 0x7f, '^');
                }
            }
            else {
                # A dummy ptr to pass to printlbl() to satisfy prototypes;
                my $nlp;

                $pbuf->{mnem} = "fcb";
                printlbl ($pbuf->{operand}, '^', $c, $nlp);
                $pbuf->{instr} = sprintf ( "%02x", $c & 0xff);
                printline ('pseud', $pbuf, 'L', $pc);
                $pbuf->{mnem} = "fcc";
            }
            
            $cmdent = $pc + 1;
        }

        ++$pc;
    }       # End while ($nb--) - all chars moved

    if (length ($oper_tmp)) {
        addelims ($pbuf->{operand}, $oper_tmp);
        printline ('pseud', $pbuf, 'L', $pc);
    }
}

# #################################################### #
# insertbnds() - Add boundary area specification       #
# Passed: $bp - pointer to boundary area ptr           #
# #################################################### #

sub insertbnds {
    my $bp = shift;

    my $amode = 0;
    $nowclass = $bp->{b_class};
    my $pbytesiz = 1;

    if ($bp->{b_typ} == 1) {       # ASCII
        movasc (($bp->{b_hi}) - $pc + 1)
    }
    
    # 6 = Word, 4 = Long

    elsif (($bp->{b_type} == 6) || ($bp->{b_typ} == 4)) {
        $pbytesiz = 2;
        movbytes ($bp, $pbytesiz);
    }

    # 2 = Byte  5 = short

    elsif (($bp->{b_type} == 2) || ($bp->{b_type} == 5)) {
        movbytes ($bp, $pbytesiz);
    }
    elsif ($bp->{b_type} == 3) {    # "C"ode not implemented yet
        return undef;
    }

    $nowclass = 0;

}


# #################################################### #
# getidxoffset() - Reads offset bytes for an indexed   #
#       instruction - 1 or 2 depending on postbyte     #
# Passed:  $postbyte                                   #
# Returns: the signed integer offset read from the     #
#          current file position                       #
#          Also updates $pc by the size of the offset  #
# #################################################### #

sub getidxoffset {
    my $postbyte = shift;

    my $offset;

    # Set up for n,R or n,PC later if other cases don't apply.

    if  (!($postbyte & 1)) {
        $offset = o9_fgetbyte (MODULE);

        # --------------------------
        if ($offset > 0x7f) {
            $offset -= 256;
        }
    }
    else {
#        my $msk = o9_fgetword (MODULE);
        $offset = o9_fgetword (MODULE);

        if ( $offset > 0x7fff) {

            # The following should negate a 16-bit integer
            # on any system...
            # Note that $offset -= 65536 would fail on a 16-bit system

            $offset |= (-1) ^ 0xffff;
        }
#
#        if ($msk > hex '7fff') {
#            $offset = (-1) ^ hex 'ffff';
#        }
#        else {
#            $offset = 0;
#        }
#
#        $offset |= $msk;
    }

    $pc += ($postbyte & 1) + 1;

    return $offset;
}

# #################################################### #
# txidx() - Handle indexed modes                       #
# Passed:  $pbuf : REFERENCE to $pbuf (possibly        #
#                  already partially written to        #
#          $amode : Current addressing mode            #
# #################################################### #

sub txidx {
    my ($pbuf, $amode) = @_;

    # To prevent warnings about uninitialized variable

    unless (defined $pbuf->{operand}) {
        $pbuf->{operand} ="";
    }

    my $postbyte;
    my ($oper1, $oper2) = ("", "");

    $pbuf->{opcod} .= sprintf "%02x ", ($postbyte = o9_fgetbyte (MODULE));
    ++$pc;

    if ($postbyte == hex "9f") {   # Extended indirect [mmnn]
        $amode = 'am_ext';

        my $da = o9_fgetword (MODULE);
        $pbuf->{opcode} .= sprintf "%04x", $da;
        $pc += 2;

        $oper1 = lblcalc ($da, $amode, 2);
        $pbuf->{operand} .= '[' . $oper1 . ']' ;
        return 1;
    }
    else {          # Then anything but extended indirect

        # Current register offset

        my $regnam = $regordr[(($postbyte >> 5) & 3)];
        #$amode += ($postbyte >> 5) & 3;
        
        # The following hopefully replaces the above for string amodes
        
        my $am = $am_vals{$amode};
        $am += ($postbyte >> 5) & 3;
        my $found;

#        my @zip = keys(%am_vals);  # Dummy to reset has pointer

        foreach my $key (keys %am_vals) {
            if ($am_vals{$key} == $am) {
#        while ( my ($key, $val) = each (%am_vals)) {
#            if ($val == $am) {
                $amode = $key;
                $found = 1;
                last;
            }
        }

        unless ($found) {
            print STDERR "txidx(): Lookup error. Cannot find amode $am\n";
            exit 1;
        }

        if (! ($postbyte & hex "80")) {     # 0RRx xxxx = 5-bit
            unless ($postbyte & hex "1f") {
                return undef;
            }

            # $sbit is the offset portion of the postbyte

            my $sbit = $postbyte & hex "0f";

            if ($postbyte & hex "10") {   # Sign bit
                $sbit -= hex "10";
            }

            $oper1 = lblcalc ($sbit, $amode, 1);
            $pbuf->{operand} .= sprintf "%s,%s", $oper1, $regnam;
            return 1;
        }
        else {
            my ($kls, $myclass);

            if ($kls = clashere ($ladds->{$amode}, $pc)) {
                $myclass = $kls->{b_typ};
            }
            else {
                $myclass = $dfltlbls->{$amode};
            }

            my $pbytsiz = ($postbyte & 1) + 1;

            my $pbflg = $postbyte & hex "0f";

            if ( $pbflg == 0) {
                $oper1 = sprintf ",%s+", $regnam;
            }
            elsif ($pbflg == 1) {
                $oper1 = sprintf ",%s++", $regnam;
            }
            elsif ($pbflg == 2) {
                $oper1 = sprintf ",-%s", $regnam;
            }
            elsif ($pbflg == 3) {
                $oper1 = sprintf ",--%s", $regnam;
            }
            elsif ($pbflg == 4) {
                $oper1 = sprintf ",%s", $regnam;
            }
            elsif ($pbflg == 5) {
                $oper1 = sprintf "b,%s", $regnam;
            }
            elsif ($pbflg == 6) {
                $oper1 = sprintf "a,%s", $regnam;
            }
            elsif ($pbflg == hex "0b") {
                $oper1 = sprintf "d,%s", $regnam;
            }
            elsif (($pbflg == 8) || ($pbflg == 9)) { # <n,R >nn,R
                regput ($postbyte, $pbuf, \$amode, \$oper1);
                $oper2 = ",$regnam";
            }
            elsif (($pbflg == hex "0c") ||              # n,PC (8-bit)
                            ($pbflg == hex "0d")) {     # nn,PC (16-bit)
                $amode = 'am_rel';                
                $myclass = $dfltlbls->{$amode};
                regput ($postbyte, $pbuf, \$amode, \$oper1);
                $oper2 = ',pcr';
            }
            else {
                return undef;
            }
        }
    }

    if ($postbyte & hex "10") {
        $pbuf->{operand} .= "[" . $oper1 . $oper2 . "]";
    }
    else {
        $pbuf->{operand} .= $oper1 . $oper2;
    }

    return 1;
}

# #################################################### #
# regput() - Process either n[n],R or n[n],PC offsets  #
# Passed: $postbyte                                    #
#         $pbf - print buffer ref                      #
#         $amode - REFERENCE to amode variable         #
#         $op1 - the first part of the operand string  #
# #################################################### #

sub regput {
    my ($postbyte, $pbf, $amode, $op1) = @_;

    my $offset = getidxoffset ($postbyte);

#    if (($offset < 127) && ($offset > -128) && ($postbyte & 1)) {
#        $$op1 = ">";
#    }

    if ($postbyte & 1) {
        if ($offset < 0x80 and $offset >= -128) {
            $$op1 = ">";
        }

        $pbf->{opcod} .= sprintf "%04x", $offset & hex "ffff";
    }
    else {
        $pbf->{opcod} .= sprintf "%02x", $offset & hex "ff";
    }

    $$op1 .= lblcalc ($offset, $$amode, 1);
#    $$op2 = sprintf ",%s", $reg;
}

# #################################################### #
# getcmd() parse through data and assemble (if valid)  #
#       an assembler command line                      #
# Return Status:                                       #
#   Success: file positioned at begin of next cmd      #
#   Failure: file positioned at original entry point   #
# #################################################### #

sub getcmd {
    my $pbuf = shift;
    
    unless (defined $pbuf->{operand}) {     # Prevent warnings about
        $pbuf->{operand} = "";              #   undefined variable
    }

    my $oldpos = tell MODULE;     # Keep original file position for failure
    my ($tbl, $pbytsiz);
    my ($firstbyte, $pcbump) = (0, 0);
    my $noncode;

    unless ( $tbl = iscmd (\$firstbyte, \$pcbump, $cputyp, MODULE)) {
        # Not a legal instruction byte.
        # Reset file to original position
        seek MODULE, $oldpos, 0;
        return undef;
    }

    # If illegal code, then print it out in pass 2

    if ($noncode && $pass2) {
        return undef;
    }

    # Now move stuff to printer buffer

    $pbuf->{instr} = sprintf ( ($pcbump == 2) ? "%04x" : "%02x", $firstbyte);
    $pc += $pcbump;
    $pcbump = 0;    #reset offset - not sure if needed
    $pbuf->{mnem} = $tbl->{mnem};
    my $amode = $tbl->{opttype};
    $pbytsiz = $tbl->{postbytes};
    
    # Special case for OS9

    if (($ostype eq'os9') && ($tbl->{mnem} =~ /swi2/)) {
        $pbuf->{mnem} = "os9";
        $pbuf->{opcod} = sprintf "%02x", (my $ch = o9_fgetbyte(MODULE));
        ++$pc;

        unless ($pass2) {
            addlbl ($ch, '!');
        }
        else {
            if (my $nl = findlbl ('!', $ch)) {
                $pbuf->{operand} .= $nl->{sname};
            }
            else {
                $pbuf->{operand} = sprintf('$%02x', $ch);
            }
        }

        return $pbuf;
    }

    # take care of aim/oim/tim immediate #
    # TODO  add addressing mode for this!!! 

    if ($tbl->{mnem} =~ /im/) {
        my $im = o9_fgetbyte(MODULE);

        ++$pc;
        $pbuf->{opcode} = sprintf "%02x", $im;
        $pbuf->{operand} = sprintf '#$%02x', $im;
    }

    if ($amode eq 'am_inh') {
        return $pbuf;
    }
    elsif ($amode eq 'am_xidx') {
        unless (txidx (\%$pbuf, $amode)) {
            return undef;
        }
        else {
            return $pbuf;
        }
    }
    elsif ($amode eq 'am_psh') {
        my $pbyte = o9_fgetbyte (MODULE);
        ++$pc;
        $pbuf->{opcod} = sprintf '%02x', $pbyte;
        
        my $cptr;

        if (($pbyte == 0x36) || ($pbyte == 0x37)) {
            $cptr = $psh_pulu;
        }
        else {
            $cptr = $psh_puls;
        }

        for my $ct (0 ... 7) {
            if ($pbyte & 1) {
                if (length ($pbuf->{operand})) {
                    $pbuf->{operand} .= "," . $cptr->[$ct];
                }
                else {
                    $pbuf->{operand} = $cptr->[$ct];
                }

            }

            $pbyte >>= 1;
        }
    }
    elsif ($amode eq 'am_tfm') {
        my $pbyte = o9_fgetbyte(MODULE);
        ++$pc;
        $pbuf->{opcod} = sprintf "%02x", $pbyte & 0xff;
        my $ct = ($pbyte >> 4) &0x0f;
        $pbyte &= 0xff;

        if (($ct > 4) || ($pbyte > 4)) {        # Illegal
            return undef;
        }

        # Do r0
        
        $pbuf->{operand} = $regreg->[$ct];

        if ( !($tbl->{opcode}) & 1) {
            $pbuf->{operand} .= "+";
        }
        elsif ($tbl->{opcode} == 0x39) {
            $pbuf->{operand} .= "-";
        }

        # now r1

        $pbuf->{operand} .= "," . $regreg->[$pbyte & 0x0f];

        if (($tbl->{opcode} == 0x38) || ($tbl->{opcode} == 0x3b)) {
            $pbuf->{operand} .= "+";
        }
        elsif ($tbl->{opcode} == 0x39) {
            $pbuf->{operand} .= "-";
        }
    }
    elsif ($amode eq 'am_reg') {
        my $regpt;

        if ($cputyp == M_09) {
            $regpt = $regreg;
        }
        else {
            $regpt = $reg03;
        }

        my $pbyte = o9_fgetbyte (MODULE);
        ++$pc;
        $pbuf->{opcod} = sprintf '%02x', $pbyte & 0xff;
        my $rg_to = ($pbyte >> 4) & 0x0f;
        #my $rg_from = $pbuf & 0x0f;
        my $rg_from = $pbyte & 0x0f;

        # Abort if 6809 codes are out of range
        
        if ($cputyp == M_09) {
            if ( ($rg_to > 0x0b) || ($rg_from > 0x0b) ) {
                return undef;        # Illegal postbyte
            }
        }

        unless ($rg_to == 0x0c) {      #"0" register for 6309
            unless (($rg_to & 0x08) == ($rg_from & 0x08)) {
                return undef;
            }
        }

        $pbuf->{operand} = $regpt->[$rg_to] . "," . $regpt->[$rg_from & 0x0f];
    }
    elsif (($amode eq 'am_bytei') || ($amode eq 'am_dimm') ||
            ($amode eq 'am_ximm') || ($amode eq 'am_yimm') ||
            ($amode eq 'am_uimm') || ($amode eq 'am_wimm') ||
            ($amode eq 'am_drct') || ($amode eq 'am_ext')  ||
            ($amode eq 'am_rel')) {

        my $offset;

        # Immediate mode

        unless (($amode eq 'am_drct') || ($amode eq 'am_ext') ||
                ($amode eq 'am_rel')) {
            $pbuf->{operand} = '#';
        }

        if ($tbl->{postbytes} == 4) {
            $offset = o9_fgetword (MODULE);
            $pbuf->{opcod} = sprintf '%04x', $offset & hex 'ffff';
            $pbuf->{operand} .= '$' . $pbuf->{opcod};
            $pc += 2;

            $offset = o9_fgetword (MODULE);
            my $tmpbuf = sprintf '%04x', $offset & hex 'ffff';
            $pbuf->{opcod} .= $tmpbuf;
            $pbuf->{operand} .= $tmpbuf;
            $pc += 2;
            return $pbuf;
        }
        elsif ($tbl->{postbytes} == 2) {
            $offset = o9_fgetword (MODULE);
            $pbuf->{opcod} .= sprintf '%04x', ($offset & hex 'ffff');
            #$pbuf->{operand} .= sprintf "%04x", ($offset & hex 'ffff');

            if ($offset > 0x7ffff) {
                $offset |= (-1) ^ 0xffff;
            }

            $pc += 2;
        }
        else {
            # NOTE!!! This may not work for signed bytes
            $offset = o9_fgetbyte (MODULE);
            ++$pc;

            # Try this...

            if ($offset > 127) {
                $offset -= 256;
            }

            if (($amode eq 'am_drct') || ($amode eq 'am_bytei')) {
                $offset &= 0xff;
            }
            
            $pbuf->{opcod} .= sprintf "%02x", $offset & 0xff;
        }

        my $lc = lblcalc ($offset,$amode, $tbl->{postbytes});
        unless (defined $lc) {
            print STDERR "lblcalc() returnd undef for offset $offset, for amode $amode\n";
        }
        $pbuf->{operand} .= $lc;
        #$pbuf->{operand} .= lblcalc ($offset,$amode, $tbl->{postbytes}); 
    }
    elsif ($amode eq 'am_bit') {
        my $pbyte = o9_fgetbyte (MODULE);
        ++$pc;

        $pbuf->{opcod} = sprintf '%02x', $pbyte;
        $pbyte &= 0xff;
        my $tmpbyt = $pbyte >> 6;

        if ($tmpbyt > 3) {
            return undef;
        }

        $pbuf->{operand} = sprintf '%s,', $psh_puls->[$tmpbyt];
        $tmpbyt = ($pbyte >> 3) & 0x7;
        my $tmpbuf = o9_fgetbyte(MODULE);
        ++$pc;
        $pbuf->{opcod} .= sprintf '%02x', $tmpbuf;
        $pbuf->{operand} .= sprintf '%d,$02x,%d', $tmpbyt,
                                                  $tmpbuf,
                                                  $pbyte & 0x07;
    }

    if ($upcase) {
        $pbuf->{operand} = uc ($pbuf->{operand});
    }

    return 1;
}

# #################################################### #
# nonboundslbl ()                                      #
#                                                      #
# #################################################### #

sub nonboundslbl {
    my $pb = shift;

    for my $addr ($prevent + 1 ... $cmdent-1) {
        my ($nl, $lblnam, $operand, $mnem);

        if ($nl = findlbl ('L', $addr)) {
#            my $bf->{lbnm} = $nl->{sname};
#            $bf->{mnem} = "equ";
            $mnem = 'equ';

            ### NOTE:: I don't think $addr would ever be > $cmdent????
            if ($addr > $cmdent) {
#                $bf->{operand} = sprintf '*+%d', $addr - $cmdent;
                $operand = sprintf '*+%d', $addr - $cmdent;
            }
            else {
#                $bf->{operand} = sprintf '*-%d', ($cmdent - $addr);
                $operand = sprintf '*-%d', ($cmdent - $addr);
            }

            if ($upcase) {
                $nl->{myaddr} = uc $nl->{myaddr};
                $nl->{instr} = uc $nl->{instr};
                $nl->{sname} = uc $nl->{sname};
                $mnem = uc $mnem;
                $operand = uc $operand;
            }
    
            #printf $pseudcmd, $linnum++, (sprintf "%04x", $addr), "",
            printf $pseudcmd, $linnum++, $addr, "",
                            $nl->{sname}, $mnem, $operand;
            ++$pglin;
    
            if ($wrtsrc) {
                printf $wrtsrc "%s %s %s\n", $nl->{sname}, $mnem, $operand;
            }
        }
    }
}

# #################################################### #
# printcomment() - print comments for range            #
# Passed: $class - label class                         #
#         $cmdlow - low address range for comment      #
#         $cmdhi  - upper address range                #
# #################################################### #

sub printcomment {
    my ($class, $cmdlow, $cmdhi) = @_;

    for my $addr ($cmdlow ... $cmdhi) {
        my $me = $comments->{$class};

        while ($me) {
            if ($addr < $me->{adrs}) {
                $me = $me->{cmtLeft};
            }
            elsif ( $addr > $me->{adrs}) {
                $me = $me->{cmtRight};
            }
            else {
                my $line;
                
                unless ($line = $me->{commts}) {
                    return undef;
                }

                do {
                    printf "%5d       * %s\n", $linnum++, $line->{ctxt};

                    if ($wrtsrc) {
                        printf $wrtsrc, "* %s\n", $line->{ctxt};
                    }
                } while ($line = $line->{nextline});

                $me = undef; # Found the address, force a break
            }
        }
    }
}

# #################################################### #
# printformatted() - print the formatted line          #
# Passed: $pfmt (format) , $pb (data buffer)           #
# #################################################### #

sub printformatted {
    my ($pfmt, $pb) = @_;

    if ( !$pglin || ($pglin > ($pgdepth - 6))) {
        startpage();
    }

    if ($upcase) {
        for (keys %$pb) {
            $pb->{$_} = uc $pb->{$_};
        }
    }
    unless (defined $pb->{instr} and defined $pb->{lbnm} and
        defined $pb->{mnem} and defined $pb->{operand}) {
        printf STDERR "UNDEFINED entry @ %04x\n", $cmdent;
        print STDERR Dumper $pb;
    }
    if ($pfmt eq 'real') {
        unless (defined $pb->{opcod}) {
            printf STDERR "UNDEFINED entry @ %04x\n", $cmdent;
            print STDERR Dumper $pb;
        }
    }

    if ($pfmt eq 'pseud') {
        printf $pseudcmd, $linnum, $cmdent, $pb->{instr}, $pb->{lbnm},
                          $pb->{mnem}, $pb->{operand};
    }
    else {
        printf $realcmd, $linnum, $cmdent, $pb->{instr}, $pb->{opcod},
                         $pb->{lbnm}, $pb->{mnem}, $pb->{operand};
    }
}
# #################################################### #
# outputline - actually print out the line             #
# Passed: $pfmt - the printf format                    #
#         $pb - the print buffer hash reference        #
# #################################################### #

sub outputline {
    my ($pfmt, $pb) = @_;

    my $nl;
    if ($inprog && ($nl = findlbl ('L', $cmdent))) {
        $pb->{lbnm} = $nl->{sname};
    }

    printformatted ($pfmt, $pb);
}

# #################################################### #
# printcleanup() - cleans up and prepares for next cmd #
# Passed: $pb print buffer data                        #
#         $cmdlow - command entry address              #
# #################################################### #

sub printcleanup {
    my ($pb, $cmdlow) = @_;

    $prevent = $cmdent;
    #$cmdlen = 0;
    ++$pglin;
    ++$linnum;
}

# #################################################### #
# printline() - The generic, global printline function #
#       It checks for unlisted boundaries, prints the  #
#       line, and then does cleanup                    #
# Passed:  $pfmt - The printf format to use            #
#          $pb - hash containing the data              #
#          $class - label class                        #
#          $cmdlow - the command entry address         #
#          $cmdhi - the end address of the command     #
# #################################################### #

sub printline {
    my ($pfmt, $pb, $class, $cmdlow, $cmdhi) = @_;

    nonboundslbl($pb);     #check for non-boundary labels

    printcomment ($class, $cmdlow, $cmdhi);
    outputline ($pfmt, $pb);
    printcleanup ($pb, $cmdlow);
}

# #################################################### #
# os9modline() - print out OS9 mod line                #
# Note: We're going to assume that the file is         #
#       is positioned at the right place - we may      #
#       try merged modules later                       #
# #################################################### #

sub os9modline {
    my $ll = $symlst->{'L'};
    my $progstart = tell MODULE;
    
    $inprog =0;
    my $prtbf;
    $cmdent = o9_fgetword(MODULE);
    $prtbf->{instr} = sprintf '%04x', o9_fgetword(MODULE);
    $prevent = $cmdent + 1; # To prevent nonboundslbl() printouts
    $prtbf->{lbnm} = "";
    $prtbf->{mnem} = "mod";
    $prtbf->{operand} = sprintf "%s,%s,\$%02x,\$%02x,%s",
                                findlbl('L', $modsiz)->{sname},
                                findlbl('L', $modnam)->{sname},
                                $modtyp, $modrev,
                                findlbl('L', $modexe)->{sname};

    if ($hdrlen == 13) {
        $prtbf->{operand} .= "," . findlbl('D',$moddata)->{sname};
    }

    printline ('pseud', $prtbf, "", 0, 0);

    seek MODULE, $progstart+$hdrlen, 0;
}

# #################################################### #
# progdis() : the mainline disassembler routine        #
# #################################################### #

sub progdis {
    ($linnum, $pglin) = (0, 0);

    if ($pass2) {
        if ($ostype eq 'os9') {
            os9modline();
        }
    
        wrtequates(1);  # Do Standard Named labels
        wrtequates(0);  # then non-standard labels
    
        if ($ostype eq 'os9') {
            os9dataprint();
        }
    }

    # Now begin parsing through program
    
    if ($ostype eq 'coco') {
        $pc = $modload;

        if ($pass2) {
            rsorg();
        }
    }
    else {
        $pc = $hdrlen;
    }

    while ($pc < $codend) {
        my $cmdlen = 0;     # count of bytes in current command
        my $cmdbuf = "";

        $prevent = $cmdent;
        $cmdent = $pc;

        my $pbuf = undef;

        if (my $nl = findlbl('L',$pc)) {
            $pbuf->{lbnm} = $nl->{sname};
        }
        else {
            $pbuf->{lbnm} = "";
        }

        # To prevent errors in printout, initialize all elements

        for ("instr", "mnem", "opcod", "operand") {
            $pbuf->{$_} = "";
        }

        my $oldpos = tell MODULE;

        if (my $bp = clashere($dbounds, $pc)) { # In data boundary?
            insertbnds($bp);
        }
        else {      # No, not in data boundary
            if (getcmd($pbuf) and ($pc <= $codend)) {
                if ($pass2) {
                    printline ('real', $pbuf, "L", $cmdent, $pc);
                }
            }

            # Allocate byte
            else {      # getcmd() failed (probably illegal code
                    # Condition at this point.  either getcmd() returned
                    # NULL, implying illegal code, or we've run past the
                    # end of the module.  In this case, this wasn't a legal
                    # code, so must either be data or garbage.
                    # In either case, we'll pick off a single byte and
                    # write an "fcb" command, bump $pc to next byte,
                    # and continue

                    # Restore file pisition to original

                seek MODULE, $oldpos, 0;
                my $bcode = o9_fgetbyte(MODULE);
                $pc = $cmdent;

                if ($pass2) {
                    $pbuf->{instr} = sprintf ("%02x", $bcode);
                    $pbuf->{mnem} = "fcb";
                    $pbuf->{operand} = sprintf ("\$%02x", $bcode);
                    printline ('pseud', $pbuf, "L", $cmdent,
                                $cmdent+1);
                }

                ++$pc;
                $cmdent = $pc;
            }
        }

        # CocoDos supports multiple block loads
        # If we're at the end of the current block,
        # check to see if another follows, and, if so,
        # update pointers to new state

        if (($ostype eq 'coco') && ($pc == $codend)) {
            my $hstart;

            if (($hstart = o9_fgetbyte(MODULE)) == 0) { # Another block follows
                my $prevcmdent = $cmdent;

                seek MODULE, -1, SEEK_CUR;  # unget the byte
                rsdoshdr;

                if ($pass2) {
                    # The following $cmdent juggling is to try to get
                    # any label equ's just before the new org to print
                    # with the correct offset

                    my $newcmdent = $cmdent;
                    rsorg();
                }
            }
            else {      # else it's the end
                o9_fgetword(MODULE);
                $modexe = o9_fgeword(MODULE);
                addlbl ($modexe, 'L');
            }
        }
    }

    # Write end-of-file lines

    if ($pass2) {
        my $pbuf = {};

        if ($ostype eq 'coco') {
            rsend();
        }
        elsif ($ostype eq 'os9') {
            $cmdent = $pc;
            clear_pbuf ($pbuf);
            $pbuf->{instr} = sprintf "%04x%02x", o9_fgetword (MODULE),
                                                 o9_fgetbyte (MODULE);
            $pbuf->{mnem} = 'emod';

            blankline();
            printline ('pseud', $pbuf, 0, 0, 0);
            blankline();
            $pc += 3;
            $cmdent = $pc;

            clear_pbuf ($pbuf);

            if (my $nl = findlbl ('L', $pc)) {
                $pbuf->{lbnm} = $nl->{sname};
                $pbuf->{mnem} = 'equ';
                $pbuf->{operand} = '*';
                printline ('real', $pbuf, 0, 0, 0);
            }

            blankline();
            clear_pbuf ($pbuf);
            $pbuf->{mnem} = 'end';
            printformatted ('real', $pbuf);

            if ($wrtsrc) {
                print $wrtsrc " end\n";
            }
        }
    }
}

# ------------------------------------------- #

# ########################################### #
#                                             #
# Program Entry Point                         #
#                                             #
# ########################################### #

# ------------------------------------------- #

# Get Program Options

do_opt(\@ARGV);

# Open module file for read

if ($#ARGV < 0) {
    print  STDERR "Error, Please specify a module file to read";
    exit 1;
}

$modfile = $ARGV[0];

open (MODULE,"<",$modfile) or die ("Error opening $modfile");
binmode MODULE;

# Now do some pass-1 stuff

if ($ostype eq 'coco') {
    rsdoshdr(MODULE);
}
# Here we'd do some elsif's if any ostypes other than OS9 are added
else {          # Default: OS9
    os9hdr(MODULE);
}

progdis (MODULE);
$pass2 = 1;

getlabels (MODULE);

# Pass 1 done, now do Pass 2

seek MODULE, 0, 0;

if ($ostype eq 'coco') {
    rsdoshdr MODULE;    # Reset header pointers to first header
}

#print Dumper $symlst;exit;
progdis (MODULE);         # Pass 2

exit 0;


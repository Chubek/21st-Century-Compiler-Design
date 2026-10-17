# Generator backend, loaded by aurocks.pl. Standard Perl only.
use strict;
use warnings;
use FindBin qw($Bin);

sub regex_nullable {
    my ($a) = @_;
    my $t = $a->{type};
    return 1 if $t eq 'empty' || $t eq 'eol';
    return length($a->{text}) == 0 if $t eq 'literal';
    return $a->{min} == 0 || regex_nullable($a->{child}) if $t eq 'repeat';
    return scalar(grep { regex_nullable($_) } @{$a->{parts}}) > 0 if $t eq 'alt';
    return scalar(grep { !regex_nullable($_) } @{$a->{parts}}) == 0 if $t eq 'concat';
    return 0;
}

sub validate_recursion {
    my ($rules, $nt) = @_;
    my %nullable;
    my $changed = 1;
    while ($changed) {
        $changed = 0;
        for my $r (@$rules) {
            next if $nullable{$r->{lhs}};
            if (!grep { !$nt->{$_} || !$nullable{$_} } @{$r->{rhs}}) {
                $nullable{$r->{lhs}} = 1;
                $changed = 1;
            }
        }
    }
    my %edge;
    for my $r (@$rules) {
        for my $s (@{$r->{rhs}}) {
            last unless $nt->{$s};
            $edge{$r->{lhs}}{$s} = 1;
            last unless $nullable{$s};
        }
    }
    my (%active, %done);
    my $visit;
    $visit = sub {
        my ($n) = @_;
        die "left-recursive nullable-prefix cycle at $n\n" if $active{$n};
        return if $done{$n};
        $active{$n} = 1;
        $visit->($_) for sort keys %{$edge{$n} || {}};
        delete $active{$n};
        $done{$n} = 1;
    };
    $visit->($_) for sort keys %$nt;
    undef $visit;
}

sub emit {
    my ($pro, $epi, $start, $directives, $rules, $nt, $api) = @_;
    $api //= "parse_$start";
    die "reserved entrypoint\n" if $api =~ /^au_/;
    die "C keyword cannot be an entrypoint\n" if $api =~ /^(?:auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|inline|int|long|register|restrict|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while|_Bool|_Complex|_Imaginary)$/;
    my $skip = exists $directives->{skip} ? substr($directives->{skip}[0], 1, -1) : undef;
    my ($terms, $layout, $ids) = collect_terminals($rules, $skip);
    my $skip_id = -1;
    if ($layout) { $skip_id = @$terms; $layout->{id} = $skip_id; push @$terms, $layout }
    for my $term (@$terms) {
        die "nullable terminal/layout [$term->{text}]; use %empty\n" if regex_nullable($term->{ast});
    }
    validate_recursion($rules, $nt);
    my @names = sort keys %$nt;
    my %number; @number{@names} = (0 .. $#names);
    my (@ordered, @first);
    for my $name (@names) {
        push @first, scalar @ordered;
        push @ordered, grep { $_->{lhs} eq $name } @$rules;
    }
    push @first, scalar @ordered;
    my $width = 1;
    for (@ordered) { $width = @{$_->{rhs}} if @{$_->{rhs}} > $width }
    die "grammar exceeds case-study capacity\n" if $width > 64 || @ordered > 512 || @names > 128;
    my $tables = "enum { AU_MAX_RHS = $width };\n";
    my @rx;
    for my $term (@$terms) {
        my $i = $term->{id};
        my $nfa = regex_compile_nfa($term->{ast}, $term);
        my $n = @{$nfa->{states}};
        $tables .= "static const AURxState au_rx_$i\[\] = {\n";
        for my $s (@{$nfa->{states}}) {
            $tables .= sprintf(" {AU_RX_%s, %d, %d, %d},\n", uc($s->{kind}), $s->{out1}, $s->{out2}, $s->{arg});
        }
        $tables .= "};\nstatic int au_class_$i(unsigned int cls, unsigned char ch) {\n (void)ch;\n switch (cls) {\n";
        for my $j (0 .. $#{$nfa->{classes}}) {
            $tables .= " case $j: return " . regex_class_expr($nfa->{classes}[$j], 'ch') . ";\n";
        }
        $tables .= " default: return 0;\n }\n}\n";
        push @rx, " {au_rx_$i, $n, $nfa->{start}, au_class_$i, " . c_quote($term->{text}) . "}";
    }
    @rx = (' {NULL, 0, 0, NULL, ""}') unless @rx;
    $tables .= "static const AURx au_regex[] = {\n" . join(",\n", @rx) . "\n};\n";
    $tables .= "static const int au_layout = $skip_id;\n";
    $tables .= "typedef struct { size_t count; int rhs[AU_MAX_RHS]; } AURule;\nstatic const AURule au_rules[] = {\n";
    my $actions = '';
    for my $i (0 .. $#ordered) {
        my $r = $ordered[$i];
        my @symbols = map { exists $nt->{$_} ? $number{$_} : -1 - $ids->{$_} } @{$r->{rhs}};
        $tables .= ' {' . scalar(@symbols) . ', {' . join(',', @symbols ? @symbols : (0)) . "}},\n";
        $actions .= "    case $i: { " . action_c($r->{action}, scalar @symbols) . " } break;\n";
    }
    $tables .= "};\nstatic const size_t au_first[] = {" . join(',', @first) . "};\n";
    $tables .= "static int au_choice(AUParser *, size_t, AUCont *, size_t);\n";
    for my $name (@names) {
        $tables .= "static int au_nt_$name(AUParser *p, size_t at, AUCont *k) { return au_choice(p, at, k, $number{$name}); }\n";
    }
    $tables .= "static int (*const au_nonterminal[])(AUParser *,size_t,AUCont *) = {" . join(',', map { "au_nt_$_" } @names) . "};\n";
    my $entry = <<"C";
AUResult $api(const char *input, size_t length, const AULimits *limits)
{
    AUParser p = {0};
    p.result.status = AU_INVALID;
    if (!input) return p.result;
    p.input = input; p.length = length;
    p.limits = limits ? *limits : (AULimits){1000000, 256};
    if (p.limits.depth > 256) return p.result;
    p.result.status = AU_SYNTAX;
    AUCont done = {au_complete, NULL};
    (void)au_nt_$start(&p, 0, &done);
    return p.result;
}
C
    my $runtime = slurp("$Bin/aurocks-runtime.c");
    $runtime =~ s{/\* AUROCKS_PROLOGUE \*/}{$pro};
    $runtime =~ s{/\* AUROCKS_TABLES \*/}{$tables};
    $runtime =~ s{/\* AUROCKS_ACTIONS \*/}{$actions};
    $runtime =~ s{/\* AUROCKS_ENTRY \*/}{$entry};
    print "/* Generated by aurocks.pl: continuation-based recursive descent. */\n";
    print "#include <stddef.h>\n#include <stdint.h>\n#include <stdlib.h>\n#include <string.h>\n", $runtime, "\n", $epi;
}
1;

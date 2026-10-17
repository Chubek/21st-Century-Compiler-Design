#!/usr/bin/env perl
use strict;
use warnings;
use 5.010;
use FindBin qw($Bin);

=pod

=head1 NAME

aurocks.pl - generate a backtracking recursive-descent C99 parser

=head1 SYNOPSIS

  perl support/aurocks.pl grammar.g > parser.c

The generated output is complete C99 source code.

=head1 DESCRIPTION

C<aurocks.pl> is a parser generator for the small grammar
language used by the Aurocks examples.  It reads a grammar file, extracts its
C prologue, directives, productions, semantic actions, and C epilogue, and
writes a C99 parser implementation to standard output.

The generated parser is scannerless: literal terminals and regular-expression
terminals are compiled into matcher code that runs directly against the input
buffer.  No separate lexer is required.  A C<%skip> regular expression may be
used to consume layout such as spaces and newlines between grammar symbols.

Continuations retry alternatives even after a callee succeeds locally but its
caller fails. The first complete parse in depth-first production order wins.
Actions run only after end-of-input acceptance. This is not GLR or GLL.
The generator loads aurocks-emit.pl and aurocks-runtime.c alongside this file;
the generated C source is standalone. See aurocks.md for the full contract.

=head1 INPUT GRAMMAR

Grammar files are divided into three regions:

=over 2

=item * C<%{ ... %}>

The C prologue is copied after public runtime types and before functions.
Place type declarations, constructor prototypes, includes, and other
application-specific declarations here.

=item * The first and second C<%%>

The production section.  This contains directives and grammar rules.

=item * Text after the second C<%%>

The C epilogue.  It is copied verbatim to the end of the generated source.
This is normally where constructor and helper function definitions live.

=back

The generator requires a C<%start> directive and at least one production.
Production names are C identifiers.  A production has one or more
alternatives separated by C<|> and terminated by C<;>:

  expression:
        NUMBER tail { $$ = make_expression($1, $2); }
      ;

Symbols may be nonterminal names, quoted literals, regular expressions written
between slashes, or the special C<%empty> symbol.

=head1 DIRECTIVES

Only the following directives are supported, before the first C<%%>:

=over 2

=item C<%start I<name>>

Selects the start production.  The generated API is
C<parse_I<name>(input, length, limits)>, returning an C<AUResult>.

=item C<%skip I</regexp/>>

Defines insignificant input to skip before terminal matches and at end of
input. The byte-oriented regex subset is compiled into an NFA and must consume
at least one byte. Use C<+>, rather than C<*>, for layout.

=back

Unsupported directives are errors; no GLR/SPPF policies are silently ignored.

=head1 SEMANTIC ACTIONS

Actions are C fragments enclosed in braces.  The following substitutions are
performed before the action is emitted:

=over 2

=item C<$$>

Becomes the local C<result> variable.  Assign the semantic value of the
production to it.

=item C<$1>, C<$2>, ...

Become entries of the local C<v> array, corresponding to right-hand
side symbols.

=item C<$TOKEN>

Becomes C<token>, a nul-terminated copy of the last direct terminal child.
C<$TOKEN_LEN> is its byte length. Without a direct terminal these are NULL/0.

=back

Values are C<void *>; actions supply casts and checked arithmetic.
Use C<au_alloc(p, size)> for result-owned storage. No numeric boxing is inferred.
Substitutions leave C strings/comments intact. Nested braces are supported.
Without an action, unary rules forward their child; others produce NULL,
which is a valid successful value. Mid-rule actions/predicates are unsupported.

=head1 GENERATED C RUNTIME

The generated source contains:

=over 2

=item * A parser state with input length, budgets, and furthest failure.

=item * A whitespace/layout skipper driven by C<%skip>.

=item * Literal matching for quoted terminals.

=item * Compiled regular-expression matching for slash-delimited terminals.

=item * One generated function per nonterminal.

=item * Continuations for the remaining production and caller context.

=item * C<parse_I<start>()> and C<au_result_free()> entry points.

=back

Results distinguish AU_OK, AU_SYNTAX, AU_LIMIT, AU_NOMEM, and AU_INVALID.
Always release them with au_result_free(), including on failure.
--entrypoint changes the public name; it does not add legacy aliases.

=head1 SCANNERLESS MATCHING

Quoted terminals are matched literally at the current input position.
Regular-expression terminals are anchored automatically, so a terminal cannot
skip forward looking for a later match. Each terminal chooses its longest
accepted prefix and does not surrender bytes. C<%skip> is applied before terminals
and once after the start production.

Because matching happens inside production evaluation, a grammar can express
token-like rules and structural rules in one specification.  This is useful
for languages where lexical interpretation depends on grammatical context.

=head1 LIMITATIONS

Left recursion, including cycles through nullable prefixes, is rejected.
Backtracking can require exponential work. Budget exhaustion is inconclusive,
not a syntax error. There is no memoization, recovery, or ambiguity forest.

Regular expressions use the documented ASCII/byte subset, not full POSIX ERE
or Perl syntax. Actions must be valid C and respect the ownership API.
The two %% separators must occupy their own lines. See aurocks.md for limits.

=head1 OUTPUT

The generated C99 source is written to standard output.  It contains the C
prologue, parser runtime, generated parsing functions, public entry points,
and C epilogue in one translation unit.  Diagnostics and malformed-input
errors are written by the Perl generator to standard error and terminate with
a non-zero exit status.

=head1 EXAMPLES

Generate and compile the book's calculator:

  perl support/aurocks.pl example-code/aurocks/calculator.g > parser.c
  cc -std=c99 parser.c -o calculator

The resulting parser exposes:

  AUResult parse_Expr(const char *input, size_t length,
                      const AULimits *limits);

=head1 AUTHOR

The Aurocks project.

=cut

sub slurp {
    my ($path) = @_;
    open my $fh, '<', $path or die "cannot open $path: $!";
    local $/;
    return <$fh>;
}

sub quoted {
    my ($s) = @_;
    return undef unless $s =~ /^\s*(['"])(.*)\1\s*$/s;
    my $v = $2;
    $v =~ s/\\(.)/regex_escape_value($1)/ges;
    return $v;
}

sub grammar_tokens {
    my ($s) = @_;
    my @t;
    pos($s) = 0;
    while (pos($s) < length($s)) {
        next if $s =~ /\G(?:\s+|\/\/[^\n]*|\/\*.*?\*\/)/gcs;
        if ($s =~ /\G(%[A-Za-z_]+|[A-Za-z_]\w*|[:;|])/gc) {
            push @t, $1;
        } elsif ($s =~ /\G('(?:\\.|[^'\\])*'|"(?:\\.|[^"\\])*"|\/(?:\\.|[^\/\\])*\/)/gcs) {
            push @t, $1;
        } elsif ($s =~ /\G\{/gc) {
            my $start = pos($s) - 1;
            my $depth = 1;
            while ($depth && pos($s) < length($s)) {
                next if $s =~ /\G(?:"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\/\/[^\n]*|\/\*.*?\*\/)/gcs;
                die "unterminated action string/comment\n" if substr($s, pos($s)) =~ /^(?:['"]|\/\*)/;
                $s =~ /\G(.)/gcs or die "unterminated action\n";
                ++$depth if $1 eq '{';
                --$depth if $1 eq '}';
            }
            die "unterminated action\n" if $depth;
            push @t, substr($s, $start, pos($s) - $start);
        } else {
            die "invalid grammar token at byte " . pos($s) . "\n";
        }
    }
    return @t;
}

sub parse_grammar {
    my ($g) = @_;
    my @regions = split /^%%[ \t]*\r?$/m, $g, -1;
    die "grammar needs two %% markers on their own lines\n" unless @regions == 3;
    my ($header, $body, $epilogue) = @regions;
    my $prologue = '';
    $prologue = $1 if $header =~ s/%\{(.*?)%\}//s;
    my @header = grammar_tokens($header);
    my %d;
    while (@header) {
        my $key = shift @header;
        die "unsupported directive $key\n" unless $key eq '%start' || $key eq '%skip';
        my $value = shift @header // die "missing value for $key\n";
        $key = substr($key, 1);
        die "duplicate %$key\n" if exists $d{$key};
        die "invalid %$key\n" unless $key eq 'start' ? $value =~ /^[A-Za-z_]\w*$/ : $value =~ m{^/.*\/$}s;
        $d{$key} = [$value];
    }
    my $start = $d{start} ? $d{start}[0] : undef;
    die "missing %start directive\n" unless $start;
    my @rules;
    my @t = grammar_tokens($body);
    while (@t) {
        my $lhs = shift @t;
        die "expected production name\n" unless $lhs =~ /^[A-Za-z_]\w*$/;
        die "expected colon after $lhs\n" unless @t && shift(@t) eq ':';
        while (1) {
            my (@rhs, $action);
            while (@t && $t[0] ne '|' && $t[0] ne ';') {
                my $s = shift @t;
                die "symbol after action in $lhs\n" if defined $action;
                if ($s =~ /^\{(.*)\}$/s) { $action = $1; next }
                die "invalid symbol $s in $lhs\n" unless $s eq '%empty' || $s =~ /^[A-Za-z_]\w*$/ || $s =~ m{^['"/]};
                push @rhs, $s;
            }
            die "missing semicolon for $lhs\n" unless @t;
            die "use %empty for an empty alternative\n" unless @rhs;
            if (grep { $_ eq '%empty' } @rhs) {
                die "%empty must be the only symbol\n" unless @rhs == 1;
                @rhs = ();
            }
            push @rules, { lhs => $lhs, rhs => \@rhs, action => $action // '' };
            last if shift(@t) eq ';';
        }
    }
    die "no productions found\n" unless @rules;
    my %nonterm = map { $_->{lhs} => 1 } @rules;
    die "undefined start $start\n" unless $nonterm{$start};
    for my $r (@rules) {
        for my $s (@{$r->{rhs}}) {
            die "undefined nonterminal $s\n" if $s =~ /^[A-Za-z_]\w*$/ && !$nonterm{$s};
        }
    }
    return ($prologue, $epilogue, $start, \%d, \@rules, \%nonterm);
}

sub c_quote {
    my ($s) = @_;
    $s =~ s/\\/\\\\/g;
    $s =~ s/"/\\"/g;
    $s =~ s/\?/\\?/g; # Prevent C99 trigraphs in generated diagnostic strings.
    $s =~ s/\n/\\n/g;
    $s =~ s/([^\x20-\x7e])/sprintf('\\%03o', ord($1))/ge;
    return qq{"$s"};
}

sub action_c {
    my ($a, $count) = @_;
    my $out = '';
    pos($a) = 0;
    while (pos($a) < length($a)) {
        if ($a =~ /\G("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\/\/[^\n]*|\/\*.*?\*\/)/gcs) { $out .= $1 }
        elsif ($a =~ /\G\$(\$|TOKEN_LEN\b|TOKEN\b|[0-9]+)/gc) {
            my $v = $1;
            if ($v eq '$') { $out .= 'result' }
            elsif ($v eq 'TOKEN') { $out .= 'token' }
            elsif ($v eq 'TOKEN_LEN') { $out .= 'token_len' }
            else {
                die "action reference \$$v outside RHS\n" unless $v >= 1 && $v <= $count;
                $out .= "v[" . ($v - 1) . "]";
            }
        } else {
            $a =~ /\G(.)/gcs;
            die "unknown action substitution\n" if $1 eq '$';
            $out .= $1;
        }
    }
    return $out;
}

sub regex_escape_value {
    my ($c) = @_;
    return "\n" if $c eq 'n';
    return "\r" if $c eq 'r';
    return "\t" if $c eq 't';
    return "\f" if $c eq 'f';
    return "\b" if $c eq 'b';
    return "\a" if $c eq 'a';
    return chr(11) if $c eq 'v';
    return "\0" if $c eq '0';
    return '\\' if $c eq '\\';
    die "unsupported escape \\$c\n" if $c =~ /[A-Za-z0-9]/;
    return $c;
}

sub c_byte_literal {
    my ($n) = @_;
    return "'\\\\'" if $n == 92;
    return "'\\''" if $n == 39;
    return sprintf("0x%02X", $n) if $n < 32 || $n > 126;
    return sprintf("'%s'", chr($n));
}

sub regex_posix_class_expr {
    my ($name, $var) = @_;
    my %map = (
        alnum => "((($var) >= '0' && ($var) <= '9') || ((($var) >= 'A' && ($var) <= 'Z') || (($var) >= 'a' && ($var) <= 'z')))",
        alpha => "((($var) >= 'A' && ($var) <= 'Z') || (($var) >= 'a' && ($var) <= 'z'))",
        blank => "(($var) == ' ' || ($var) == '\\t')",
        cntrl => "((($var) <= 0x1F) || (($var) == 0x7F))",
        digit => "(($var) >= '0' && ($var) <= '9')",
        graph => "(($var) >= 0x21 && ($var) <= 0x7E)",
        lower => "(($var) >= 'a' && ($var) <= 'z')",
        print => "(($var) >= 0x20 && ($var) <= 0x7E)",
        punct => "((($var) >= 0x21 && ($var) <= 0x2F) || (($var) >= 0x3A && ($var) <= 0x40) || (($var) >= 0x5B && ($var) <= 0x60) || (($var) >= 0x7B && ($var) <= 0x7E))",
        space => "($var == ' ' || $var == '\\t' || $var == '\\n' || $var == '\\r' || $var == '\\f' || $var == '\\x0b')",
        upper => "(($var) >= 'A' && ($var) <= 'Z')",
        xdigit => "((($var) >= '0' && ($var) <= '9') || ((($var) >= 'A' && ($var) <= 'F') || (($var) >= 'a' && ($var) <= 'f')))",
        word => "((($var) >= '0' && ($var) <= '9') || ((($var) >= 'A' && ($var) <= 'Z') || (($var) >= 'a' && ($var) <= 'z')) || ($var) == '_')",
    );
    my $k = lc $name;
    die "unsupported POSIX class [$name]\n" unless exists $map{$k};
    return $map{$k};
}

sub regex_class_item_expr {
    my ($item, $var) = @_;
    if ($item->{type} eq 'char') {
        return "($var == " . c_byte_literal($item->{ch}) . ")";
    }
    if ($item->{type} eq 'range') {
        return "(($var >= " . c_byte_literal($item->{from}) . ") && ($var <= " . c_byte_literal($item->{to}) . "))";
    }
    if ($item->{type} eq 'posix') {
        return regex_posix_class_expr($item->{name}, $var);
    }
    if ($item->{type} eq 'shorthand') {
        return regex_posix_class_expr($item->{name}, $var);
    }
    die "unknown character class item\n";
}

sub regex_class_expr {
    my ($node, $var) = @_;
    my @parts = map { regex_class_item_expr($_, $var) } @{ $node->{items} || [] };
    die "empty character class\n" unless @parts;
    my $expr = join(' || ', @parts);
    return $node->{neg} ? "(!($expr))" : "($expr)";
}

sub regex_parse {
    my ($pat) = @_;
    my $i = 0;
    my $len = length($pat);
    my ($parse_expr, $parse_concat, $parse_repeat, $parse_atom, $parse_class, $parse_class_atom);

    my $peek = sub { $i < $len ? substr($pat, $i, 1) : undef };
    my $eat = sub { $i < $len ? substr($pat, $i++, 1) : undef };
    my $err = sub { die "bad regular expression [$pat]\n"; };

    my $escaped_node = sub {
        my ($c) = @_;
        return { type => 'class', neg => 0, items => [ { type => 'posix', name => 'digit' } ] } if $c eq 'd';
        return { type => 'class', neg => 1, items => [ { type => 'posix', name => 'digit' } ] } if $c eq 'D';
        return { type => 'class', neg => 0, items => [ { type => 'posix', name => 'space' } ] } if $c eq 's';
        return { type => 'class', neg => 1, items => [ { type => 'posix', name => 'space' } ] } if $c eq 'S';
        return { type => 'class', neg => 0, items => [ { type => 'posix', name => 'word' } ] } if $c eq 'w';
        return { type => 'class', neg => 1, items => [ { type => 'posix', name => 'word' } ] } if $c eq 'W';
        return { type => 'char', ch => ord(regex_escape_value($c)) };
    };

    $parse_class_atom = sub {
        my $c = $peek->();
        $err->() unless defined $c;
        if ($c eq '\\') {
            $eat->();
            my $e = $eat->();
            $err->() unless defined $e;
            return { type => 'posix', name => { d => 'digit', s => 'space', w => 'word' }->{$e} }
                if $e =~ /^[dsw]$/;
            return { type => 'char', ch => ord(regex_escape_value($e)), escaped => 1 };
        }
        if ($c eq '[' && substr($pat, $i + 1, 1) eq ':') {
            my $end = index($pat, ':]', $i + 2);
            $err->() if $end < 0;
            my $name = substr($pat, $i + 2, $end - ($i + 2));
            $i = $end + 2;
            return { type => 'posix', name => $name };
        }
        $eat->();
        return { type => 'char', ch => ord($c), escaped => 0 };
    };

    $parse_class = sub {
        my $neg = 0;
        my @items;
        $eat->();
        my $c = $peek->();
        if (defined $c && $c eq '^') {
            $neg = 1;
            $eat->();
        }
        my $first = 1;
        while (1) {
            $c = $peek->();
            $err->() unless defined $c;
            last if $c eq ']' && !$first;
            my $atom = $parse_class_atom->();
            # NOTE: the historical `!$first` guard here prevented a range
            # whose start atom is in first position, so [a-z] compiled to
            # {a,-,z} while [ab-c] worked.  A leading '-' stays literal
            # regardless: '-' can only start a range when followed by
            # '-' (peek check below), and '[-...]' has no '-' in second
            # position.  Fixed 2026-09-23.
            if ($atom->{type} eq 'char'
                && $atom->{ch} != 45
                && defined($peek->())
                && $peek->() eq '-'
                && substr($pat, $i + 1, 1) ne ']') {
                $eat->();
                my $rhs = $parse_class_atom->();
                $err->() unless $rhs->{type} eq 'char';
                $err->() if $rhs->{ch} < $atom->{ch};
                push @items, { type => 'range', from => $atom->{ch}, to => $rhs->{ch} };
            } else {
                push @items, $atom;
            }
            $first = 0;
        }
        $eat->();
        return { type => 'class', neg => $neg, items => \@items };
    };

    $parse_atom = sub {
        my $c = $peek->();
        $err->() unless defined $c;
        if ($c eq '(') {
            $eat->();
            my $node = $parse_expr->();
            $err->() unless defined($peek->()) && $peek->() eq ')';
            $eat->();
            return $node;
        }
        if ($c eq '[') {
            return $parse_class->();
        }
        if ($c eq '.') {
            $eat->();
            return { type => 'any' };
        }
        if ($c eq '$') {
            $eat->();
            return { type => 'eol' };
        }
        if ($c eq '^' && $i == 0) {
            $eat->();
            return { type => 'empty' };
        }
        if ($c eq '\\') {
            $eat->();
            my $e = $eat->();
            $err->() unless defined $e;
            return $escaped_node->($e);
        }
        if ($c eq '*' || $c eq '+' || $c eq '?' || $c eq '{' || $c eq '}' || $c eq '^') {
            $err->();
        }
        $eat->();
        return { type => 'char', ch => ord($c) };
    };

    $parse_repeat = sub {
        my $node = $parse_atom->();
        my $quantified = 0;
        while (defined(my $c = $peek->())) {
            if ($c =~ /^[*+?{]$/) { $err->() if $quantified++; }
            if ($c eq '*') {
                $eat->();
                $node = { type => 'repeat', min => 0, max => undef, child => $node };
                next;
            }
            if ($c eq '+') {
                $eat->();
                $node = { type => 'repeat', min => 1, max => undef, child => $node };
                next;
            }
            if ($c eq '?') {
                $eat->();
                $node = { type => 'repeat', min => 0, max => 1, child => $node };
                next;
            }
            if ($c eq '{') {
                $eat->();
                my $read_num = sub {
                    my $n = '';
                    while (1) {
                        my $d = $peek->();
                        last unless defined $d && $d =~ /[0-9]/;
                        $n .= $d;
                        $eat->();
                    }
                    return $n;
                };
                my $m = $read_num->();
                $err->() if $m eq '';
                my $n = $m;
                if (defined($peek->()) && $peek->() eq ',') {
                    $eat->();
                    $n = $read_num->();
                    $n = undef if $n eq '';
                }
                $err->() unless defined($peek->()) && $peek->() eq '}';
                $eat->();
                die "repeat bound exceeds 256\n" if length($m) > 3 || $m > 256 || (defined($n) && (length($n) > 3 || $n > 256));
                $node = { type => 'repeat', min => int($m), max => defined($n) ? int($n) : undef, child => $node };
                next;
            }
            last;
        }
        return $node;
    };

    $parse_concat = sub {
        my @parts;
        while (1) {
            my $c = $peek->();
            last unless defined $c && $c ne '|' && $c ne ')';
            push @parts, $parse_repeat->();
        }
        return { type => 'empty' } unless @parts;
        return $parts[0] if @parts == 1;
        return { type => 'concat', parts => \@parts };
    };

    $parse_expr = sub {
        my @alts = ($parse_concat->());
        while (defined($peek->()) && $peek->() eq '|') {
            $eat->();
            push @alts, $parse_concat->();
        }
        return $alts[0] if @alts == 1;
        return { type => 'alt', parts => \@alts };
    };

    my $ast = $parse_expr->();
    $err->() if $i != $len;
    return $ast;
}

sub regex_literal_ast {
    my ($text) = @_;
    return { type => 'literal', text => $text };
}

sub regex_compile_nfa {
    my ($ast, $desc) = @_;
    my @states;
    my @classes;

    my $new_state = sub {
        my ($kind, $out1, $out2, $arg) = @_;
        die "regular expression exceeds 1024 NFA states\n" if @states >= 1024;
        push @states, {
            kind => $kind,
            out1 => defined($out1) ? $out1 : -1,
            out2 => defined($out2) ? $out2 : -1,
            arg  => defined($arg)  ? $arg  : 0,
        };
        return $#states;
    };

    my $patch = sub {
        my ($outs, $target) = @_;
        for my $o (@$outs) {
            $states[$o->[0]]{$o->[1]} = $target;
        }
    };

    my $concat = sub {
        my ($a, $b) = @_;
        $patch->($a->{outs}, $b->{start});
        return { start => $a->{start}, outs => $b->{outs} };
    };

    my $compile;
    my $class_id = sub {
        my ($node) = @_;
        return $node->{class_id} if defined $node->{class_id};
        $node->{class_id} = scalar @classes;
        push @classes, $node;
        return $node->{class_id};
    };

    my $repeat_frag;
    $repeat_frag = sub {
        my ($node, $count) = @_;
        my $frag;
        for (1 .. $count) {
            my $next = $compile->($node);
            $frag = defined $frag ? $concat->($frag, $next) : $next;
        }
        return $frag || do {
            my $s = $new_state->('jmp', undef, undef, undef);
            { start => $s, outs => [ [ $s, 'out1' ] ] };
        };
    };

    $compile = sub {
        my ($node) = @_;
        my $type = $node->{type};
        if ($type eq 'literal') {
            my @bytes = unpack('C*', $node->{text});
            return do {
                my $s = $new_state->('jmp', undef, undef, undef);
                { start => $s, outs => [ [ $s, 'out1' ] ] };
            } unless @bytes;
            my $frag;
            for my $b (@bytes) {
                my $s = $new_state->('char', undef, undef, $b);
                my $next = { start => $s, outs => [ [ $s, 'out1' ] ] };
                $frag = defined $frag ? $concat->($frag, $next) : $next;
            }
            return $frag;
        }
        if ($type eq 'char') {
            my $s = $new_state->('char', undef, undef, $node->{ch});
            return { start => $s, outs => [ [ $s, 'out1' ] ] };
        }
        if ($type eq 'any') {
            my $s = $new_state->('any', undef, undef, 0);
            return { start => $s, outs => [ [ $s, 'out1' ] ] };
        }
        if ($type eq 'eol') {
            my $s = $new_state->('eol', undef, undef, 0);
            return { start => $s, outs => [ [ $s, 'out1' ] ] };
        }
        if ($type eq 'class') {
            my $id = $class_id->($node);
            my $s = $new_state->('class', undef, undef, $id);
            return { start => $s, outs => [ [ $s, 'out1' ] ] };
        }
        if ($type eq 'empty') {
            my $s = $new_state->('jmp', undef, undef, undef);
            return { start => $s, outs => [ [ $s, 'out1' ] ] };
        }
        if ($type eq 'concat') {
            my @parts = @{ $node->{parts} || [] };
            my $frag = $compile->(shift @parts);
            for my $part (@parts) {
                $frag = $concat->($frag, $compile->($part));
            }
            return $frag;
        }
        if ($type eq 'alt') {
            my @parts = @{ $node->{parts} || [] };
            my $frag = $compile->(shift @parts);
            for my $part (@parts) {
                my $rhs = $compile->($part);
                my $s = $new_state->('split', $frag->{start}, $rhs->{start}, undef);
                $frag = { start => $s, outs => [ @{ $frag->{outs} }, @{ $rhs->{outs} } ] };
            }
            return $frag;
        }
        if ($type eq 'repeat') {
            my $min = $node->{min};
            my $max = $node->{max};
            my $frag;
            if (defined $max) {
                die "bad regular expression [$desc->{text}]\n" if $max < $min;
                $frag = $min ? $repeat_frag->($node->{child}, $min) : do {
                    my $s = $new_state->('jmp', undef, undef, undef);
                    { start => $s, outs => [ [ $s, 'out1' ] ] };
                };
                my $extra = $max - $min;
                for (1 .. $extra) {
                    next if $extra <= 0;
                    my $opt = $compile->($node->{child});
                    my $s = $new_state->('split', $opt->{start}, undef, undef);
                    my $maybe = { start => $s, outs => [ @{ $opt->{outs} }, [ $s, 'out2' ] ] };
                    $frag = $concat->($frag, $maybe);
                }
            } else {
                $frag = $min ? $repeat_frag->($node->{child}, $min) : do {
                    my $s = $new_state->('jmp', undef, undef, undef);
                    { start => $s, outs => [ [ $s, 'out1' ] ] };
                };
                my $star = $compile->($node->{child});
                my $s = $new_state->('split', $star->{start}, undef, undef);
                $patch->($star->{outs}, $s);
                my $tail = { start => $s, outs => [ [ $s, 'out2' ] ] };
                $frag = defined $frag ? $concat->($frag, $tail) : $tail;
                return $frag;
            }
            return $frag;
        }
        die "bad regular expression [$desc->{text}]\n";
    };

    my $frag = $compile->($ast);
    my $match = $new_state->('match', undef, undef, undef);
    $patch->($frag->{outs}, $match);
    return {
        states => \@states,
        classes => \@classes,
        start => $frag->{start},
    };
}

sub parse_terminal_symbol {
    my ($s) = @_;
    my $lit = quoted($s);
    return { kind => 'literal', text => $lit, key => 'lit:' . unpack('H*', $lit), ast => regex_literal_ast($lit) }
        if defined $lit;
    if ($s =~ m!^/(.*)/$!s) {
        my $pat = $1;
        return { kind => 'regex', text => $pat, key => 're:' . unpack('H*', $pat), ast => regex_parse($pat) };
    }
    return undef;
}

sub collect_terminals {
    my ($rules, $skip) = @_;
    my %by_key;
    my %by_symbol;
    my @terms;
    my $add = sub {
        my ($desc) = @_;
        return $by_key{$desc->{key}} if exists $by_key{$desc->{key}};
        $desc->{id} = scalar @terms;
        $by_key{$desc->{key}} = $desc->{id};
        push @terms, $desc;
        return $desc->{id};
    };
    for my $r (@$rules) {
        for my $s (@{ $r->{rhs} || [] }) {
            my $desc = parse_terminal_symbol($s) or next;
            my $id = $add->($desc);
            $by_symbol{$s} = $id unless exists $by_symbol{$s};
        }
    }
    my $skip_desc = undef;
    if (defined $skip && $skip ne '') {
        $skip_desc = { kind => 'regex', text => $skip, key => 'skip:' . unpack('H*', $skip), ast => regex_parse($skip), id => 'skip' };
    }
    return (\@terms, $skip_desc, \%by_symbol);
}

require "$Bin/aurocks-emit.pl";

sub main {
    my $entrypoint;
    my @args;
    while (@ARGV) {
        my $a = shift @ARGV;
        if ($a eq '--entrypoint' || $a eq '-e') {
            $entrypoint = shift @ARGV // die "--entrypoint requires a name\n";
        } elsif ($a eq '--help' || $a eq '-h') {
            print "usage: $0 [--entrypoint NAME] grammar.g\n";
            return;
        } else {
            push @args, $a;
        }
    }
    die "usage: $0 [--entrypoint NAME] grammar.g\n" unless @args == 1;
    my ($path) = @args;
    my ($pro, $epi, $start, $d, $rules, $nonterm) = parse_grammar(slurp($path));
    die "invalid entrypoint name\n"
        if defined($entrypoint) && $entrypoint !~ /^[A-Za-z_]\w*$/;
    emit($pro, $epi, $start, $d, $rules, $nonterm, $entrypoint);
}
main();

#!/bin/awk

BEGIN {
  defun = 0;
  quote = 0;
  indent = 0;
  print_already = 0;
  open_end = 0;
  verbatim = 0;
  prefix = "";
  commentline = 0;
  FS = " "
}

function print_indent () {
  for (j = 0; j < indent; j++) {
    printf (" ");
  }
}

function print_line (start, end) {
  #print "print_line", start, end, $0;
  # print "start: ", start, " end: ", end;
  # print "indent: ", indent, " quote: ", quote;
  for (i = start; i <= end; i++) {

    # check double-quote.
    if (($(i) ~ /".*"/) &&
        (($(i) ~ /^".*[^\\]"/) || ($(i) ~ /[^\\]".*[^\\]"/) ||
         ($(i) ~ /^""/))) {
      # if double-quote is twice.
      quote = 0;
    }  else if (($(i) ~ /"/) &&
                (($(i) ~ /^"/) || ($(i) ~ /[^\\]"/))) {
      # if double-quote is once.
      if (quote == 0) {
        quote = 1;
      } else {
        quote = 0;
      }
    }

    printf "%s", $(i);
    # print "\n"
    # print "debug: defun: ", defun, "i: ", i, "$(i): ", $(i), "quote: ", quote;

    newline_end = 0;
    if (quote == 0) {
      if (($(i) ~ /\/\//) || (commentline == 1)) {
        commentline = 1;
        if (i == end) {
          printf ("\n");
          #print_indent();
          commentline = 0;
	  open_end = 0;
        } else {
          printf (" ");
        }
      } else if ($(i) ~ /\([ ]*$/) {
        if (i == end) {
          open_end = 1;
        }
      } else if ($(i) ~ /\)[^"]*$/) {
        # print "defun close: ", $0;
        if (defun == 1) {
          defun = 0;
        }
        if (i == end) {
          printf ("\n");
        }
      } else {
        printf ("\n");
        newline_end = 1;
        if (i < end) {
          # if ) is inside the string (e.g., "(ipv4)"), indent.
          # if ) is outside the string, it is defun end, so no indent.
          if ($(i) !~ /\)/) {
            #printf ("not closing\n");
            print_indent();
          } else if ($(i) ~ /\).*".*/) {
            #printf ("include close and doubleq\n");
            print_indent();
          }
        }
      }
    }

    if (quote == 1) {
      if ($(i) ~ /\\$/) {
        verbatim = 1;
        # printf ("verbatim on");
        printf ("\n");
      } else {
        printf (" ");
      }
    }
  }
}

/^(DEFUN|DEFPY|ALIAS)/ {
  defun = 1;
  indent = length ($1) + 2;
  #printf ("define_command\n");
  printf $1;
  printf " ";
  print_line(2, NF);
  print_already = 1;
}

/^(#if|#else|#endif)/ {
  if (defun == 1 && newline_end == 0) {
    printf ("\n");
  }
  print $0;
  print_already = 1;
}

{
  if (print_already != 1) {
    #print "default: ", $0;
    if (defun == 1) {
      #printf ("default not already and defun\n");
      if (open_end != 1) {
        #printf ("open_end: %d\n", open_end);
        print_indent();
      } else {
        #printf ("no indent for open_end\n");
        open_end = 0;
      }
      if (verbatim == 1) {
        # print "verbatim!: ", $0;
        # print "gensub:"
        prefix = gensub(/^([ \t]+)[^ \t]*.*$/, "\\1", 1, $0);
        # printf "prefix: %s\n", prefix;
        printf("%s", prefix);
        verbatim = 0;
      }
      print_line(1, NF);
      # print "print_line_after_varbatim: ", $0;
    } else {
      # print "defun: ", defun;
      print $0;
    }
  }
  print_already = 0;
}


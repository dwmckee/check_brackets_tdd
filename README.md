# check_brackets_tdd

A practice problem for learning Test Driven Design (TDD). Probably there
are sample problems out there, but I've thought of one of my own.

## The Project

Write a bracket nesting validator, improving it in stages. 

### Specification

At all stages the program will accept a arbitrary number of files to
process, and will return results to the generic output sink appropriate
to your programming environment.

I'll be writing a traditional command-line program, so that means
accepting filename arguments on the command line and writing messages to
the standard output or standard error. If you work in a different
environment, feel free to make other choices.

#### Stages

1. Works on a fixed set of bracket pairs (`()`, `[]`, and `{}`) and
reports the filename of each file that has an error. The program will be
silent by default on correct files. A report-on-correct switch is
optional.

2. Expand the error report to include the line and column number at
which the error was detected.

3. Expand the error report to include the class of error (close doesn't
match open, close with no open, reached end-of-file with one or more
open brackets). Bad match reports should also include the line and
column number of the non-matching open bracket; end-of-file reports
should include the line and column numbers of the unmatched open
bracket.

4. Allow the user to specify arbitrary character pairs to be treated as
open/close pairs (including canceling or overriding the default pairs).
Attempting to specify a single character more than once represents a
error and suitable diagnostic must be produced.

5. Allow a single character (such as `'` or `"`) to server as both open and
close for a pair; note that these pairs cannot nest without an
intervening scope. That is `'a'b'c'` has two single levels pairs not one
pair inside another, but `'a"b'c'd"e'` is three levels deep.

### Test

The test suite developed along the way should be as comprehensive as
reasonably achievable. It may be necessary to use more than one testing
mechanism to make this happen, which is OK.

# The pf

Forgot what that stands for. pifuck?

## The representation of input

1. All of standard input is read before starting the program.
2. Bijective encoding
3. Translate to base-pi
4. Input is in register 0

## The registers

A register contains a string, initially empty

* Register 0: input register, filled with digits 0..3 (base-pi representation
  of input).
* Register 1: output register, writing to this register will end the program
  and go to the output stage. Contains pseudo-digits 0..3 (only the low 2 bits
  will be considered when converting from base-e to a number to output).
* The program may use any number of auxiliary registers numbered 2 and up.

## The flag

There is one flag bit, set to 1 whenever a replacement is made. It is cleared
by conditional GOTO.

## The program

Each command-line argument to the interpreter is one adjustment in the program,
numbered accordingly (1-based indexing).

* Find/replace adjustment: `I/pat/repl/O`
  Replace `pat` with `repl` everywhere it appears in input register `I`,
  writing the output into register `O`. If a replacement is made, set the flag.

* Unconditional GOTO: `I/` (`I` > 0)
  Jump to adjustment index `I`.

* Conditional GOTO: `-I/` (`I` > 0)
  Jump to adjustment index `I` if the flag is set.

## The representation of output

1. The adjusted output is stored in register 1
2. Translate from base-e
3. Bijective decoding
4. Output to stdout

# The example program

    ./pf 0/0/20birthday21repatriate02draft20200flammulatedThraxerythrean020depth00/1 </dev/null

Or e.g.

    echo | ./pf 0/100/20birthday21repatriate02draft20200flammulatedThraxerythrean020depth00/1

# The constant string conversion program

The example program(s) were generated with the help of the conversion program:

    echo "Hello world!" | ./conv


# The build instructions

Run `make` to build `pf` (the interpreter) and `conv` (the string converter).

Requires libmpfr and libgmp and their development headers.

# The license

This software is licensed under the MIT license. See the LICENSE file.

# About
  

Compiler for the Tiger Functional Language Specification implemented on C language

This Work is based on the specification of the book:

  

©1998 by [Andrew W. Appel](http://www.cs.princeton.edu/~appel) with Maia Ginsburg

Published by Cambridge University Press

([New York](http://www.cup.org/), [Cambridge](http://www.cup.cam.ac.uk/)).

ISBN 0-521-60765-5 (paperback)

  

## Lexical Analysis

  

The lexical analysis is made with **FLEX**.

  

Flex is a tool for generating programs that perform pattern-matching on text. Flex is upward compatible with Lex: all properly-written Lex specifications ought to work with Flex with no change.

Flex is distributed by the [Free Software Foundation](http://www.gnu.org/fsf/fsf.html). Latest versions likely to be available at [these sites](http://www.gnu.org/order/ftp.html).

  

## Parser Generator

  

**Bison** is a general-purpose parser generator that converts a grammar description for an LALR(1) context-free grammar into a C program to parse that grammar. Bison is upward compatible with Yacc: all properly-written Yacc grammars ought to work with Bison with no change.

Bison was written primarily by Robert Corbett; Richard Stallman made it Yacc-compatible. Wilfred Hansen of Carnegie Mellon University added multicharacter string literals and other features.

Bison is distributed by the [Free Software Foundation](http://www.gnu.org/fsf/fsf.html). Latest versions likely to be available at [these sites](http://www.gnu.org/order/ftp.html).

## The Compiler Output

  

This compiler parses the **FUNCTIONAL TIGER** code inputted into **LLVA** instruction code. That can be properly compiled with an **LLVM** compiler.

The LLVM Project is a collection of modular and reusable compiler and toolchain technologies. Despite its name, LLVM has little to do with traditional virtual machines, though it does provide helpful libraries that can be [used to build them](http://vmkit.llvm.org/). The name "LLVM" itself is not an acronym; it is the full name of the project.

LLVM began as a [research project](http://llvm.org/pubs/2004-01-30-CGO-LLVM.html) at the [University of Illinois](http://cs.illinois.edu/), with the goal of providing a modern, SSA-based compilation strategy capable of supporting both static and dynamic compilation of arbitrary programming languages. Since then, LLVM has grown to be an umbrella project consisting of a number of subprojects, many of which are being used in production by a wide variety of [commercial and open source](http://llvm.org/Users.html) projects as well as being widely used in [academic research](http://llvm.org/pubs/). Code in the LLVM project is licensed under the ["UIUC" BSD-Style license](http://llvm.org/docs/DeveloperPolicy.html#license).

## Required softwares installation

  

This implementation of the compiler has only been tested on linux OS. There is no guarantee that the code will work on Windows OS or MAC OS properly.

  

We need three softwares installed for before compiling the code.

  

- GCC 
- FLEX (Version 2.6.0 - Recomended) 
- Bison (Version 3.0.4 - Recomended, but works on past versions) 
- LLVM Compiler

### Instalation Instructions

```{r, engine='sh', count_lines}
sudo apt-get install gcc bison flex llvm
```

### Building the project

to build the project, Simply run:

```{r, engine='sh', count_lines}
make clean && make
```

To clean all the outputs, run:

```{r, engine='sh', count_lines}
make clean
```


### Running the compiler

once that the code has been compiled, you can simply run:

```{r, engine='sh', count_lines}
./a.out <PATH_TO_THE_FILE>/<FILE>.tig
```
For Example:

```{r, engine='sh', count_lines}
./a.out ./home/sum.tig
```
  

If you want the compiler emmit LLVA instruction code, simply run the same command follow by `-llvm-emit`:

```{r, engine='sh', count_lines}
./a.out <PATH_TO_THE_FILE>/<FILE>.tig -llvm-emit
```
For Example:

```{r, engine='sh', count_lines}
./a.out ./home/sum.tig -llvm-emit
```

Whe you run the compiler with this flag, and file called `<INPUT_FILE_NAME>.ll` will be generated, this file is the input for the LLVM compiler.



Every time that you run the program, a file called `<INPUT_FILE_NAME>.arv` will be generated,  this file contains the structure of the grammar tree.

## Examples

On the folder `Tiger` you will find some tiger examples that are showed on the book

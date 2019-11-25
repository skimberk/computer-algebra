Welcome to Sebastian's computer algebra system!
It's pretty rudimentary, but still, I think it's pretty cool.

Table of Contents:
- bigint.c - Implements arbitrary precision integer arithmetic: addition, subtraction, multiplication, division
- fraction.c - Implements rational arithmetic: +, -, *, /, as well as positive integer exponents and integer factorials
- interactive.c - Implements a REPL for rational/integer arithmetic, gives its own instructions on run
- polynomial.c - Implements polynomial addition, subtraction, and multiplication

To play with rational arithmetic:
- compile by running 'clang -g fraction.c interactive.c bigint.c -o interactive'
- run REPL by running './interactive'
- follow on-screen instructions!

To play with polynomial arithmetic:
- edit main method in 'polynomial.c', there's some sample arithmetic there already
  - use createFromStringPolynomial to create a polynomial from space-separated fractions
    these fractions represent the coefficients of the polynomial, from lowest to highest degree
  - use replacePolynomial to replace polynomial with another polynomial (and free memory for polynomial getting replaced)
  - use freePolynomial to free memory for polynomial
  - addPolynomial, subtractPolynomial, and multiplyPolynomial should be self explanatory
- compile by running 'clang -g fraction.c polynomial.c bigint.c -o polynomial'
- execute by running './polynomial'

Also, I did all my compiling and testing on mirage, so ideally compile there!
It'll probably work elsewhere too, but no promises! The only potentially
unportable things I do (which I can think of) are using uint32_t, doing 64 bit
multiplication/division, and using strtok_r. But even those should be pretty portable!

I used the following two books as references for algorithms/general implementation details:

Lamagna, Edmund A. Computer Algebra : Concepts and Techniques . Boca Raton,
Florida: CRC Press, 2019. Print.

Geddes, K. O. (Keith O.), Czapor, S. R. , and Labahn, G. . Algorithms for Computer
Algebra . Boston: Kluwer Academic, 1992. Print.

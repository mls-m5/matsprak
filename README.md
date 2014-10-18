Goals

* Compatibility with c (link to c-libraries)
* Medium fast (fast, but mayby not incredibly fast
** Class member names looked up at compile time (alternative to find it later)
* Beautiful code
 * "and or Integer etc..." alternatives to &&, ||, int
 * Optional semicolons
 * Not forced paranthesis around statements in if statements
 * standard functions loaded on reference (eg. Sin, Cos...)
 * Built in string functions

* Minimal administration
** code should work out of the box, without extra files)
** Classes contained in single files 
* Stand alone code. No interpreter should be nessesary
* Fast parsing
* Easy built ide-tools runnable from command line
 * Auto-completion
 * Renaming of classes, variables etc
 * Function argument hinting
 * Code to code generation for correcting code convention stuff

* Easy debuggable
 * Beautiful informative debug messages
 * Type safe
  * Types resolved at compile time, with non-standard later lookup
* Flexible:
 * Inheritance

Secondary goal
* Easy to convert to faster language for increased performance
* Practical templating functions, that does not increase to much 
* Optional garbage collection that is not obscure or inpractical to use
* Built in beautiful awesome, replacable (qt, gtk, opengl) gui
* Standard replacable ide with auto-completion, renaming and ide-tools
* Built in network functions


Language specification
===========
Comments
--------
C++ style comments

//this is a comment

/*this is not a comment*/

Functions
--------
A example to define a function

function main

end function //The "function part is optional but can be added for
				//Readability and error checking

example 2 - the same function

function main () // The paranthesis is also optional

end


Reserved keywords:
call - call a function
function, sub - define functions
auto - undefined variable type
pointer - pointer
ref - reference
pointer? - pointer
virtual - for inheritated functions
string - string type

...standard c++ types


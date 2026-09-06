# rbtree

this is my attempt at a red-black tree (to use in [TacOS](https://github.com/UnmappedStack/TacOS). It isn't the best code per se but it is decent enough. It uses stuff like tagged/coloured pointers to store the colour information of a node within the same memory as other information in the node which saves on memory usage, and interestingly, by using iterative search instead of recursive search, it was like 30% faster than my implementation with recursive search.

anyways this is really nothing particularly unique and probably not the best thing to learn from so unless you're here to criticise or improve it I don't recommend reading it lol.

to build it just do `cc main.c -o out` (if you're debugging then I also use `-g -fsanitize=undefined -Wall -Werror`) then:
    - To test and time it for a bunch of insertions and searches etc do `time ./out`
    - To test it with whether or not balancing is turned on modify the `DO_BALANCE` macro in `main.c`

I don't intend to do a proper build system or anything because this is just a quick thing which will be later merged into TacOS separately.

this is under the mozilla public licence 2.0, see the LICENSE file.

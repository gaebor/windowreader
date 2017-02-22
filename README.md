# Window Reader
This small `C++` program reads a text file and chops it into windows.
For example a window of 5 words is

    The Turboliners were [ a family of gas turbine ] trains built for Amtrak in the 1970s.

Newlines and pagebreaks indicates the end of a sentence.
Space and tab indicate the word boundaries. A window never spans over multiple sentences.
In the output, there is one window per line.

If a vocabulary is given then the windowreader outputs word indices rather than the words themselves.

You can specify windows with left span and right span.
The following window spans 2 to left and two to right: a central windows of width 2.

    The Turboliners were [ a family of gas turbine ] trains built for Amtrak in the 1970s.
                           -2  -1    0  1     2

For example a window with 3 to left and 0 to right:

    The Turboliners [ were a family of ] gas turbine trains built for Amtrak in the 1970s.
                        -3 -2  -1    0  

The program reads the text from `stdin` and writes the reformatted text to `stdout` (default).
See `--help` for more details.

## Performance
This tool uses O(1) memory (constant in input size) and is roughly 5-10 times slower than a [wc](https://linux.die.net/man/1/wc).

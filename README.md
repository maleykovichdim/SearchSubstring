# SearchSubstring

# Task

Write a program `mtfind` that performs substring search in a text file based on a mask using multithreading. The mask is a string where "?" represents any character.

## Program Requirements

The program takes the following command line parameters:

1. The name of the text file in which the search should be performed (file size up to 1 GB).
2. The mask for searching, in quotes. The maximum length of the mask is 1000 characters.

### Output Format

The output of the program should be in the following format:

- On the first line: the number of found occurrences.
- Each subsequent line contains information about each occurrence, separated by spaces: line number, position in the line, and the found substring itself.
- The order of output for found occurrences must match their order in the file.
- All numbering starts from 1 (for user-friendliness).

### Additional Considerations

- The text file must be encoded in 7-bit ASCII.
- The search is case-sensitive.
- Each occurrence can only appear on one line. The mask cannot contain newline characters.
- Found occurrences must not overlap. If there are overlapping occurrences in the file, any one of them should be outputted.
- Spaces and delimiters are considered in the search just like other characters.
- You can use STL, Boost, and features from C++11, C++14, and later versions.
- Multithreading must be used. Single-threaded solutions will not be accepted.
- A significant advantage will be evenly distributing work among threads, regardless of the number of lines in the input file.

## Submission Guidelines

The solution should be submitted as an archive with source code and a CMake or Visual Studio project, or as a link to an online Git repository. The code should compile with GCC or MSVC.

## Example

Input file `input.txt`:

I've paid my dues
Time after time.
I've done my sentence
But committed no crime.
And bad mistakes ?
I've made a few.
I've had my share of sand kicked in my face
But I've come through.
text


Program execution: 

mtfind input.txt "?ad"
text


Expected output:

3
5 5 bad
6 6 mad
7 6 had
text


## Evaluation Criteria

The evaluation of the solution will be based on:

- Correctness of the provided results.
- Quality and readability of the code, ease of further development and maintenance.
- Speed of execution and memory consumption.

All criteria are equally important.
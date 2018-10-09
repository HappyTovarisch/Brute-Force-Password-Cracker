Jake Bernstein
10/9/2018
Brute-Force password cracker

This is a simple brute-force password cracker program designed to discover passwords consisting of characters a-z using input hashes.
The program runs through a list of all possible hashes until it finds one that matches the input hash. If the cracker reaches 
'zzzzzzzz' without finding a match, it will print an error message and quit.

Usage:
./crack <Threads> <KeyLength> <Hash>
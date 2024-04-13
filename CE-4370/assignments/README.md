## Design using Kicad PCB tool

1) Please run the tutorial found at  https://docs.kicad.org/8.0/en/getting_started_in_kicad/getting_started_in_kicad.html   

2) You are to design two circuits in Kicad.    You must pick two out of the four choices.    You need to create a schematic and an PCB.    Your design needs to use real available parts.    Your first priority is a working circuit and then complexity and cost.

A) Design a circuit that detects when a LED connected to a 12V power supply is on.    The output should safely be able to be connected to any
3.3V input pin of the ESP32 without any chance of overvoltage or overcurrent (even in case of a short of the LED).    Extra points if you can handle non-common grounds.
B) Design a switch debouncing circuit.
C) Design a USB-UART debugging interface for the ESP32.
D) Design an interface to control a digital potentiometer using the SPI protocol.
Please upload your schematic file (*.kicad_sch) and your PCB file (*.kicad_pcb) files.

## Endianness and NoMachine

On the ESP32 devices, and ZERO write the following programs:
1) Write a program to show the endianness of a machine
    Use a union to figure this out:
        union {
            int value ;
            unsigned char bytes[4] ;
        }
        Hint: load the union's bytes and read out the resulting value to determine the endianness.   Using a type definitions is very useful too here.
2) Write a program to find the relative time of the following operators:  integer +, integer -, integer *, integer /, integer %, float +, float -, float *, and float /.


Integer Arithmetic Operator times:

| Operator       | Relative Time (usec) |
| -------------- | -------------------- |
| Addition       | 0000.00              |
| Subtraction    | 0000.00              |
| Multiplication | 0000.00              |
| Division       | 0000.00              | 

Floating Point Arithmetic Operator times:

| Operator       | Relative Time (usec) |
| -------------- | -------------------- |
| Addition       | 0000.00              |
| Subtraction    | 0000.00              |
| Multiplication | 0000.00              |
| Division       | 0000.00              | 


On NoMachine write the following programs:
1) Write a program to count the lines in a file.    An example file is attached.  The program will take one argument - the name of the file to be opened and read.



2) Average the numbers found in the attached file



3) Find the max, min, variance, and standard deviation.



4) Write a program to show the endianness of a machine



5) Write a recursive program that causes the program to crash.   How calls does it take before it crashes?




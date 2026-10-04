**Name:** Abdul Kareem Aftab  
**Roll Number:** BSCS25082  

27 sept

i added header files and global variables,
then added stack class with functions like pop etc, and snapshot copying functions

28 sept

added timeline node class and timeline struct for its class
and then implemented structs of frame, variable, snapshot and TTDBHeader
and implementation of snapshot in the doubly linked timeline

29 sept

implemented functions
1. write_bytes
2. read_bytes
3. file_position
4. seek_file
5. close_file
6. write_header
   
and added functionEntry, pendingPatch, and token struct

30 sept

implemented functions 
1. read_source_line
2. first_word
3. second_word
4. is_identifier

and completed validate_program for checking function boundaries and rejecting nested declarations.

1 oct

implemented functions
1. write_resolve_record
2. read_resolve_record
3. resolve_program to generate resolve.bin

and resolve function calls, and locate main.

2 oct

implemented functions
1. tokenize_line
2. find_variable
3. read_value
4. make_frame
5. build_snapshot.

and completed excute_program for executing instructions and recording program states.

3 oct

implemented functions
1. write_string
2. write_variable
3. write_time_travel_debugger
   
connected the four major functions through main. 
fixed identifier validation
andconfirmed 11/11 tests passed.





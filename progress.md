**Name:** Abdul Kareem Aftab  
**Roll Number:** BSCS25082  

we first added header files given in server.cpp, then we decleared the global functions.
then we implemented the stack class using template

after that we used struct of time line, to make time line node class
then some other structs for helping in program, mentioned below : 
1. timeline
2. snapshot
3. variable
4. frame
5. TTDBHeader

after that we make some changes in function of timeline class.
then using previous classes and some new logics we made functions required : 
1. void write_bytes(FILE *file, const void *dataNode, size_t size)
2. void read_bytes(FILE *file, void *dataNode, size_t size)
3. int64_t file_position(FILE *file)
4. void seek_file(FILE *file, int64_t position)
5. void close_file(FILE *&file)
6. void write_header(FILE *file, const TTDBHeader &header)

then we added few more structs
1. functionentry
2. pendingpatch
3. token

after that few more functions,
1. read_source_line
2. first_word
3. second_word
4. is_identifier
5. validate_program
6. write_resolve_record
7. read_resolve_record
8. resolve_program
9. tokenize_line
10. find_variable
11. read_value
12. make_frame
13. build_snapshot
14. excute_program
15. write_string
16. write_variable
17. write_time_travel_debugger

from these functions,
1. bool validate_program(const char *sourcePath)
2. int64_t resolve_program(const char *sourcePath, const char *resolveBinPath)
3. void excute_program(const char *resolveBinPath, int64_t mainOffset, timeLine &timelineNode)
4. void write_time_travel_debugger(timeLine &timeline, const char *timeTravelDebuggerPath)
these can be called big four as these functions hold every major logic of our program.
these functions alone hold major part of our proram

then we implemented our main functions
it uses our big four

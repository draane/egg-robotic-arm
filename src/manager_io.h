#ifndef MANAGER_IO
#define MANGER_IO

// two lists of integers are passed: 
// These are the 8 pins of input and output defined in files, if speciied the option -if, -of in command line.
void manager_io(int*, int*);


unsigned char make_one_byte_from_string(char* );
#endif

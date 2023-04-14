typedef struct Program {
    int process_pid;
    char* program_name;
    int argc;
    char argv[64][64];
} Program;

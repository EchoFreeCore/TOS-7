static const char* dummy_errors[] = {
    "Success", "Unknown error", "System failure"
};
char* strerror(int errnum) {
    extern char* sys_errlist[];
    extern int sys_nerr;
    if (errnum >= 0 && errnum < sys_nerr) {
        return sys_errlist[errnum];
    }
    return "Unknown error";
}
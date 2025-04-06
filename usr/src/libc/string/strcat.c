char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d != '\0') {
        d++;
    }
    while ((*d++ = *src++) != '\0');
    return dest;
}
size_t strcspn(const char* s, const char* reject) {
    size_t len = 0;
    while (*s && !strchr(reject, *s)) {
        len++;
        s++;
    }
    return len;
}
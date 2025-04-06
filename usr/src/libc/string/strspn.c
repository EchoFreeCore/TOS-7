size_t strspn(const char* s, const char* accept) {
    size_t len = 0;
    while (*s && strchr(accept, *s)) {
        len++;
        s++;
    }
    return len;
}
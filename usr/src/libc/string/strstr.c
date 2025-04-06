char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*)haystack;
    }
    for (const char* h = haystack; *h; h++) {
        const char* htmp = h;
        const char* n = needle;
        while (*htmp && *n && (*htmp == *n)) {
            htmp++;
            n++;
        }
        if (*n == '\0') {
            return (char*)h;
        }
    }
    return NULL;
}

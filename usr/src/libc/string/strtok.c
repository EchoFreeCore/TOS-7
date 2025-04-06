char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    if (str == NULL) {
        str = last;
    }
    if (str == NULL) {
        return NULL;
    }

    str += strspn(str, delim);
    if (*str == '\0') {
        last = NULL;
        return NULL;
    }

    char* token = str;
    str = strpbrk(token, delim);
    if (str) {
        *str = '\0';
        last = str + 1;
    }
    else {
        last = NULL;
    }
    return token;
}
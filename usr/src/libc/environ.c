#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>

#define ENV_GROWTH 8
static size_t env_capacity = 0;
char **environ = NULL;

char *getenv(const char *name) {
    if (!name || !environ)
        return NULL;

    size_t len = strlen(name);
    for (char **e = environ; *e; ++e) {
        if (!strncmp(*e, name, len) && (*e)[len] == '=') {
            return *e + len + 1;
        }
    }
    return NULL;
}

int putenv(char *string) {
    if (!string || !strchr(string, '='))
        return -1;

    char *eq = strchr(string, '=');
    size_t namelen = eq - string;

    // ta bort befintlig nyckel om den finns
    for (size_t i = 0; environ && environ[i]; ++i) {
        if (!strncmp(environ[i], string, namelen) && environ[i][namelen] == '=') {
            environ[i] = string;
            return 0;
        }
    }

    if (!environ) {
        environ = calloc(ENV_GROWTH, sizeof(char *));
        env_capacity = ENV_GROWTH;
    }

    size_t i = 0;
    while (environ[i]) i++;

    if (i + 1 >= env_capacity) {
        env_capacity += ENV_GROWTH;
        environ = realloc(environ, env_capacity * sizeof(char *));
    }

    environ[i] = string;
    environ[i + 1] = NULL;
    return 0;
}


int setenv(const char *name, const char *value, int overwrite) {
    if (!name || !*name || strchr(name, '='))
        return -1;

    size_t namelen = strlen(name);
    size_t vallen = strlen(value);
    size_t i;

    if (!environ) {
        environ = calloc(ENV_GROWTH, sizeof(char *));
        env_capacity = ENV_GROWTH;
    }

    for (i = 0; environ[i]; ++i) {
        if (!strncmp(environ[i], name, namelen) && environ[i][namelen] == '=') {
            if (!overwrite)
                return 0;
            free(environ[i]);
            break;
        }
    }

    if (i + 1 >= env_capacity) {
        env_capacity += ENV_GROWTH;
        environ = realloc(environ, env_capacity * sizeof(char *));
    }

    char *entry = malloc(namelen + vallen + 2);
    sprintf(entry, "%s=%s", name, value);
    environ[i] = entry;
    environ[i + 1] = NULL;
    return 0;
}

int unsetenv(const char *name) {
    if (!name || !*name || strchr(name, '=') || !environ)
        return -1;

    size_t namelen = strlen(name);
    for (size_t i = 0; environ[i]; ++i) {
        if (!strncmp(environ[i], name, namelen) && environ[i][namelen] == '=') {
            free(environ[i]);
            // flytta alla efterföljande
            for (size_t j = i; environ[j]; ++j) {
                environ[j] = environ[j + 1];
            }
            return 0;
        }
    }
    return 0;
}

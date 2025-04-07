/* stdio.c - Minimal stdio implementation for TOS-7 (UNIX-style, circa 1992)
 * POSIX.1-1988 / ANSI C89 compatible
 */

 #include <unistd.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <string.h>
 #include <stdarg.h>
 #include <stdio.h>
 
 #define MAX_FILES 256
 
 /* File table and standard streams */
 static FILE file_table[MAX_FILES];
 
 FILE* stdin  = &file_table[0];
 FILE* stdout = &file_table[1];
 FILE* stderr = &file_table[2];
 
 __attribute__((constructor))
 static void init_stdio_files() {
     for (int i = 0; i < MAX_FILES; ++i)
         file_table[i].fd = -1;
 }

 /* Open mode mapping */
 static int mode_to_flags(const char* mode) {
     if (strcmp(mode, "r") == 0) return O_RDONLY;
     if (strcmp(mode, "w") == 0) return O_WRONLY | O_CREAT | O_TRUNC;
     if (strcmp(mode, "a") == 0) return O_WRONLY | O_CREAT | O_APPEND;
     return -1;
 }
 
 FILE* fopen(const char* path, const char* mode) {
     int flags = mode_to_flags(mode);
     if (flags < 0) return NULL;
 
     int fd = open(path, flags, 0666);
     if (fd < 0) return NULL;
 
     for (int i = 3; i < MAX_FILES; ++i) {
         if (file_table[i].fd == -1) {
             FILE* f = &file_table[i];
             f->fd = fd;
             f->flags = (flags == O_RDONLY) ? _F_READ : _F_WRITE;
             f->bufpos = 0;
             f->buflen = 0;
             f->last_op = 0;
             f->error = 0;
             f->eof = 0;
             f->unget = -1;
             f->bufmode = _IOFBF;
             f->bufuser = 0;
             f->bufsize = BUFSIZ;
             f->buffer = f->internal;
             return f;
         }
     }
 
     close(fd);
     return NULL;
 }
 
 int fclose(FILE* f) {
     if (!f || f->fd < 0) return EOF;
     fflush(f);
     int res = close(f->fd);
     f->fd = -1;
     if (f->bufuser && f->buffer) f->buffer = NULL;
     return res;
 }
 
 int fflush(FILE* f) {
     if (!f) return EOF;
     if (f->last_op == 2 && f->bufpos > 0) {
         ssize_t res = write(f->fd, f->buffer, f->bufpos);
         if (res < 0) {
             f->error = 1;
             return EOF;
         }
         f->bufpos = 0;
     }
     return 0;
 }
 
 int fputc(int c, FILE* f) {
     if (!f || !(f->flags & _F_WRITE)) return EOF;
     if (f->last_op == 1) {
         f->bufpos = 0;
         f->buflen = 0;
     }
     f->last_op = 2;
     f->buffer[f->bufpos++] = (char)c;
     if (f->bufpos >= f->bufsize || c == '\n') return fflush(f);
     return (unsigned char)c;
 }
 
 int fgetc(FILE* f) {
     if (!f || !(f->flags & _F_READ)) return EOF;
 
     if (f->unget != -1) {
         int c = f->unget;
         f->unget = -1;
         return c;
     }
 
     if (f->last_op == 2) {
         fflush(f);
         f->bufpos = 0;
         f->buflen = 0;
     }
     f->last_op = 1;
 
     if (f->bufpos >= f->buflen) {
         ssize_t res = read(f->fd, f->buffer, f->bufsize);
         if (res < 0) {
             f->error = 1;
             return EOF;
         }
         if (res == 0) {
             f->eof = 1;
             return EOF;
         }
         f->bufpos = 0;
         f->buflen = (size_t)res;
     }
     return (unsigned char)f->buffer[f->bufpos++];
 }
 
 int fwrite(const void* ptr, size_t size, size_t nmemb, FILE* f) {
     const char* p = (const char*)ptr;
     size_t total = size * nmemb;
     size_t written = 0;
 
     for (size_t i = 0; i < total; ++i) {
         if (fputc(p[i], f) == EOF) break;
         ++written;
     }
 
     return written / size;
 }
 
 int fread(void* ptr, size_t size, size_t nmemb, FILE* f) {
     char* p = (char*)ptr;
     size_t total = size * nmemb;
     size_t readn = 0;
 
     for (size_t i = 0; i < total; ++i) {
         int ch = fgetc(f);
         if (ch == EOF) break;
         p[i] = (char)ch;
         ++readn;
     }
 
     return readn / size;
 }
 
 int ungetc(int c, FILE* f) {
     if (!f || f->unget != -1) return EOF;
     f->unget = c;
     return c;
 }
 
 int setvbuf(FILE* f, char* buf, int mode, size_t size) {
     if (!f || mode < 0 || mode > 2) return -1;
     fflush(f);
     f->bufmode = mode;
     if (buf) {
         f->buffer = buf;
         f->bufsize = size;
         f->bufuser = 1;
     } else {
         f->buffer = f->internal;
         f->bufsize = BUFSIZ;
         f->bufuser = 0;
     }
     return 0;
 }
 
 void setbuf(FILE* f, char* buf) {
     setvbuf(f, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
 }
 
 int fputs(const char* s, FILE* f) {
     while (*s) {
         if (fputc(*s++, f) == EOF) return EOF;
     }
     return 0;
 }
 
 char* fgets(char* s, int size, FILE* f) {
     if (!s || size <= 0) return NULL;
     char* p = s;
     for (int i = 0; i < size - 1; ++i) {
         int ch = fgetc(f);
         if (ch == EOF) break;
         *p++ = ch;
         if (ch == '\n') break;
     }
     *p = '\0';
     return (p == s) ? NULL : s;
 }
 
 int printf(const char* fmt, ...) {
     char tmp[1024];
     va_list ap;
     va_start(ap, fmt);
     int len = vsnprintf(tmp, sizeof(tmp), fmt, ap);
     va_end(ap);
     fwrite(tmp, 1, len, stdout);
     return len;
 }
 
 int fprintf(FILE* f, const char* fmt, ...) {
     char tmp[1024];
     va_list ap;
     va_start(ap, fmt);
     int len = vsnprintf(tmp, sizeof(tmp), fmt, ap);
     va_end(ap);
     fwrite(tmp, 1, len, f);
     return len;
 }
 
 int fseek(FILE* f, long offset, int whence) {
     if (!f) return -1;
     fflush(f);
     f->bufpos = 0;
     f->buflen = 0;
     f->unget = -1;
     f->eof = 0;
     off_t res = lseek(f->fd, offset, whence);
     return (res == (off_t)-1) ? -1 : 0;
 }
 
 long ftell(FILE* f) {
     if (!f) return -1;
     off_t base = lseek(f->fd, 0, SEEK_CUR);
     if (base == (off_t)-1) return -1;
     return base - (f->buflen - f->bufpos);
 }
 
 void rewind(FILE* f) {
     fseek(f, 0L, SEEK_SET);
     clearerr(f);
 }
 
 int feof(FILE* f) {
     return f ? f->eof : 0;
 }
 
 int ferror(FILE* f) {
     return f ? f->error : 0;
 }
 
 void clearerr(FILE* f) {
     if (f) {
         f->error = 0;
         f->eof = 0;
     }
 }
 
 int getc(FILE* f) {
     return fgetc(f);
 }
 
 int putc(int c, FILE* f) {
     return fputc(c, f);
 }
 
 int getchar(void) {
     return getc(stdin);
 }
 
 int putchar(int c) {
     return putc(c, stdout);
 }
 
 int sscanf(const char* str, const char* format, ...) {
     va_list ap;
     int result;
     va_start(ap, format);
     result = vsscanf(str, format, ap);
     va_end(ap);
     return result;
 }
 
 int scanf(const char* format, ...) {
     va_list ap;
     int result;
     va_start(ap, format);
     result = vfscanf(stdin, format, ap);
     va_end(ap);
     return result;
 }

 
 

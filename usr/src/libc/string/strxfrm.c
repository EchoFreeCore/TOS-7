
size_t strxfrm(char* dest, const char* src, size_t n) {
	size_t i = 0;
	while (src[i]) i++; // räkna total längd
	if (n > 0) {
		size_t j = 0;
		while (j < n - 1 && src[j]) {
			dest[j] = src[j];
			j++;
		}
		dest[j] = '\0';
	}
	return i;
}
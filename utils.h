u_int16_t compute_icmp_checksum(const void *buff, int length);

void ERROR(const char* str);

void print_as_bytes (unsigned char* buff, ssize_t length);

long long get_current_time_ms();
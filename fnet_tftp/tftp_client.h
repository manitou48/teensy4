// TFTP client
void tftp_open(IPAddress server, char * filename, int write_flag);
int tftp_read(uint8_t *buff);
int tftp_write(uint8_t *buff, int bytes);

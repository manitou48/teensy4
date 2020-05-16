// TFTP context containing callback functions for TFTP transfers
struct tftp_context {
  void* (*open)(const char* fname, const char* mode, uint8_t write);
  void (*close)(void* handle);
  int (*read)(void* handle, void* buf, int bytes);
  int (*write)(void* handle, void* buf, int bytes);
};

void tftp_init(const struct tftp_context* ctx);

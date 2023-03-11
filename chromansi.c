const unsigned char chromansi[] = { \
 0x1B, 0x5B, 0x30, 0x6D, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x35, 0x6D, 0x24, 0x24, 0x49, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x69, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x1B, \
 0x5B, 0x30, 0x6D, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2E, \
 0x2C, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3A, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, \
 0x3B, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x31, \
 0x3B, 0x33, 0x30, 0x6D, 0x69, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x69, 0x69, \
 0x69, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x24, 0x24, 0x24, 0x24, 0x24, \
 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, \
 0x24, 0x24, 0x24, 0x2A, 0x27, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x20, 0x20, 0x1B, 0x5B, \
 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x27, 0x3A, 0x3A, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x69, 0x69, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x49, 0x49, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x24, 0x24, 0x0A, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x49, 0x49, 0x49, 0x69, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, \
 0x69, 0x2A, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x27, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x5F, \
 0x2C, 0x5F, 0x2C, 0x2E, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x60, 0x27, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3A, 0x3B, 0x3B, \
 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, \
 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x3B, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, \
 0x69, 0x69, 0x49, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x49, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x50, 0x27, 0x60, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x36, 0x6D, 0x5F, 0x2C, 0x76, 0x1B, 0x5B, 0x31, 0x6D, 0x6F, 0x23, 0x6C, 0x24, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x62, 0x2E, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x3A, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, \
 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, \
 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x6D, 0x69, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x49, \
 0x49, 0x49, 0x49, 0x24, 0x24, 0x24, 0x0A, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, \
 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x27, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x2C, 0x64, 0x7C, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x31, 0x6D, \
 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x31, 0x6D, \
 0x23, 0x73, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x3D, 0x2C, 0x5F, 0x1B, 0x5B, 0x33, 0x35, \
 0x6D, 0x60, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x27, 0x22, 0x3A, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x22, 0x27, 0x60, 0x27, 0x1B, \
 0x5B, 0x31, 0x6D, 0x22, 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, \
 0x6D, 0x49, 0x24, 0x24, 0x27, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x2E, 0x64, 0x24, 0x1B, \
 0x5B, 0x31, 0x6D, 0x37, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x22, 0x22, 0x5E, 0x60, \
 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x62, 0x2E, \
 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x3A, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x3B, 0x3B, 0x3B, 0x27, 0x1B, 0x5B, 0x31, 0x6D, 0x27, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x60, 0x60, 0x27, 0x1B, 0x5B, 0x31, 0x6D, 0x69, 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x24, 0x0A, 0x69, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x27, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x36, 0x6D, 0x2C, 0x64, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x36, 0x6D, 0x24, 0x50, 0x2A, 0x22, 0x60, 0x27, 0x22, 0x5E, 0x6F, 0x24, 0x7C, 0x24, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x23, 0x3D, 0x2C, \
 0x2E, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x60, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x3B, 0x69, 0x69, 0x27, 0x1B, 0x5B, 0x33, 0x36, 0x6D, \
 0x2E, 0x64, 0x6C, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x6F, 0x3D, 0x2E, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x36, 0x6D, 0x5F, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x35, 0x6D, 0x60, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x27, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x2E, 0x23, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, \
 0x5E, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x60, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x2C, 0x49, \
 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x2E, 0x1B, 0x5B, 0x30, 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, \
 0x3A, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, \
 0x1B, 0x5B, 0x33, 0x37, 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x20, 0x5F, 0x2E, 0x2C, 0x1B, \
 0x5B, 0x31, 0x6D, 0x5F, 0x79, 0x76, 0x6F, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x24, \
 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x6F, 0x1B, \
 0x5B, 0x31, 0x6D, 0x3D, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x2E, 0x1B, 0x5B, 0x33, 0x35, \
 0x6D, 0x60, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x0A, 0x1B, 0x5B, 0x33, 0x30, 0x6D, 0x3A, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x27, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x2E, 0x24, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x33, \
 0x35, 0x6D, 0x2C, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x69, 0x49, 0x62, 0x1B, 0x5B, 0x30, 0x6D, \
 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x2C, 0x60, 0x27, 0x2A, 0x6F, 0x34, 0x1B, 0x5B, 0x31, 0x6D, \
 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x33, 0x35, 0x6D, \
 0x2E, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x6D, 0x20, \
 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x36, 0x6D, 0x24, 0x60, 0x5E, 0x22, 0x1B, 0x5B, 0x31, 0x6D, 0x22, 0x37, 0x24, 0x23, 0x24, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x23, 0x27, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x20, \
 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x24, 0x62, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x20, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, \
 0x64, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x2A, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x27, 0x60, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x5F, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x2E, 0x2E, 0x1B, 0x5B, 0x31, 0x6D, 0x2C, 0x2C, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x2E, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x60, 0x1B, 0x5B, 0x31, 0x6D, 0x34, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x36, 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x3A, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x60, 0x1B, 0x5B, 0x31, 0x6D, 0x27, 0x0A, 0x1B, 0x5B, 0x30, \
 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x2E, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2C, \
 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x69, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x24, 0x1B, 0x5B, \
 0x30, 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x24, 0x24, 0x24, 0x24, 0x76, 0x6F, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x24, 0x23, 0x73, 0x3D, 0x2C, 0x5F, 0x1B, 0x5B, 0x33, 0x30, 0x6D, 0x60, \
 0x27, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x3A, 0x1B, \
 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x3A, 0x1B, 0x5B, \
 0x33, 0x37, 0x6D, 0x20, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x2C, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x2E, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x60, 0x5E, 0x24, 0x24, 0x6F, 0x3D, \
 0x2E, 0x5F, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x60, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x27, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x20, 0x6C, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x24, \
 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x20, \
 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3A, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x69, 0x49, 0x49, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x2C, 0x6C, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x6C, 0x0A, 0x1B, 0x5B, 0x33, 0x37, 0x6D, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x2E, 0x24, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x33, \
 0x35, 0x6D, 0x2C, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x69, 0x49, 0x49, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x24, 0x1B, 0x5B, 0x33, 0x37, 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, \
 0x60, 0x24, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, \
 0x2E, 0x60, 0x6F, 0x24, 0x7C, 0x1B, 0x5B, 0x31, 0x6D, 0x7C, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x24, 0x62, 0x5F, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x36, 0x6D, 0x2E, 0x60, 0x34, 0x24, 0x6C, 0x1B, 0x5B, 0x33, 0x37, 0x6D, 0x20, 0x1B, 0x5B, 0x31, \
 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, \
 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x5F, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x2C, \
 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2E, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x5F, \
 0x1B, 0x5B, 0x31, 0x6D, 0x5F, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x2E, 0x2C, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x20, 0x1B, 0x5B, 0x33, \
 0x36, 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x37, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x62, 0x1B, 0x5B, 0x31, 0x6D, 0x2C, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x5F, 0x1B, 0x5B, \
 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x5F, 0x2E, 0x1B, \
 0x5B, 0x31, 0x6D, 0x2C, 0x5F, 0x64, 0x24, 0x7C, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, \
 0x2A, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x35, 0x6D, 0x2E, 0x24, 0x0A, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x36, 0x6D, 0x3A, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2C, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x69, 0x69, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x27, 0x60, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x5F, 0x5F, 0x60, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x2E, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x20, 0x1B, 0x5B, \
 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2C, 0x2C, 0x2E, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x60, \
 0x27, 0x5C, 0x6F, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x7C, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x24, 0x3D, 0x2E, 0x20, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2C, 0x3B, 0x3B, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, \
 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x69, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x49, 0x24, \
 0x62, 0x2C, 0x2E, 0x2E, 0x20, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x60, 0x24, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x24, 0x24, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x2A, 0x22, 0x27, 0x1B, 0x5B, 0x31, \
 0x6D, 0x22, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x60, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x5F, \
 0x2C, 0x5F, 0x1B, 0x5B, 0x31, 0x6D, 0x69, 0x49, 0x24, 0x24, 0x0A, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x36, 0x6D, 0x60, 0x34, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x62, 0x2E, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x49, 0x27, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x36, 0x6D, 0x2E, 0x24, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x36, 0x6D, 0x60, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x24, 0x2E, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x35, 0x6D, 0x60, 0x24, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x60, 0x1B, 0x5B, 0x33, \
 0x36, 0x6D, 0x5F, 0x2C, 0x76, 0x73, 0x23, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x6D, 0x20, 0x1B, \
 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x3B, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x69, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x69, 0x69, 0x69, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x49, 0x49, 0x49, 0x24, 0x24, 0x2E, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x60, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x6C, 0x27, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, \
 0x2C, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3A, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, \
 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x69, 0x69, 0x1B, 0x5B, 0x31, \
 0x6D, 0x49, 0x24, 0x24, 0x24, 0x0A, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x6C, 0x1B, 0x5B, \
 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2E, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x60, 0x27, 0x34, \
 0x7C, 0x7C, 0x24, 0x23, 0x5F, 0x2C, 0x5F, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x35, 0x6D, 0x60, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x2C, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x24, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x27, 0x1B, 0x5B, 0x33, 0x37, 0x6D, 0x20, 0x1B, 0x5B, 0x33, \
 0x36, 0x6D, 0x60, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, \
 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x2C, 0x3D, 0x23, 0x24, 0x24, 0x24, 0x24, 0x24, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x7C, 0x24, 0x6F, 0x2A, 0x27, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x2C, \
 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, \
 0x3B, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x69, 0x69, 0x69, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, \
 0x35, 0x6D, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, \
 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x6C, 0x24, 0x24, 0x37, 0x3A, 0x1B, 0x5B, 0x31, 0x3B, \
 0x33, 0x30, 0x6D, 0x20, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, \
 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x69, 0x49, 0x49, \
 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, \
 0x31, 0x6D, 0x24, 0x24, 0x0A, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x24, 0x24, 0x1B, \
 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x62, 0x2C, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x2E, \
 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x60, 0x27, 0x2A, 0x6F, 0x24, 0x7C, 0x24, 0x1B, 0x5B, 0x31, 0x6D, \
 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x27, 0x1B, 0x5B, 0x33, 0x37, \
 0x6D, 0x20, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x20, 0x60, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, \
 0x24, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x24, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x7C, 0x24, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x36, 0x6D, 0x6F, 0x2A, 0x27, 0x60, 0x1B, 0x5B, 0x33, 0x35, 0x6D, \
 0x2E, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2C, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, 0x3B, \
 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x3B, 0x3B, 0x3B, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x69, 0x1B, \
 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x69, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x49, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, \
 0x49, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x24, 0x24, \
 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x24, 0x1B, 0x5B, 0x31, 0x6D, 0x24, 0x24, 0x1B, 0x5B, \
 0x30, 0x6D, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, 0x3A, 0x24, 0x24, 0x24, 0x1B, 0x5B, 0x33, 0x37, \
 0x6D, 0x20, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x2C, 0x3B, 0x3B, 0x3B, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x3B, 0x1B, 0x5B, 0x31, 0x3B, 0x33, 0x30, 0x6D, 0x3B, 0x1B, 0x5B, 0x30, \
 0x3B, 0x33, 0x35, 0x6D, 0x69, 0x69, 0x69, 0x49, 0x49, 0x1B, 0x5B, 0x31, 0x6D, 0x49, 0x1B, 0x5B, \
 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x24, 0x0A, 0x1B, 0x5B, 0x31, 0x6D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, \
 0x2D, 0x2D, 0x2D, 0x2D, 0x1B, 0x5B, 0x30, 0x6D, 0x20, 0x20, 0x20, 0x1B, 0x5B, 0x33, 0x36, 0x6D, \
 0x60, 0x22, 0x2A, 0x22, 0x60, 0x1B, 0x5B, 0x33, 0x37, 0x6D, 0x20, 0x1B, 0x5B, 0x31, 0x3B, 0x33, \
 0x35, 0x6D, 0x2D, 0x2D, 0x2D, 0x1B, 0x5B, 0x30, 0x3B, 0x33, 0x35, 0x6D, 0x20, 0x1B, 0x5B, 0x33, \
 0x36, 0x6D, 0x60, 0x34, 0x6F, 0x2A, 0x27, 0x20, 0x1B, 0x5B, 0x33, 0x35, 0x6D, 0x20, 0x20, 0x1B, \
 0x5B, 0x31, 0x6D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, \
 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, \
 0x2D, 0x2D, 0x2D, 0x1B, 0x5B, 0x30, 0x6D, 0x20, 0x20, 0x20, 0x20, 0x1B, 0x5B, 0x31, 0x3B, 0x33, \
 0x35, 0x6D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, \
 0x1B, 0x5B, 0x30, 0x6D, 0x0A, 0x00 };

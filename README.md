AERIS Bootloader library

IN PROGRESS

• User/Client sends binary in 1024-byte chunks OTA with UDP (Faster than TCP)
• Local device (with wifi) repackages binary into custom DFU messaging schema
• Flashes new binary using UART bootloader library!
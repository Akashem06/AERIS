AERIS Bootloader library

IN PROGRESS

• User/Client sends binary in 1024-byte chunks OTA with UDP (Faster than TCP)<br>
• Local device (with wifi) repackages binary into custom DFU messaging schema<br>
• Flashes new binary using UART bootloader library!<br>

USE:<br>
make all    -> Build binary
make clean  -> Delete binary
make format -> Format all files
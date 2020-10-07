# simple_uart_example

NS16550 uart driver implementation for QEMU Malta

The example works in pooling mode only.

QEMU run example (with gdb support):

> qemu-system-mips64 -M malta -nographic -kernel uart -s -S -m 128M -icount auto -singlestep

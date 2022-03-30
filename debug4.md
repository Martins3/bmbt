## 第一版

```txt
PCI: init bdf=00:02.0 id=8086:10d3
PCI: No VGA devices found
cmos: read index=0x08 val=0x01
cmos: read index=0x5f val=0x00
cmos: read index=0x08 val=0x01
cmos: read index=0x5f val=0x00
cmos: read index=0x00 val=0x00
Found 1 cpu(s) max supported 1 cpu(s)
Copying PIR from 0x08d83c40 to 0x000f5d00
Copying MPTABLE from 0x00006e30/8d670f0 to 0x000f5c20
Copying SMBIOS entry point from 0x00006e30 to 0x000f5af0
CPU Mhz=1105
cmos: write index=0x0a val=0x26
cmos: read index=0x0b val=0x02
cmos: write index=0x0b val=0x02
cmos: read index=0x0c val=0x00
cmos: read index=0x0d val=0x80
cmos: read index=0x0a val=0x26
cmos: read index=0x00 val=0x00
cmos: read index=0x02 val=0x00
cmos: read index=0x04 val=0x00
cmos: read index=0x32 val=0x19
Scan for VGA option rom
No VGA found, scan for other display
Turning on vga text mode console
SeaBIOS (version rel-1.14.0-14-g748d619-dirty-20220316_232631-maritns3-pc)
WARNING - Timeout at i8042_flush:71!
All threads complete.
cmos: read index=0x10 val=0x00
Searching bootorder for: HALT
Found 1 serial ports
Scan for option roms
Running option rom at c000:0003
cmos: read index=0x00 val=0x00
Searching bootorder for: /rom@genroms/linuxboot_dma.bin
Searching bootorder for: HALT
Space available for UMB: c0800-e9800, f5580-f5af0
Returned 262144 bytes of ZoneHigh
e820 map has 5 items:
  0: 0000000000000000 - 000000000009fc00 = 1 RAM
  1: 000000000009fc00 - 00000000000a0000 = 2 RESERVED
  2: 00000000000f0000 - 0000000000100000 = 2 RESERVED
  3: 0000000000ec4000 - 0000000008dc4000 = 1 RAM
  4: 00000000fffc0000 - 0000000100000000 = 2 RESERVED
pam [00] (3 -> 1)
pam [01] (3 -> 1)
pam [02] (3 -> 1)
pam [03] (3 -> 1)
pam [04] (3 -> 1)
pam [05] (3 -> 3)
pam [06] (3 -> 3)
pam [07] (3 -> 3)
pam [08] (3 -> 3)
pam [09] (3 -> 3)
pam [10] (3 -> 3)
pam [11] (3 -> 3)
pam [12] (3 -> 3)
pam [00] (1 -> 1)
pam [01] (1 -> 1)
pam [02] (1 -> 1)
pam [03] (1 -> 1)
pam [04] (1 -> 1)
pam [05] (3 -> 1)
pam [06] (3 -> 1)
pam [07] (3 -> 1)
pam [08] (3 -> 1)
pam [09] (3 -> 1)
pam [10] (3 -> 1)
pam [11] (3 -> 3)
pam [12] (3 -> 3)
cmos: read index=0x0f val=0x00
enter handle_19:
  NULL
Booting from ROM...
Booting from c000:0054
cmos: read index=0x00 val=0x00
guest ip : c0060
failed in [memory dispatch] with offset=[100000]
memory_region_look_up:110: g_assert_not_reached!!! bmbt never call exit !!!
```

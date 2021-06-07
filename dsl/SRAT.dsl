/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20190509 (64-bit version)
 * Copyright (c) 2000 - 2019 Intel Corporation
 * 
 * Disassembly of SRAT, Fri Jun  4 20:42:20 2021
 *
 * ACPI Data Table [SRAT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "SRAT"    [System Resource Affinity Table]
[004h 0004   4]                 Table Length : 000000C0
[008h 0008   1]                     Revision : 02
[009h 0009   1]                     Checksum : 39
[00Ah 0010   6]                       Oem ID : "LOONGS"
[010h 0016   8]                 Oem Table ID : "LOONGSON"
[018h 0024   4]                 Oem Revision : 00000002
[01Ch 0028   4]              Asl Compiler ID : "LIUX"
[020h 0032   4]        Asl Compiler Revision : 01000013

[024h 0036   4]               Table Revision : 00000001
[028h 0040   8]                     Reserved : 0000000000000000

[030h 0048   1]                Subtable Type : 00 [Processor Local APIC/SAPIC Affinity]
[031h 0049   1]                       Length : 10

[032h 0050   1]      Proximity Domain Low(8) : 00
[033h 0051   1]                      Apic ID : 00
[034h 0052   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
[038h 0056   1]              Local Sapic EID : 00
[039h 0057   3]    Proximity Domain High(24) : 000000
[03Ch 0060   4]                 Clock Domain : 00000000

[040h 0064   1]                Subtable Type : 00 [Processor Local APIC/SAPIC Affinity]
[041h 0065   1]                       Length : 10

[042h 0066   1]      Proximity Domain Low(8) : 00
[043h 0067   1]                      Apic ID : 01
[044h 0068   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
[048h 0072   1]              Local Sapic EID : 00
[049h 0073   3]    Proximity Domain High(24) : 000000
[04Ch 0076   4]                 Clock Domain : 00000000

[050h 0080   1]                Subtable Type : 00 [Processor Local APIC/SAPIC Affinity]
[051h 0081   1]                       Length : 10

[052h 0082   1]      Proximity Domain Low(8) : 00
[053h 0083   1]                      Apic ID : 02
[054h 0084   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
[058h 0088   1]              Local Sapic EID : 00
[059h 0089   3]    Proximity Domain High(24) : 000000
[05Ch 0092   4]                 Clock Domain : 00000000

[060h 0096   1]                Subtable Type : 00 [Processor Local APIC/SAPIC Affinity]
[061h 0097   1]                       Length : 10

[062h 0098   1]      Proximity Domain Low(8) : 00
[063h 0099   1]                      Apic ID : 03
[064h 0100   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
[068h 0104   1]              Local Sapic EID : 00
[069h 0105   3]    Proximity Domain High(24) : 000000
[06Ch 0108   4]                 Clock Domain : 00000000

[070h 0112   1]                Subtable Type : 01 [Memory Affinity]
[071h 0113   1]                       Length : 28

[072h 0114   4]             Proximity Domain : 00000000
[076h 0118   2]                    Reserved1 : 0000
[078h 0120   8]                 Base Address : 0000000000000000
[080h 0128   8]               Address Length : 0000000010000000
[088h 0136   4]                    Reserved2 : 00000000
[08Ch 0140   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
                               Hot Pluggable : 0
                                Non-Volatile : 0
[090h 0144   8]                    Reserved3 : 0000000000000000

[098h 0152   1]                Subtable Type : 01 [Memory Affinity]
[099h 0153   1]                       Length : 28

[09Ah 0154   4]             Proximity Domain : 00000000
[09Eh 0158   2]                    Reserved1 : 0000
[0A0h 0160   8]                 Base Address : 0000000090000000
[0A8h 0168   8]               Address Length : 00000003F0000000
[0B0h 0176   4]                    Reserved2 : 00000000
[0B4h 0180   4]        Flags (decoded below) : 00000001
                                     Enabled : 1
                               Hot Pluggable : 0
                                Non-Volatile : 0
[0B8h 0184   8]                    Reserved3 : 0000000000000000

Raw Table Data: Length 192 (0xC0)

    0000: 53 52 41 54 C0 00 00 00 02 39 4C 4F 4F 4E 47 53  // SRAT.....9LOONGS
    0010: 4C 4F 4F 4E 47 53 4F 4E 02 00 00 00 4C 49 55 58  // LOONGSON....LIUX
    0020: 13 00 00 01 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0030: 00 10 00 00 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0040: 00 10 00 01 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0050: 00 10 00 02 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0060: 00 10 00 03 01 00 00 00 00 00 00 00 00 00 00 00  // ................
    0070: 01 28 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // .(..............
    0080: 00 00 00 10 00 00 00 00 00 00 00 00 01 00 00 00  // ................
    0090: 00 00 00 00 00 00 00 00 01 28 00 00 00 00 00 00  // .........(......
    00A0: 00 00 00 90 00 00 00 00 00 00 00 F0 03 00 00 00  // ................
    00B0: 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00  // ................

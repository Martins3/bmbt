/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20190509 (64-bit version)
 * Copyright (c) 2000 - 2019 Intel Corporation
 *
 * Disassembly of APIC, Fri Jun  4 20:41:55 2021
 *
 * ACPI Data Table [APIC]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "APIC"    [Multiple APIC Description Table (MADT)]
[004h 0004   4]                 Table Length : 0000005E
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : 4B
[00Ah 0010   6]                       Oem ID : "LOONGS"
[010h 0016   8]                 Oem Table ID : "LOONGSON"
[018h 0024   4]                 Oem Revision : 00000001
[01Ch 0028   4]              Asl Compiler ID : "LIUX"
[020h 0032   4]        Asl Compiler Revision : 00000000

[024h 0036   4]           Local Apic Address : 1FE01400
[028h 0040   4]        Flags (decoded below) : 00000001
                         PC-AT Compatibility : 1

[02Ch 0044   1]                Subtable Type : 00 [Processor Local APIC]
[02Dh 0045   1]                       Length : 08
[02Eh 0046   1]                 Processor ID : 01
[02Fh 0047   1]                Local Apic ID : 00
[030h 0048   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1
                      Runtime Online Capable : 0

[034h 0052   1]                Subtable Type : 00 [Processor Local APIC]
[035h 0053   1]                       Length : 08
[036h 0054   1]                 Processor ID : 02
[037h 0055   1]                Local Apic ID : 01
[038h 0056   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1
                      Runtime Online Capable : 0

[03Ch 0060   1]                Subtable Type : 00 [Processor Local APIC]
[03Dh 0061   1]                       Length : 08
[03Eh 0062   1]                 Processor ID : 03
[03Fh 0063   1]                Local Apic ID : 02
[040h 0064   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1
                      Runtime Online Capable : 0

[044h 0068   1]                Subtable Type : 00 [Processor Local APIC]
[045h 0069   1]                       Length : 08
[046h 0070   1]                 Processor ID : 04
[047h 0071   1]                Local Apic ID : 03
[048h 0072   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1
                      Runtime Online Capable : 0

[04Ch 0076   1]                Subtable Type : 01 [I/O APIC]
[04Dh 0077   1]                       Length : 0C
[04Eh 0078   1]                  I/O Apic ID : 00
[04Fh 0079   1]                     Reserved : 00
[050h 0080   4]                      Address : 10000000
[054h 0084   4]                    Interrupt : 00000040

[058h 0088   1]                Subtable Type : 04 [Local APIC NMI]
[059h 0089   1]                       Length : 06
[05Ah 0090   1]                 Processor ID : 05
[05Bh 0091   2]        Flags (decoded below) : 0000
                                    Polarity : 0
                                Trigger Mode : 0
[05Dh 0093   1]         Interrupt Input LINT : 01

Raw Table Data: Length 94 (0x5E)

    0000: 41 50 49 43 5E 00 00 00 01 4B 4C 4F 4F 4E 47 53  // APIC^....KLOONGS
    0010: 4C 4F 4F 4E 47 53 4F 4E 01 00 00 00 4C 49 55 58  // LOONGSON....LIUX
    0020: 00 00 00 00 00 14 E0 1F 01 00 00 00 00 08 01 00  // ................
    0030: 01 00 00 00 00 08 02 01 01 00 00 00 00 08 03 02  // ................
    0040: 01 00 00 00 00 08 04 03 01 00 00 00 01 0C 00 00  // ................
    0050: 00 00 00 10 40 00 00 00 04 06 05 00 00 01        // ....@.........

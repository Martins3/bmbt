/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20190509 (64-bit version)
 * Copyright (c) 2000 - 2019 Intel Corporation
 * 
 * Disassembly of SLIT, Fri Jun  4 20:42:15 2021
 *
 * ACPI Data Table [SLIT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "SLIT"    [System Locality Information Table]
[004h 0004   4]                 Table Length : 0000002D
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : F2
[00Ah 0010   6]                       Oem ID : "LOONGS"
[010h 0016   8]                 Oem Table ID : "LOONGSON"
[018h 0024   4]                 Oem Revision : 00000002
[01Ch 0028   4]              Asl Compiler ID : "LIUX"
[020h 0032   4]        Asl Compiler Revision : 01000013

[024h 0036   8]                   Localities : 0000000000000001
[02Ch 0044   1]                 Locality   0 : 0A

Raw Table Data: Length 45 (0x2D)

    0000: 53 4C 49 54 2D 00 00 00 01 F2 4C 4F 4F 4E 47 53  // SLIT-.....LOONGS
    0010: 4C 4F 4F 4E 47 53 4F 4E 02 00 00 00 4C 49 55 58  // LOONGSON....LIUX
    0020: 13 00 00 01 01 00 00 00 00 00 00 00 0A           // .............

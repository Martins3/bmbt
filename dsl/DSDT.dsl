/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20190509 (64-bit version)
 * Copyright (c) 2000 - 2019 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of DSDT, Fri Jun  4 20:42:00 2021
 *
 * Original Table Header:
 *     Signature        "DSDT"
 *     Length           0x00001A9C (6812)
 *     Revision         0x02
 *     Checksum         0x91
 *     OEM ID           "LGSON "
 *     OEM Table ID     "TP-R00  "
 *     OEM Revision     0x00000476 (1142)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20160527 (538314023)
 */
DefinitionBlock ("", "DSDT", 2, "LGSON ", "TP-R00  ", 0x00000476)
{
    Scope (_SB)
    {
        Device (PCI0)
        {
            Name (_BBN, Zero)  // _BBN: BIOS Bus Number
            Name (_ADR, Zero)  // _ADR: Address
            Name (_HID, EisaId ("PNP0A08") /* PCI Express Bus */)  // _HID: Hardware ID
            Name (_CID, EisaId ("PNP0A03") /* PCI Bus */)  // _CID: Compatible ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode,
                    0x0000,             // Granularity
                    0x0000,             // Range Minimum
                    0x00FF,             // Range Maximum
                    0x0000,             // Translation Offset
                    0x0100,             // Length
                    ,, )
                WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
                    0x0000,             // Granularity
                    0x4000,             // Range Minimum
                    0xFFFF,             // Range Maximum
                    0x0000,             // Translation Offset
                    0xC000,             // Length
                    ,, , TypeStatic, DenseTranslation)
                DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
                    0x00000000,         // Granularity
                    0x40000000,         // Range Minimum
                    0x7FFFFFFF,         // Range Maximum
                    0x00000000,         // Translation Offset
                    0x40000000,         // Length
                    ,, , AddressRangeMemory, TypeStatic)
            })
            Name (_PRT, Package (0x3D)  // _PRT: PCI Routing Table
            {
                Package (0x04)
                {
                    0x0003FFFF, 
                    Zero, 
                    Zero, 
                    0x4C
                }, 

                Package (0x04)
                {
                    0x0003FFFF, 
                    One, 
                    Zero, 
                    0x4E
                }, 

                Package (0x04)
                {
                    0x0004FFFF, 
                    Zero, 
                    Zero, 
                    0x71
                }, 

                Package (0x04)
                {
                    0x0004FFFF, 
                    One, 
                    Zero, 
                    0x70
                }, 

                Package (0x04)
                {
                    0x0005FFFF, 
                    Zero, 
                    Zero, 
                    0x73
                }, 

                Package (0x04)
                {
                    0x0005FFFF, 
                    One, 
                    Zero, 
                    0x72
                }, 

                Package (0x04)
                {
                    0x0006FFFF, 
                    Zero, 
                    Zero, 
                    0x5D
                }, 

                Package (0x04)
                {
                    0x0006FFFF, 
                    One, 
                    Zero, 
                    0x5C
                }, 

                Package (0x04)
                {
                    0x0007FFFF, 
                    Zero, 
                    Zero, 
                    0x7A
                }, 

                Package (0x04)
                {
                    0x0008FFFF, 
                    Zero, 
                    Zero, 
                    0x50
                }, 

                Package (0x04)
                {
                    0x0008FFFF, 
                    One, 
                    Zero, 
                    0x51
                }, 

                Package (0x04)
                {
                    0x0008FFFF, 
                    0x02, 
                    Zero, 
                    0x52
                }, 

                Package (0x04)
                {
                    0x0009FFFF, 
                    Zero, 
                    Zero, 
                    0x60
                }, 

                Package (0x04)
                {
                    0x0009FFFF, 
                    One, 
                    Zero, 
                    0x60
                }, 

                Package (0x04)
                {
                    0x0009FFFF, 
                    0x02, 
                    Zero, 
                    0x60
                }, 

                Package (0x04)
                {
                    0x0009FFFF, 
                    0x03, 
                    Zero, 
                    0x60
                }, 

                Package (0x04)
                {
                    0x000AFFFF, 
                    Zero, 
                    Zero, 
                    0x61
                }, 

                Package (0x04)
                {
                    0x000AFFFF, 
                    One, 
                    Zero, 
                    0x61
                }, 

                Package (0x04)
                {
                    0x000AFFFF, 
                    0x02, 
                    Zero, 
                    0x61
                }, 

                Package (0x04)
                {
                    0x000AFFFF, 
                    0x03, 
                    Zero, 
                    0x61
                }, 

                Package (0x04)
                {
                    0x000BFFFF, 
                    Zero, 
                    Zero, 
                    0x62
                }, 

                Package (0x04)
                {
                    0x000BFFFF, 
                    One, 
                    Zero, 
                    0x62
                }, 

                Package (0x04)
                {
                    0x000BFFFF, 
                    0x02, 
                    Zero, 
                    0x62
                }, 

                Package (0x04)
                {
                    0x000BFFFF, 
                    0x03, 
                    Zero, 
                    0x62
                }, 

                Package (0x04)
                {
                    0x000CFFFF, 
                    Zero, 
                    Zero, 
                    0x63
                }, 

                Package (0x04)
                {
                    0x000CFFFF, 
                    One, 
                    Zero, 
                    0x63
                }, 

                Package (0x04)
                {
                    0x000CFFFF, 
                    0x02, 
                    Zero, 
                    0x63
                }, 

                Package (0x04)
                {
                    0x000CFFFF, 
                    0x03, 
                    Zero, 
                    0x63
                }, 

                Package (0x04)
                {
                    0x000DFFFF, 
                    Zero, 
                    Zero, 
                    0x64
                }, 

                Package (0x04)
                {
                    0x000DFFFF, 
                    One, 
                    Zero, 
                    0x64
                }, 

                Package (0x04)
                {
                    0x000DFFFF, 
                    0x02, 
                    Zero, 
                    0x64
                }, 

                Package (0x04)
                {
                    0x000DFFFF, 
                    0x03, 
                    Zero, 
                    0x64
                }, 

                Package (0x04)
                {
                    0x000EFFFF, 
                    Zero, 
                    Zero, 
                    0x65
                }, 

                Package (0x04)
                {
                    0x000EFFFF, 
                    One, 
                    Zero, 
                    0x65
                }, 

                Package (0x04)
                {
                    0x000EFFFF, 
                    0x02, 
                    Zero, 
                    0x65
                }, 

                Package (0x04)
                {
                    0x000EFFFF, 
                    0x03, 
                    Zero, 
                    0x65
                }, 

                Package (0x04)
                {
                    0x000FFFFF, 
                    Zero, 
                    Zero, 
                    0x68
                }, 

                Package (0x04)
                {
                    0x000FFFFF, 
                    One, 
                    Zero, 
                    0x68
                }, 

                Package (0x04)
                {
                    0x000FFFFF, 
                    0x02, 
                    Zero, 
                    0x68
                }, 

                Package (0x04)
                {
                    0x000FFFFF, 
                    0x03, 
                    Zero, 
                    0x68
                }, 

                Package (0x04)
                {
                    0x0010FFFF, 
                    Zero, 
                    Zero, 
                    0x69
                }, 

                Package (0x04)
                {
                    0x0010FFFF, 
                    One, 
                    Zero, 
                    0x69
                }, 

                Package (0x04)
                {
                    0x0010FFFF, 
                    0x02, 
                    Zero, 
                    0x69
                }, 

                Package (0x04)
                {
                    0x0010FFFF, 
                    0x03, 
                    Zero, 
                    0x69
                }, 

                Package (0x04)
                {
                    0x0011FFFF, 
                    Zero, 
                    Zero, 
                    0x6A
                }, 

                Package (0x04)
                {
                    0x0011FFFF, 
                    One, 
                    Zero, 
                    0x6A
                }, 

                Package (0x04)
                {
                    0x0011FFFF, 
                    0x02, 
                    Zero, 
                    0x6A
                }, 

                Package (0x04)
                {
                    0x0011FFFF, 
                    0x03, 
                    Zero, 
                    0x6A
                }, 

                Package (0x04)
                {
                    0x0012FFFF, 
                    Zero, 
                    Zero, 
                    0x6B
                }, 

                Package (0x04)
                {
                    0x0012FFFF, 
                    One, 
                    Zero, 
                    0x6B
                }, 

                Package (0x04)
                {
                    0x0012FFFF, 
                    0x02, 
                    Zero, 
                    0x6B
                }, 

                Package (0x04)
                {
                    0x0012FFFF, 
                    0x03, 
                    Zero, 
                    0x6B
                }, 

                Package (0x04)
                {
                    0x0013FFFF, 
                    Zero, 
                    Zero, 
                    0x66
                }, 

                Package (0x04)
                {
                    0x0013FFFF, 
                    One, 
                    Zero, 
                    0x66
                }, 

                Package (0x04)
                {
                    0x0013FFFF, 
                    0x02, 
                    Zero, 
                    0x66
                }, 

                Package (0x04)
                {
                    0x0013FFFF, 
                    0x03, 
                    Zero, 
                    0x66
                }, 

                Package (0x04)
                {
                    0x0014FFFF, 
                    Zero, 
                    Zero, 
                    0x67
                }, 

                Package (0x04)
                {
                    0x0014FFFF, 
                    One, 
                    Zero, 
                    0x67
                }, 

                Package (0x04)
                {
                    0x0014FFFF, 
                    0x02, 
                    Zero, 
                    0x67
                }, 

                Package (0x04)
                {
                    0x0014FFFF, 
                    0x03, 
                    Zero, 
                    0x67
                }, 

                Package (0x04)
                {
                    0x0017FFFF, 
                    Zero, 
                    Zero, 
                    0x53
                }
            })
        }
    }

    Scope (_SB.PCI0)
    {
        Device (PCIB)
        {
            Name (_ADR, 0x000A0000)  // _ADR: Address
            Device (XHCI)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_S3D, 0x04)  // _S3D: S3 Device State
                Name (_S4D, 0x04)  // _S4D: S4 Device State
                PowerResource (PUBS, 0x03, 0x0000)
                {
                    Name (_STA, One)  // _STA: Status
                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                    {
                        _STA = One
                    }

                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                    {
                        _STA = Zero
                    }
                }

                Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
                {
                    Return (PCID (Arg0, Arg1, Arg2, Arg3))
                }

                Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
                {
                    PUBS
                })
                Name (_PR3, Package (0x01)  // _PR3: Power Resources for D3hot
                {
                    PUBS
                })
            }
        }

        Device (USB0)
        {
            Name (_ADR, 0x00040001)  // _ADR: Address
            Device (RHUB)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Device (PRT1)
                {
                    Name (_ADR, One)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB0.RHUB.PRT1._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB0.RHUB.PRT1._PLD.PLDP */
                    }
                }

                Device (PRT2)
                {
                    Name (_ADR, 0x02)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB0.RHUB.PRT2._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB0.RHUB.PRT2._PLD.PLDP */
                    }
                }

                Device (PRT3)
                {
                    Name (_ADR, 0x03)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB0.RHUB.PRT3._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0xA9, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .J......
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB0.RHUB.PRT3._PLD.PLDP */
                    }
                }
            }

            Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
            {
                ^PCIB.XHCI.PUBS
            })
            Method (_PRW, 0, NotSerialized)  // _PRW: Power Resources for Wake
            {
                Return (Package (0x02)
                {
                    0x0A, 
                    0x03
                })
            }
        }

        Device (USB1)
        {
            Name (_ADR, 0x00040000)  // _ADR: Address
            Device (RHUB)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Device (PRT1)
                {
                    Name (_ADR, One)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB1.RHUB.PRT1._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB1.RHUB.PRT1._PLD.PLDP */
                    }
                }

                Device (PRT2)
                {
                    Name (_ADR, 0x02)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB1.RHUB.PRT2._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB1.RHUB.PRT2._PLD.PLDP */
                    }
                }

                Device (PRT3)
                {
                    Name (_ADR, 0x03)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB1.RHUB.PRT3._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0xA9, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .J......
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB1.RHUB.PRT3._PLD.PLDP */
                    }
                }
            }
        }

        Device (USB2)
        {
            Name (_ADR, 0x00050001)  // _ADR: Address
            Device (RHUB)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Device (PRT1)
                {
                    Name (_ADR, One)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB2.RHUB.PRT1._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB2.RHUB.PRT1._PLD.PLDP */
                    }
                }

                Device (PRT2)
                {
                    Name (_ADR, 0x02)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB2.RHUB.PRT2._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB2.RHUB.PRT2._PLD.PLDP */
                    }
                }

                Device (PRT3)
                {
                    Name (_ADR, 0x03)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB2.RHUB.PRT3._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0xA9, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .J......
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB2.RHUB.PRT3._PLD.PLDP */
                    }
                }
            }

            Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
            {
                ^PCIB.XHCI.PUBS
            })
            Method (_PRW, 0, NotSerialized)  // _PRW: Power Resources for Wake
            {
                Return (Package (0x02)
                {
                    0x0D, 
                    0x03
                })
            }
        }

        Device (USB3)
        {
            Name (_ADR, 0x00050000)  // _ADR: Address
            Device (RHUB)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Device (PRT1)
                {
                    Name (_ADR, One)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB3.RHUB.PRT1._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB3.RHUB.PRT1._PLD.PLDP */
                    }
                }

                Device (PRT2)
                {
                    Name (_ADR, 0x02)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB3.RHUB.PRT2._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0x01, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB3.RHUB.PRT2._PLD.PLDP */
                    }
                }

                Device (PRT3)
                {
                    Name (_ADR, 0x03)  // _ADR: Address
                    Method (_UPC, 0, Serialized)  // _UPC: USB Port Capabilities
                    {
                        Name (UPCP, Package (0x04)
                        {
                            0xFF, 
                            Zero, 
                            Zero, 
                            Zero
                        })
                        Return (UPCP) /* \_SB_.PCI0.USB3.RHUB.PRT3._UPC.UPCP */
                    }

                    Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                    {
                        Name (PLDP, Package (0x01)
                        {
                            Buffer (0x14)
                            {
                                /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                                /* 0008 */  0xA9, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .J......
                                /* 0010 */  0x00, 0x00, 0x00, 0x00                           // ....
                            }
                        })
                        Return (PLDP) /* \_SB_.PCI0.USB3.RHUB.PRT3._PLD.PLDP */
                    }
                }
            }
        }

        Device (NIC0)
        {
            Name (_ADR, 0x00030000)  // _ADR: Address
            Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
            {
                ^PCIB.XHCI.PUBS
            })
            Method (_PRW, 0, NotSerialized)  // _PRW: Power Resources for Wake
            {
                Return (Package (0x02)
                {
                    0x05, 
                    0x03
                })
            }
        }

        Device (NIC1)
        {
            Name (_ADR, 0x00030001)  // _ADR: Address
            Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
            {
                ^PCIB.XHCI.PUBS
            })
            Method (_PRW, 0, NotSerialized)  // _PRW: Power Resources for Wake
            {
                Return (Package (0x02)
                {
                    0x06, 
                    0x03
                })
            }
        }

        Name (HDTF, Buffer (0x0E)
        {
             0x02, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xEF, 0x00   // ........
        })
        Name (HPTF, Buffer (0x15)
        {
            /* 0000 */  0x02, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xEF, 0x10,  // ........
            /* 0008 */  0x03, 0x00, 0x00, 0x00, 0xA0, 0xEF, 0x00, 0x00   // ........
        })
        Device (SAT0)
        {
            Name (_ADR, 0x00080000)  // _ADR: Address
            Device (PORT)
            {
                Name (DIP0, Zero)
                Name (_ADR, 0xFFFF)  // _ADR: Address
                Method (_SDD, 1, NotSerialized)  // _SDD: Set Device Data
                {
                    DIP0 = Zero
                    If ((SizeOf (Arg0) == 0x0200))
                    {
                        CreateWordField (Arg0, 0x9C, M078)
                        If ((M078 & 0x08))
                        {
                            DIP0 = One
                        }
                    }
                }

                Method (_GTF, 0, NotSerialized)  // _GTF: Get Task File
                {
                    If (DIP0)
                    {
                        Return (HPTF) /* \_SB_.PCI0.HPTF */
                    }

                    Return (HDTF) /* \_SB_.PCI0.HDTF */
                }
            }
        }

        Device (SAT1)
        {
            Name (_ADR, 0x00080001)  // _ADR: Address
            Device (PORT)
            {
                Name (DIP1, Zero)
                Name (_ADR, 0xFFFF)  // _ADR: Address
                Method (_SDD, 1, NotSerialized)  // _SDD: Set Device Data
                {
                    DIP1 = Zero
                    If ((SizeOf (Arg0) == 0x0200))
                    {
                        CreateWordField (Arg0, 0x9C, M078)
                        If ((M078 & 0x08))
                        {
                            DIP1 = One
                        }
                    }
                }

                Method (_GTF, 0, NotSerialized)  // _GTF: Get Task File
                {
                    If (DIP1)
                    {
                        Return (HPTF) /* \_SB_.PCI0.HPTF */
                    }

                    Return (HDTF) /* \_SB_.PCI0.HDTF */
                }
            }
        }

        Device (SAT2)
        {
            Name (_ADR, 0x00080002)  // _ADR: Address
            Device (PORT)
            {
                Name (DIP2, Zero)
                Name (_ADR, 0xFFFF)  // _ADR: Address
                Method (_SDD, 1, NotSerialized)  // _SDD: Set Device Data
                {
                    DIP2 = Zero
                    If ((SizeOf (Arg0) == 0x0200))
                    {
                        CreateWordField (Arg0, 0x9C, M078)
                        If ((M078 & 0x08))
                        {
                            DIP2 = One
                        }
                    }
                }

                Method (_GTF, 0, NotSerialized)  // _GTF: Get Task File
                {
                    If (DIP2)
                    {
                        Return (HPTF) /* \_SB_.PCI0.HPTF */
                    }

                    Return (HDTF) /* \_SB_.PCI0.HDTF */
                }
            }
        }
    }

    Scope (_SB)
    {
        Device (C000)
        {
            Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_PXM, Zero)  // _PXM: Device Proximity
            Name (_STA, 0x0F)  // _STA: Status
            Method (_PPC, 0, NotSerialized)  // _PPC: Performance Present Capabilities
            {
                Return (Zero)
            }

            Method (_PCT, 0, NotSerialized)  // _PCT: Performance Control
            {
                Return (Package (0x02)
                {
                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }, 

                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }
                })
            }

            Method (_PSS, 0, NotSerialized)  // _PSS: Performance Supported States
            {
                Return (LPSS) /* \_SB_.C000.LPSS */
            }

            Name (LPSS, Package (0x10)
            {
                Package (0x06)
                {
                    0x07D0, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0103, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x0708, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x03, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x06D6, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0104, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x0627, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x04, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x05DC, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0105, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x0546, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x05, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x04E2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0106, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x0465, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x06, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x03E8, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0107, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x0384, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x07, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x02EE, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0108, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x02A3, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x08, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x01F4, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0109, 
                    0x09
                }, 

                Package (0x06)
                {
                    0x01C2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x09, 
                    0x09
                }, 

                Package (0x06)
                {
                    0xFA, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x010A, 
                    0x0A
                }, 

                Package (0x06)
                {
                    0xE1, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0A, 
                    0x0A
                }
            })
        }

        Device (C001)
        {
            Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_PXM, Zero)  // _PXM: Device Proximity
            Name (_STA, 0x0F)  // _STA: Status
            Method (_PPC, 0, NotSerialized)  // _PPC: Performance Present Capabilities
            {
                Return (Zero)
            }

            Method (_PCT, 0, NotSerialized)  // _PCT: Performance Control
            {
                Return (Package (0x02)
                {
                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }, 

                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }
                })
            }

            Method (_PSS, 0, NotSerialized)  // _PSS: Performance Supported States
            {
                Return (LPSS) /* \_SB_.C001.LPSS */
            }

            Name (LPSS, Package (0x10)
            {
                Package (0x06)
                {
                    0x07D0, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0103, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x0708, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x03, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x06D6, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0104, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x0627, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x04, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x05DC, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0105, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x0546, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x05, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x04E2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0106, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x0465, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x06, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x03E8, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0107, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x0384, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x07, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x02EE, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0108, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x02A3, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x08, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x01F4, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0109, 
                    0x09
                }, 

                Package (0x06)
                {
                    0x01C2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x09, 
                    0x09
                }, 

                Package (0x06)
                {
                    0xFA, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x010A, 
                    0x0A
                }, 

                Package (0x06)
                {
                    0xE1, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0A, 
                    0x0A
                }
            })
        }

        Device (C002)
        {
            Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
            Name (_UID, 0x03)  // _UID: Unique ID
            Name (_PXM, Zero)  // _PXM: Device Proximity
            Name (_STA, 0x0F)  // _STA: Status
            Method (_PPC, 0, NotSerialized)  // _PPC: Performance Present Capabilities
            {
                Return (Zero)
            }

            Method (_PCT, 0, NotSerialized)  // _PCT: Performance Control
            {
                Return (Package (0x02)
                {
                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }, 

                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }
                })
            }

            Method (_PSS, 0, NotSerialized)  // _PSS: Performance Supported States
            {
                Return (LPSS) /* \_SB_.C002.LPSS */
            }

            Name (LPSS, Package (0x10)
            {
                Package (0x06)
                {
                    0x07D0, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0103, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x0708, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x03, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x06D6, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0104, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x0627, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x04, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x05DC, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0105, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x0546, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x05, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x04E2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0106, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x0465, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x06, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x03E8, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0107, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x0384, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x07, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x02EE, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0108, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x02A3, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x08, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x01F4, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0109, 
                    0x09
                }, 

                Package (0x06)
                {
                    0x01C2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x09, 
                    0x09
                }, 

                Package (0x06)
                {
                    0xFA, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x010A, 
                    0x0A
                }, 

                Package (0x06)
                {
                    0xE1, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0A, 
                    0x0A
                }
            })
        }

        Device (C003)
        {
            Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
            Name (_UID, 0x04)  // _UID: Unique ID
            Name (_PXM, Zero)  // _PXM: Device Proximity
            Name (_STA, 0x0F)  // _STA: Status
            Method (_PPC, 0, NotSerialized)  // _PPC: Performance Present Capabilities
            {
                Return (Zero)
            }

            Method (_PCT, 0, NotSerialized)  // _PCT: Performance Control
            {
                Return (Package (0x02)
                {
                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }, 

                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }
                })
            }

            Method (_PSS, 0, NotSerialized)  // _PSS: Performance Supported States
            {
                Return (LPSS) /* \_SB_.C003.LPSS */
            }

            Name (LPSS, Package (0x10)
            {
                Package (0x06)
                {
                    0x07D0, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0103, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x0708, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x03, 
                    0x03
                }, 

                Package (0x06)
                {
                    0x06D6, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0104, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x0627, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x04, 
                    0x04
                }, 

                Package (0x06)
                {
                    0x05DC, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0105, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x0546, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x05, 
                    0x05
                }, 

                Package (0x06)
                {
                    0x04E2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0106, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x0465, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x06, 
                    0x06
                }, 

                Package (0x06)
                {
                    0x03E8, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0107, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x0384, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x07, 
                    0x07
                }, 

                Package (0x06)
                {
                    0x02EE, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0108, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x02A3, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x08, 
                    0x08
                }, 

                Package (0x06)
                {
                    0x01F4, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0109, 
                    0x09
                }, 

                Package (0x06)
                {
                    0x01C2, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x09, 
                    0x09
                }, 

                Package (0x06)
                {
                    0xFA, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x010A, 
                    0x0A
                }, 

                Package (0x06)
                {
                    0xE1, 
                    0x3A98, 
                    0x4E20, 
                    0x4E20, 
                    0x0A, 
                    0x0A
                }
            })
        }
    }

    Scope (_SB)
    {
        Device (GPO0)
        {
            Name (_HID, "LOON0002")  // _HID: Hardware ID
            Name (_ADR, Zero)  // _ADR: Address
            Name (_UID, One)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x00000000100E0000, // Range Minimum
                    0x00000000100E0BFF, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000C00, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000007C,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x05)
                {
                    Package (0x02)
                    {
                        "conf_offset", 
                        0x0800
                    }, 

                    Package (0x02)
                    {
                        "out_offset", 
                        0x0900
                    }, 

                    Package (0x02)
                    {
                        "in_offset", 
                        0x0A00
                    }, 

                    Package (0x02)
                    {
                        "gpio_base", 
                        0x10
                    }, 

                    Package (0x02)
                    {
                        "ngpios", 
                        0x39
                    }
                }
            })
        }

        Device (COMA)
        {
            Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x000000001FE001E0, // Range Minimum
                    0x000000001FE001E7, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000001A,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x05F5E100
                    }
                }
            })
        }

        Device (COMB)
        {
            Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010080000, // Range Minimum
                    0x00000000100800FF, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000100, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000048,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x02FAF080
                    }
                }
            })
        }

        Device (COMC)
        {
            Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010080100, // Range Minimum
                    0x00000000100801FF, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000100, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000048,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x02FAF080
                    }
                }
            })
        }

        Device (COMD)
        {
            Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
            Name (_UID, 0x03)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010080200, // Range Minimum
                    0x00000000100802FF, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000100, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000048,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x02FAF080
                    }
                }
            })
        }

        Device (COME)
        {
            Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
            Name (_UID, 0x04)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010080300, // Range Minimum
                    0x00000000100803FF, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000100, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000048,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x02FAF080
                    }
                }
            })
        }

        Device (RTC)
        {
            Name (_HID, "LOON0001")  // _HID: Hardware ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x00000000100D0100, // Range Minimum
                    0x00000000100D01FF, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000100, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000074,
                }
            })
        }

        Device (I2C0)
        {
            Name (_HID, "LOON0004")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010090000, // Range Minimum
                    0x0000000010090007, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000049,
                }
            })
        }

        Device (I2C1)
        {
            Name (_HID, "LOON0004")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010090100, // Range Minimum
                    0x0000000010090107, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000049,
                }
            })
        }

        Device (I2C2)
        {
            Name (_HID, "LOON0004")  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010090200, // Range Minimum
                    0x0000000010090207, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000049,
                }
            })
        }

        Device (I2C3)
        {
            Name (_HID, "LOON0004")  // _HID: Hardware ID
            Name (_UID, 0x03)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010090300, // Range Minimum
                    0x0000000010090307, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000049,
                }
            })
        }

        Device (I2C4)
        {
            Name (_HID, "LOON0004")  // _HID: Hardware ID
            Name (_UID, 0x04)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010090400, // Range Minimum
                    0x0000000010090407, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000049,
                }
            })
        }

        Device (I2C5)
        {
            Name (_HID, "LOON0004")  // _HID: Hardware ID
            Name (_UID, 0x05)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000010090500, // Range Minimum
                    0x0000000010090507, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000008, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000049,
                }
            })
        }

        Device (PWM0)
        {
            Name (_HID, "LOON0006")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x00000000100A0000, // Range Minimum
                    0x00000000100A000F, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000010, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000058,
                }
            })
        }

        Device (PWM1)
        {
            Name (_HID, "LOON0006")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x00000000100A0100, // Range Minimum
                    0x00000000100A010F, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000010, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x00000059,
                }
            })
        }

        Device (PWM2)
        {
            Name (_HID, "LOON0006")  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x00000000100A0200, // Range Minimum
                    0x00000000100A020F, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000010, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005A,
                }
            })
        }

        Device (PWM3)
        {
            Name (_HID, "LOON0006")  // _HID: Hardware ID
            Name (_UID, 0x03)  // _UID: Unique ID
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x00000000100A0300, // Range Minimum
                    0x00000000100A030F, // Range Maximum
                    0x0000000000000000, // Translation Offset
                    0x0000000000000010, // Length
                    ,, , AddressRangeMemory, TypeStatic)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005B,
                }
            })
        }

        Device (NIPM)
        {
            Name (_HID, "IPI0001")  // _HID: Hardware ID
            Name (_STR, Unicode ("IPMI_KCS"))  // _STR: Description String
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Return (ResourceTemplate ()
                {
                    IO (Decode16,
                        0x0CA2,             // Range Minimum
                        0x0CA2,             // Range Maximum
                        0x00,               // Alignment
                        0x01,               // Length
                        )
                    IO (Decode16,
                        0x0CA3,             // Range Minimum
                        0x0CA3,             // Range Maximum
                        0x00,               // Alignment
                        0x01,               // Length
                        )
                })
            }

            Method (_IFT, 0, NotSerialized)  // _IFT: IPMI Interface Type
            {
                Return (One)
            }

            Method (_SRV, 0, NotSerialized)  // _SRV: IPMI Spec Revision
            {
                Return (0x0200)
            }
        }
    }

    Scope (_SB.PCI0)
    {
        Name (PCIG, ToUUID ("e5c937d0-3553-4d7a-9117-ea4d19c3434d") /* Device Labeling Interface */)
        Method (PCID, 4, Serialized)
        {
            If ((Arg0 == PCIG))
            {
                If ((Arg1 >= 0x03))
                {
                    If ((Arg2 == Zero))
                    {
                        Return (Buffer (0x02)
                        {
                             0x01, 0x03                                       // ..
                        })
                    }

                    If ((Arg2 == 0x09))
                    {
                        Return (Package (0x05)
                        {
                            0xC350, 
                            Ones, 
                            Ones, 
                            0xC350, 
                            Ones
                        })
                    }
                }
            }

            Return (Buffer (One)
            {
                 0x00                                             // .
            })
        }
    }

    Name (_S0, Package (0x04)  // _S0_: S0 System State
    {
        Zero, 
        Zero, 
        Zero, 
        Zero
    })
    Name (_S3, Package (0x04)  // _S3_: S3 System State
    {
        0x05, 
        0x05, 
        Zero, 
        Zero
    })
    Name (_S4, Package (0x04)  // _S4_: S4 System State
    {
        0x06, 
        0x06, 
        Zero, 
        Zero
    })
    Name (_S5, Package (0x04)  // _S5_: S5 System State
    {
        0x07, 
        0x07, 
        Zero, 
        Zero
    })
    Name (SADR, 0x800000001C062494)
    Scope (_GPE)
    {
    }
}


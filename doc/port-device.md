# 分析一下可能需要模拟的设备

看来一下，感觉其实还好吧！

| Device           | Strategy             |
|------------------|----------------------|
| port92           |                      |
| ioport80         |                      |
| ioportF0         |                      |
| rtc              |                      |
| isa-debugcon     |                      |
| pci-conf-idx     |                      |
| pci-conf-data    |                      |
| fwcfg            |                      |
| fwcfg.dma        |                      |
| io               |                      |
| apm-io           |                      |
| rtc-index        |                      |
| vga              |                      |
| vbe              |                      |
| i8042-cmd        |                      |
| i8042-data       |                      |
| parallel         |                      |
| serial           |                      |
| kvmvapic         |                      |
| pcspk            | speaker 暂时不用考虑 |
| acpi-cnt         |                      |
| acpi-evt         |                      |
| acpi-gpe0        |                      |
| acpi-cpu-hotplug |                      |
| acpi-tmr         |                      |
| dma-page         |                      |
| dma-cont         |                      |
| fdc              |                      |
| e1000-io         |                      |
| piix-bmdma       |                      |
| bmdma            |                      |
| ide              |                      |

## vbe
https://wiki.osdev.org/VBE

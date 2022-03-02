capStone_data = configuration_data()
capStone_data.set('CAPSTONE_USE_SYS_DYN_MEM', '1')

# As the macro in qemu can not pass to capstone, so if
# you want to open the CONFIG_LATX_DEBUG, open the line below.
capStone_data.set('CONFIG_LATX_DEBUG', '1')

capStone_files = files(
  'capStone/cs.c',
  'capStone/MCInst.c',
  'capStone/MCInstrDesc.c',
  'capStone/MCRegisterInfo.c',
  'capStone/SStream.c',
  'capStone/utils.c'
)

capStone_data.set('CAPSTONE_HAS_X86', 1)
capStone_files += files(
  'capStone/arch/X86/X86Disassembler.c',
  'capStone/arch/X86/X86DisassemblerDecoder.c',
  'capStone/arch/X86/X86ATTInstPrinter.c',
  'capStone/arch/X86/X86IntelInstPrinter.c',
  'capStone/arch/X86/X86Mapping.c',
  'capStone/arch/X86/X86Module.c'
)

configure_file(output: 'capStone-defs.h', configuration: capStone_data)

capStone_cargs = [
  # FIXME: There does not seem to be a way to completely replace the c_args
  # that come from add_project_arguments() -- we can only add to them.
  # So: disable all warnings with a big hammer.
  '-Wno-error', '-w',

  # Include all configuration defines via a header file, which will wind up
  # as a dependency on the object file, and thus changes here will result
  # in a rebuild.
  '-include', 'capStone-defs.h'
]

libcapStone = static_library('capStone',
                             build_by_default: false,
                             sources: capStone_files,
                             c_args: capStone_cargs,
                             include_directories: 'capStone/include')
capStone = declare_dependency(link_with: libcapStone,
                              include_directories: 'capStone/include/capstone')

i386_ss.add(when: 'CONFIG_LATX', if_true: files(
  'error.c',
  'mem.c',
  'latx-config.c',
  'latx-options.c',
))

i386_ss.add(when: 'CONFIG_LATX', if_true: capStone)

i386_softmmu_ss.add(when: 'CONFIG_LATX', if_true: files(
  'sys-test.c'
))

subdir('diStorm')
subdir('ir1')
subdir('ir2')
subdir('optimization')
subdir('translator')
subdir('wrapper')

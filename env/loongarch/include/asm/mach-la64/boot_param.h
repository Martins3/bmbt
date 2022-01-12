#ifndef BOOT_PARAM_H_UOQ1F6AT
#define BOOT_PARAM_H_UOQ1F6AT

union extlist_line {
  struct _extention_list_hdr *extlist;
  u64 ext_offset;
};

struct boot_params {
  u64 signature; /* {"BPIXXXXX"} */
  union {
    void *systemtable;
    u64 systab_offset;
  };
  union extlist_line ext_location;
  u64 flags;
} __attribute__((packed));

#endif /* end of include guard: BOOT_PARAM_H_UOQ1F6AT */

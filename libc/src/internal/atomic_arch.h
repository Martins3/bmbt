#define LLSC_M "m"

#define a_ll a_ll
static inline int a_ll(volatile int *p) {
  int v;
  __asm__ __volatile__("ll.w %0, %1" : "=r"(v) : LLSC_M(*p));
  return v;
}

#define a_sc a_sc
static inline int a_sc(volatile int *p, int v) {
  int r;
  __asm__ __volatile__("sc.w %0, %1"
                       : "=r"(r), "=" LLSC_M(*p)
                       : "0"(v)
                       : "memory");
  return r;
}

#define a_ll_p a_ll_p
static inline void *a_ll_p(volatile void *p) {
  void *v;
  __asm__ __volatile__("ll.d %0, %1" : "=r"(v) : LLSC_M(*(void *volatile *)p));
  return v;
}

#define a_sc_p a_sc_p
static inline int a_sc_p(volatile void *p, void *v) {
  long r;
  __asm__ __volatile__("sc.d %0, %1"
                       : "=r"(r), "=" LLSC_M(*(void *volatile *)p)
                       : "0"(v)
                       : "memory");
  return r;
}

#define a_barrier a_barrier
static inline void a_barrier() {
  __asm__ __volatile__("dbar 0" : : : "memory");
}

#define a_pre_llsc a_barrier
#define a_post_llsc a_barrier

#undef LLSC_M

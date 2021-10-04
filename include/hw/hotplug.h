#ifndef HOTPLUG_H_NA2XQOM0
#define HOTPLUG_H_NA2XQOM0
#include <stdlib.h>
#include <string.h>

typedef struct HotplugHandler HotplugHandler;
typedef struct X86CPU X86CPU;
/**
 * hotplug_fn:
 * @plug_handler: a device performing plug/uplug action
 * @plugged_dev: a device that has been (un)plugged
 * @errp: returns an error if this function fails
 */
typedef void (*hotplug_fn)(HotplugHandler *handler, X86CPU *cpu);

/**
 * HotplugDeviceClass:
 *
 * Interface to be implemented by a device performing
 * hardware (un)plug functions.
 *
 * @parent: Opaque parent interface.
 * @pre_plug: pre plug callback called at start of device.realize(true)
 * @plug: plug callback called at end of device.realize(true).
 * @unplug_request: unplug request callback.
 *                  Used as a means to initiate device unplug for devices that
 *                  require asynchronous unplug handling.
 * @unplug: unplug callback.
 *          Used for device removal with devices that implement
 *          asynchronous and synchronous (surprise) removal.
 */
typedef struct HotplugHandlerClass {
  /* <private> */
  // InterfaceClass parent;

  /* <public> */
  hotplug_fn pre_plug;
  hotplug_fn plug;
} HotplugHandlerClass;

struct HotplugHandler {
  struct HotplugHandlerClass *hdc;
  void *parent;
  char *parent_type;
};

static inline void hotplug_check_cast(HotplugHandler *hd, char *type) {
  if (strcmp(hd->parent_type, type)) {
    exit(1);
  }
}

static inline void hotplug_handler_plug(HotplugHandler *hd, X86CPU *cpu) {
  HotplugHandlerClass *hdc = hd->hdc;
  if (hdc->plug) {
    hdc->plug(hd, cpu);
  }
}

static inline void hotplug_handler_pre_plug(HotplugHandler *hd, X86CPU *cpu) {
  HotplugHandlerClass *hdc = hd->hdc;
  if (hdc->plug) {
    hdc->pre_plug(hd, cpu);
  }
}

#endif /* end of include guard: HOTPLUG_H_NA2XQOM0 */

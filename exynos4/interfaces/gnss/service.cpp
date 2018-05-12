#define LOG_TAG "android.hardware.gnss@1.0-service.exynos4"

#define LEGACY_GPS_PATH "/vendor/lib/hw/gps.exynos4.so"

#include <android/hardware/gnss/1.0/IGnss.h>
#include <hidl/HidlTransportSupport.h>
#include <binder/ProcessState.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Gnss.h"

using android::sp;
using android::status_t;
using android::OK;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::gnss::V1_0::implementation::Gnss;

int main() {
    status_t status;
    android::sp<Gnss> service = nullptr;

    hw_module_t* module;
    void *legacyGpsLib;
    int err = 0;

    ALOGI("Gnss HAL Service 1.0 for Exynos4 is starting.");

    legacyGpsLib = dlopen(LEGACY_GPS_PATH, RTLD_LOCAL);
    if (!legacyGpsLib) {
	ALOGE("Failed to load real GPS HAL '" LEGACY_GPS_PATH "': %s", dlerror());
	err = -1;
    }

    if (err == 0) {
        module = (struct hw_module_t*)dlsym(legacyGpsLib, HAL_MODULE_INFO_SYM_AS_STR);
        if (!module) {
            ALOGE("Failed to locate the GPS HAL module sym: '" HAL_MODULE_INFO_SYM_AS_STR "': %s", dlerror());
            err = -1;
        }
    }

    if (err == 0) {
        hw_device_t* device;
        err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            service = new Gnss(reinterpret_cast<gps_device_t*>(device));
        } else {
            ALOGE("gnssDevice open %s failed: %d", GPS_HARDWARE_MODULE_ID, err);
            err = -1;
        }
    } else {
      ALOGE("gnss hw_get_module %s failed: %d", GPS_HARDWARE_MODULE_ID, err);
    }

    if (service == nullptr) {
        ALOGE("Can not create an instance of Gnss HAL Iface, exiting.");

        goto shutdown;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status = service->registerAsService();
    if (status != OK) {
        ALOGE("Could not register service for Gnss HAL Iface (%d).", status);
        goto shutdown;
    }

    ALOGI("Gnss Service is ready");
    joinRpcThreadpool();
    //Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to exit

    ALOGE("Gnss Service is shutting down");
    return 1;
}

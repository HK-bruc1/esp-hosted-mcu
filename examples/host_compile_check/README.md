# Host Compile Check

This is an ESP-IDF host-platform compile check for the local ESP-Hosted
component. It is not a runtime transport example: `app_main()` does not call
`esp_hosted_init()` and does not require a connected co-processor.

Use it when host-side component changes need an ESP-IDF build target, similar to
the `slave/` project build.

## Build

From this directory:

```bash
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" build
```

Transport-specific compile checks:

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.sdio" build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi_hd" build
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.uart" build
```

The selected target must support the requested transport in `Kconfig`.


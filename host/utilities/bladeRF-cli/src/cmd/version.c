#include "cmd.h"
#include "version.h"
#include <stdio.h>

int cmd_version(struct cli_state *state, int argc, char **argv)
{
    int status;

    struct bladerf_version fw_version, fpga_version, lib_version;
    bool fpga_loaded = false;

    bladerf_version(&lib_version);

    printf("\n");
    printf("  bladeRF-cli version:        " BLADERF_CLI_VERSION "\n");
    printf("  libbladeRF version:         %s\n", lib_version.describe);
    printf("\n");

    /* Exit cleanly if no device is attached */
    if (state->dev == NULL) {
        printf("  Device version information unavailable: No device attached.\n");
        return 0;
    }

    status = bladerf_is_fpga_configured(state->dev);
    if (status < 0) {
        state->last_lib_error = status;
        return CMD_RET_LIBBLADERF;
    } else if (status != 0) {
        fpga_loaded = true;
        status = bladerf_fpga_version(state->dev, &fpga_version);
        if (status < 0) {
            state->last_lib_error = status;
            return CMD_RET_LIBBLADERF;
        }
    }

    status = bladerf_fw_version(state->dev, &fw_version);
    if (status < 0) {
        state->last_lib_error = status;
        return CMD_RET_LIBBLADERF;
    }


    printf("  Firmware version:           %s\n", fw_version.describe);

    if (fpga_loaded) {
        printf("  FPGA version:               %s\n", fpga_version.describe);
    } else {
        printf("  FPGA version:               Unknown (FPGA not loaded)\n");
    }

    printf("\n");
    return CMD_RET_OK;
}


/* This is a simple program to query the system for how many bladeRF's are connected to the system
   compile using: gcc list_blades.c -o list_blades -lbladeRF
   PRF
 */

#include <libbladeRF.h>
#include <stdio.h>


int main( int arg_count, char **arg_vectors ) {
    /* we don't use any arguments in this program */
    struct bladerf_devinfo *available_devs = NULL;
    ssize_t num_available_devs;
    struct bladerf *dev = NULL;
    char *desired_dev_path;

    /* get the number of devices on the system */
    /* also creates a datastrucure of device info's into the pointer */
    num_available_devs = bladerf_get_device_list(&available_devs);

    if ( num_available_devs == 0 ) {
        printf("No BladeRF Devices Detected..\n");
    }

    /* now loop though all the devices and print information about them */
    int i;
    for ( i=0; i < num_available_devs; i++ ) {
        printf("BladeRF Device 0:\n");
        printf("    device path: %s\n", available_devs[i].path );
        printf("    bladeRF serial: %ld\n", available_devs[i].serial );
        printf("    fpga_configured: %d\n", available_devs[i].fpga_configured);
        printf("    fpga_version: %d.%d\n", available_devs[i].fpga_ver_maj, available_devs[i].fpga_ver_min);
        printf("    fw_version: %d.%d\n", available_devs[i].fw_ver_maj, available_devs[i].fw_ver_min);
        printf("\n");
    }

    /* don't forget to free the devicelist create by bladerf_get_device_list( ... ); */
    bladerf_free_device_list(available_devs, num_available_devs);
    return 0; /* success, no errors */
}


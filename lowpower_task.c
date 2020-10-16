/*******************************************************************************
 * File Name: lowpower_task.c
 *
 * Description: This file contains the functions for initializing the Wi-Fi
 * device, configuring the selected WLAN power save mode, connecting to the
 * AP, and suspending the network stack to enable higher power savings on
 * the PSoC 6 MCU.
 *
 * Related Document: See Readme.md
 *
 *******************************************************************************
 * (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * ("Software"), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software ("EULA").
 *
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress's integrated circuit products.
 * Any reproduction, modification, translation, compilation, or representation
 * of this Software except as specified above is prohibited without the express
 * written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "iot_wifi.h"
#include "iot_wifi_common.h"
#include "platform/iot_threads.h"
#include <stdbool.h>
#include "lowpower_task.h"
#include "lwip/netif.h"
#include "cyhal.h"
#include "cybsp.h"

/* Low Power Assistant header files. */
#include "network_activity_handler.h"

/* Wi-Fi Host Driver (WHD) header files. */
#include "whd_wifi_api.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define APP_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 8)
#define APP_TASK_PRIORITY       tskIDLE_PRIORITY

/*******************************************************************************
 * Function definitions
 ******************************************************************************/
void RunApplicationTask(void *pArgument);

/*******************************************************************************
* Function Name: InitApplication
********************************************************************************
* Summary:
*  Initializes all the tasks, queues, and other resources required to run the
*  application.
*
*******************************************************************************/
void InitApplication(void)
{
    /* Set up the application task */
    Iot_CreateDetachedThread(RunApplicationTask, NULL, APP_TASK_PRIORITY, APP_TASK_STACK_SIZE);
}

/*******************************************************************************
* Function Name: configure_wlan_powersave
********************************************************************************
*
* Summary:
*  This function configures the power-save mode of the WLAN device.
*  There are three power-save modes supported as defined in the enumeration
*  wlan_powersave_mode_t in the lowpower_task.h file.
*
* Parameters:
*  wifi: Pointer to the Wi-Fi network interface.
*  mode: The power save mode which is to be configured for the Wi-Fi interface.
*
* Return:
*  cy_rslt_t: Returns the status of configuring the WLAN interface's power-save mode.
*
*******************************************************************************/
static cy_rslt_t configure_wlan_powersave(struct netif *wifi, enum wlan_powersave_mode_t mode)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    whd_interface_t ifp;
    whd_security_t security_param;
    whd_bss_info_t ap_info;

    if (wifi->flags & NETIF_FLAG_UP)
    {
        /* Get the instance of the WLAN interface.*/
        ifp = (whd_interface_t)wifi->state;

        /* Obtain network parameters configured in the AP.*/
        result = whd_wifi_get_ap_info(ifp, &ap_info, &security_param);

        PRINT_AND_ASSERT(result, "Failed to get AP info.\n");

        APP_INFO(("Beacon period = %d, DTIM period = %d\n", ap_info.beacon_period, ap_info.dtim_period));

        /* Configure power-save mode of the WLAN device.*/
        switch (mode)
        {
        case POWERSAVE_WITHOUT_THROUGHPUT:
            result = whd_wifi_enable_powersave(ifp);

            if (CY_RSLT_SUCCESS != result)
            {
                ERR_INFO(("Failed to enable PM1 mode\n"));
            }

            break;
        case POWERSAVE_WITH_THROUGHPUT:
            result = whd_wifi_enable_powersave_with_throughput(ifp, RETURN_TO_SLEEP_MS);

            if (CY_RSLT_SUCCESS != result)
            {
                ERR_INFO(("Failed to enable PM2 mode\n"));
            }

            break;
        case POWERSAVE_DISABLED:
            result = whd_wifi_disable_powersave(ifp);

            if (CY_RSLT_SUCCESS != result)
            {
                ERR_INFO(("Failed to disable powersave\n"));
            }

            break;
        default:
            APP_INFO(("Unknown mode\n"));
            break;
        }
    }
    else
    {
        ERR_INFO(("Wi-Fi interface is not powered on. Failed to configure power-save mode\n"));
        result = CY_RSLT_TYPE_ERROR;
    }

    return result;
}

/*******************************************************************************
* Function Name: RunApplicationTask
********************************************************************************
* Summary:
*  This task initializes the Wi-Fi, and the OLM (Offload Manager) run by the LPA
*  (Low Power Assistant) middleware library. It suspends the lwIP network stack
*  indefinitely which helps RTOS to enter the Idle state, and then eventually
*  into deep-sleep power mode. The MCU will stay in deep sleep power mode till
*  the network stack resumes. The network stack resumes whenever any Tx/Rx
*  activity detected in the EMAC interface (path between Wi-Fi driver and
*  network stack).
*
* Parameters:
*  pArgument - Task argument (not used).
*
* Return:
*  void
*
*******************************************************************************/
void RunApplicationTask(void *pArgument)
{
    struct netif *wifi;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    (void)pArgument;

    /* Obtains the reference to the lwIP network interface. This will be used to
     * access the Wi-Fi driver interface to configure the WLAN power save mode.
     */
    wifi = cy_lwip_get_interface();

    /* Configures the WLAN device to a power-save mode set with the macro
     * WLAN_POWERSAVE_MODE.
     */
    result = configure_wlan_powersave(wifi, WLAN_POWERSAVE_MODE);

    PRINT_AND_ASSERT(result, "Failed to configure the requested WLAN power save mode.\n");

    while (true)
    {
        /* Configures an emac activity callback to the Wi-Fi interface and
         * suspends the network if the network is inactive for a duration of
         * NETWORK_INACTIVE_WINDOW_MS inside an interval of NETWORK_INACTIVE_INTERVAL_MS.
         * The callback is used to signal the presence/absence of network activity to
         * resume/suspend the network stack.
         */
        wait_net_suspend(wifi,
                         portMAX_DELAY,
                         NETWORK_INACTIVE_INTERVAL_MS,
                         NETWORK_INACTIVE_WINDOW_MS);

        vTaskDelay(pdMS_TO_TICKS(NETWORK_SUSPEND_DELAY_MS));
    }
}

/*******************************************************************************
 * Function Name: prvWifiConnect
 *******************************************************************************
 * Summary:
 *  The function initializes the Wi-Fi and then joins the Access Point with the
 *  provided WIFI_SSID, WIFI_PASSWORD, and WIFI_SECURITY type defined in the
 *  lowpower_task.h file.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  cy_rslt_t: Returns CY_RSLT_SUCCESS if the Wi-Fi connection is successfully
 *  established else, it returns CY_RSLT_TYPE_ERROR.
 *
 ******************************************************************************/
cy_rslt_t prvWifiConnect(void)
{
    WIFINetworkParams_t  xNetworkParams;
    WIFIReturnCode_t xWifiStatus;
    uint8_t ip_addr[4] = {0};
    uint32_t retry_count = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize Wi-Fi interface. */
    if (eWiFiSuccess != WIFI_On())
    {
        ERR_INFO(("Failed to initialize the Wi-Fi.\n"));
        configASSERT(0);
    }

    /* Setup Wi-Fi network parameters. */
    xNetworkParams.pcSSID = WIFI_SSID;
    xNetworkParams.ucSSIDLength = sizeof(WIFI_SSID);
    xNetworkParams.pcPassword = WIFI_PASSWORD;
    xNetworkParams.ucPasswordLength = sizeof(WIFI_PASSWORD);
    xNetworkParams.xSecurity = WIFI_SECURITY;

    APP_INFO(("Wi-Fi module initialized. Connecting to AP: %s\n", xNetworkParams.pcSSID));

    /* Connect to Access Point */
    for (retry_count = 0; retry_count < MAX_WIFI_RETRY_COUNT; retry_count++)
    {
        xWifiStatus = WIFI_ConnectAP(&(xNetworkParams));

        if (eWiFiSuccess == xWifiStatus)
        {
            APP_INFO(("Wi-Fi connected to AP: %s\n", xNetworkParams.pcSSID));

            xWifiStatus = WIFI_GetIP(ip_addr);

            if (eWiFiSuccess == xWifiStatus)
            {
                APP_INFO(("IP Address acquired: %s\n", ip4addr_ntoa((const ip4_addr_t *)&ip_addr)));
            }
            else
            {
                ERR_INFO(("Failed to get IP address.\n"));
            }

            break;
        }

        ERR_INFO(("Failed to join Wi-Fi network. Retrying...\n"));
    }

    if (eWiFiSuccess == xWifiStatus)
    {
        result = CY_RSLT_SUCCESS;
    }
    else
    {
        result = CY_RSLT_TYPE_ERROR;
    }

    return result;
}


/* [] END OF FILE */


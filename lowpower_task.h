/******************************************************************************
* File Name: lowpower_task.h
*
* Description: This file includes the macros and enumerations used by the
* example to connect to an AP, configure power save mode of the WLAN device,
* and configure the parameters for suspending the network stack.
*
* Related Document: See README.md
*
******************************************************************************
* Copyright (2020), Cypress Semiconductor Corporation.
******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
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
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/


/*******************************************************************************
* Include guard
*******************************************************************************/
#ifndef LOWPOWER_TASK_H_
#define LOWPOWER_TASK_H_

#include "cybsp.h"

/* Wi-Fi Credentials: Modify WIFI_SSID and WIFI_PASSWORD to match your Wi-Fi network
 * Credentials.
 */
#define WIFI_SSID                                "WIFI_SSID"
#define WIFI_PASSWORD                            "WIFI_PASSWORD"

/* Security type of the Wi-Fi access point. See 'WIFISecurity_t' structure
 * in "iot_wifi.h" for more details.
 */
#define WIFI_SECURITY                            eWiFiSecurityWPA2

#define MAX_WIFI_RETRY_COUNT                     (3)

/* Delay between subsequent calls to suspending the network stack.
 * This is a safe delay which helps in preventing the race conditions
 * that might occur when activating and de-activating the offload.
 */
#define NETWORK_SUSPEND_DELAY_MS                 (100)

/* This macro specifies the interval in milliseconds that the device monitors
 * the network for inactivity. If the network is inactive for duration lesser 
 * than INACTIVE_WINDOW_MS in this interval, the MCU does not suspend the network 
 * stack and informs the calling function that the MCU wait period timed out 
 * while waiting for network to become inactive.
 */
#define NETWORK_INACTIVE_INTERVAL_MS             (300)

/* This macro specifies the continuous duration in milliseconds for which the 
 * network has to be inactive. If the network is inactive for this duaration,
 * the MCU will suspend the network stack. Now, the MCU will not need to service
 * the network timers which allows it to stay longer in sleep/deepsleep.
 */
#define NETWORK_INACTIVE_WINDOW_MS               (200)

/* This macro defines the power-save mode that the WLAN device has to be
 * configured to. The valid values for this macro are POWERSAVE_WITH_THROUGHPUT,
 * POWERSAVE_WITHOUT_THROUGHPUT, POWERSAVE_DISABLED
 * which are defined in the enumeration wlan_powersave_mode_t.
 */
#define WLAN_POWERSAVE_MODE                      POWERSAVE_WITH_THROUGHPUT

/* This macro specifies the duration in milliseconds for which the STA stays
 * awake after receiving frames from AP in PM2 mode. The delay value must be set
 * to a multiple of 10 and not equal to zero. Minimum value is 10u. Maximum
 * value is 2000u.
 */
#define RETURN_TO_SLEEP_MS                       (10)

#define APP_INFO(x)                              do { configPRINTF(("Info: ")); configPRINTF(x); } while(0);
#define ERR_INFO(x)                              do { configPRINTF(("Error: ")); configPRINTF(x); } while(0);

#define PRINT_AND_ASSERT(result, msg, args...)   do                                 \
                                                 {                                  \
                                                     if (CY_RSLT_SUCCESS != result)    \
                                                     {                              \
                                                         ERR_INFO((msg, ## args));  \
                                                         configASSERT(0);           \
                                                     }                              \
                                                 } while(0);

/* This data type enlists enumerations that correspond to the different 
 * power-save modes available. They are,
 * POWERSAVE_WITHOUT_THROUGHPUT:This mode corresponds to (legacy) 802.11 PS-Poll
 * mode and should be used to achieve the lowest power consumption possible when
 * the Wi-Fi device is primarily passively listening to the network.
 *
 * POWERSAVE_WITH_THROUGHPUT:This mode attempts to increase throughput by
 * waiting for a timeout period before returning to sleep rather than returning
 * to sleep immediately.
 *
 * POWERSAVE_DISABLED: Powersave mode is disabled.
 */
enum wlan_powersave_mode_t
{
   POWERSAVE_WITHOUT_THROUGHPUT,
   POWERSAVE_WITH_THROUGHPUT,
   POWERSAVE_DISABLED
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
cy_rslt_t prvWifiConnect(void);
void RunApplicationTask(void *pArgument);

#endif /* LOWPOWER_TASK_H_ */


/* [] END OF FILE */


/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app.h"
#include "app_log.h"
#include "sl_sensor_rht.h" //to have access to the relative humidity and temperature sensor functions.
#include "temperature.h" //to have access to the BLE format conversion function.
#include <stdio.h>
#include "gatt_db.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  app_log_info("%s\n", __FUNCTION__);

}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log_info("%s: connection_opened!\n", __FUNCTION__);
      sl_sensor_rht_init();//Initialization of the relative humidity and temperature sensor

      break;


    case sl_bt_evt_gatt_server_user_read_request_id: // This event indicates that the client requested a read from a characteristic
      {//case doesn't know how to create its own variable
        // therefore we need to give a context with the brackets
        // in order to make a new declaration
        sl_bt_evt_gatt_server_user_read_request_t *request = &evt->data.evt_gatt_server_user_read_request; //Store the read request in a variable

        if (request->characteristic == gattdb_temperature) { // is the read request for the temperature characteristic?
                app_log_info("Read request for temperature characteristic received. Connection handle: %d\n", request->connection);
        }

        app_log_info("%s: Reading !\n", __FUNCTION__);
        int16_t TempBLE; //Initialization of the BLE temperature
        sl_status_t status = getconvTemptoBLE(&TempBLE);//We convert the temperature using the TemBLE address
        if (status == SL_STATUS_OK) {
            app_log_info("Temperature in BLE: %d\n", TempBLE);// if the conversion is successful, we display the temperature in BLE format
        }
        else{
            app_log_info("Reading failed ! %lu\n", (unsigned long )status);
        }
        sl_status_t new_st;
        //function to respond to a read access on a characteristic
        new_st = sl_bt_gatt_server_send_user_read_response(request->connection, //Connection handle
                                                           request->characteristic,//characteristic handle received in the sl_bt_evt_gatt_server_user_read_request event
                                                           0,//error code 0 means no errors
                                                           sizeof( TempBLE),
                                                           (uint8_t*) &TempBLE,
                                                            NULL); //length of data sent to the client
        if (new_st == SL_STATUS_OK) {//Check if the response has been received
                 app_log_info("Read response acquired \n");
            }
        else{
                 app_log_info("Read response failed ! %lu\n", (unsigned long )new_st);
               }
      }
      break;

      // This event indicates that the Client Configuration Characteristic Descriptor has changed
    case sl_bt_evt_gatt_server_characteristic_status_id:
              app_log_info("CCCD CHANGED !\n");
              sl_bt_evt_gatt_server_characteristic_status_t *char_st = &evt->data.evt_gatt_server_characteristic_status; //Store the characteristic status in a variable
              if (char_st->characteristic == gattdb_temperature) {// is the notify status linked to the temperature characteristic?
                             app_log_info("The Notify status is linked to the temperature \n");
                     }
              app_log_info("Flag status : %u\n",(uint8_t)char_st->status_flags);//The value of the status flag indicates if we clicked on the notiffy button
              if(char_st->status_flags==1){// is the notify button linked to the temperature characteristic?
                  app_log_info("The Notify button has been pressed for the temperature characteristic\n");
              }
              app_log_info("Notify received value : %u\n",(uint8_t)char_st->client_config_flags); //The value of the client config flag indicates if we received the confirmation of notifications reception from the client

              if(char_st->client_config_flags==1){
                 app_log_info("Notify reception confirmed !\n");
                 //sl_sleeptimer_timer_handle_t timer_handle;
                 //void callback_data=(timer_handle,char_st->client_config_flags);
                 //sl_status_t sl_sleeptimer_start_periodic_timer_ms(*timer_handle,
                  //                                                 1000,
                    //                                               callback_Notify,
                      //                                             void *callback_data,0,0);

                            }
              else{
                  app_log_info("Notify reception unconfirmed !\n");
              }
      break;


    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log_info("%s: connection_closed!\n", __FUNCTION__);
      sl_sensor_rht_deinit(); //deinitilization of the relative humidity and temperature sensor
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

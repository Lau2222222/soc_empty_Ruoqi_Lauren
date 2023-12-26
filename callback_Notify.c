/*
 * callback_Notify.c
 *
 *  Created on: 15 déc. 2023
 *      Author: Lauren
 */

#include "callback_Notify.h"

sl_sleeptimer_timer_callback_t callback_Notify(uint16_t client_config_flags, sl_sleeptimer_timer_handle_t *handle){
  if(client_config_flags==0){
      sl_status_t sl_sleeptimer_stop_timer(sl_sleeptimer_timer_handle_t *handle);
  }
}

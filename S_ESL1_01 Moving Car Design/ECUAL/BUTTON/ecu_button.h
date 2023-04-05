/* 
 * File:   ecu_button.h
 * Author: Mohamed Abdel-Wahab
 *
 * Created on February 18, 2023, 11:12 AM
 */

#ifndef ECU_BUTTON_H
#define	ECU_BUTTON_H

/**********************************  section 1: Includes ********************************************************/
#include "../../MCAL_layer/GPIO/hal_gpio.h"
#include "ecu_button_cfg.h"
/**********************************  section 2: Macro Declarations ***********************************************/

/**********************************  section 3: Macro Like Function Declarations *************************** *****/

/**********************************  section 4: Data Type Declarations  ******************************************/
typedef enum{
    BUTTON_PRESSED = 0,
    BUTTON_RELEASED
}button_status_t;

typedef enum{
    BUTTON_ACTIVE_HIGH = 0,
    BUTTON_ACTIVE_LOW
}button_active_t;

typedef struct{
    pin_config_t    button_pin;
    button_status_t button_state;
    button_active_t button_connection;
}button_t;
/**********************************  section 5: Software Interfaces Declarations ********************************************/


/**
 * @breif Initialize The assigned pin to be input 
 * @param btn he reference of the button module configuration
 * @return status of the function 
 *          E_OK :the function done successfully
 *          E_NOT_OK :the function has issues performing the function
 */
Std_ReturnType BTN_init(const button_t *btn_ptr);

/**
 * @breif Read the push button if is it pressed or released
 * @param btn    The reference of the button module configuration
 * @param btn_state  The reference of the variable that store the button status @ref button_status_t
 * @return status of the function 
 *          E_OK :the function done successfully
 *          E_NOT_OK :the function has issues performing the function
 */
Std_ReturnType BTN_read_state(const button_t *btn_ptr, button_state_t *btn_state_ptr);


#endif	/* ECU_BUTTON_H */


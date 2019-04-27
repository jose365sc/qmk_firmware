/* Copyright 2019 jose365
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <print.h>
#include "i2c_master.h"
#include "simplysplit.h"

void keyboard_post_init_kb(void) {
  debug_enable = true;
  debug_matrix = true;
  debug_keyboard = true;
  print("debug set!\n");

  keyboard_post_init_user();
}
void matrix_init_kb(void) {
	// put your keyboard start-up code here
	// runs once when the firmware starts up
  print("matrix set!\n");
  i2c_init();

	matrix_init_user();
}

void matrix_scan_kb(void) {
	// put your looping keyboard code here
	// runs every cycle (a lot)

	matrix_scan_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
	// put your per-action keyboard code here
	// runs for every action, just before processing by the firmware
  print("yes!\n");
  uint8_t buf[16];
  i2c_readReg(0x40, 0x02, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x40, 0x04, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x40, 0x06, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x40, 0x40, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x40, 0x44, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x44, 0x02, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x44, 0x04, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x44, 0x06, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x44, 0x40, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
  i2c_readReg(0x44, 0x44, buf, 1, 1000);
  uprintf("yes! %d\n", buf[0]);
	return process_record_user(keycode, record);
}

void led_set_kb(uint8_t usb_led) {
	// put your keyboard LED indicator (ex: Caps Lock LED) toggling code here

	led_set_user(usb_led);
}

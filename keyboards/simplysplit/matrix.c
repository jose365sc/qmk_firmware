/*
Copyright 2012-2018 Jun Wako, Jack Humbert, Yiancar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdint.h>
#include <stdbool.h>
#include "wait.h"
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "quantum.h"
#include "i2c_master.h"
#include "pcal9555.h"

i2c_status_t i2c_writeReg_byte(uint8_t devaddr, uint8_t regaddr, uint8_t data, uint16_t timeout)
{
    return i2c_writeReg(devaddr, regaddr, &data, 1, timeout);
}

#if (MATRIX_COLS <= 8)
#    define print_matrix_header()  print("\nr/c 01234567\n")
#    define print_matrix_row(row)  print_bin_reverse8(matrix_get_row(row))
#    define matrix_bitpop(i)       bitpop(matrix[i])
#    define ROW_SHIFTER ((uint8_t)1)
#elif (MATRIX_COLS <= 16)
#    define print_matrix_header()  print("\nr/c 0123456789ABCDEF\n")
#    define print_matrix_row(row)  print_bin_reverse16(matrix_get_row(row))
#    define matrix_bitpop(i)       bitpop16(matrix[i])
#    define ROW_SHIFTER ((uint16_t)1)
#elif (MATRIX_COLS <= 32)
#    define print_matrix_header()  print("\nr/c 0123456789ABCDEF0123456789ABCDEF\n")
#    define print_matrix_row(row)  print_bin_reverse32(matrix_get_row(row))
#    define matrix_bitpop(i)       bitpop32(matrix[i])
#    define ROW_SHIFTER  ((uint32_t)1)
#endif

#ifdef MATRIX_MASKED
    extern const matrix_row_t matrix_mask[];
#endif

static bool need_init_pca9555;
/* matrix state(1:on, 0:off) */
static matrix_row_t raw_matrix[MATRIX_ROWS]; //raw values
static matrix_row_t matrix[MATRIX_ROWS]; //debounced values

__attribute__ ((weak))
void matrix_init_quantum(void) {
    matrix_init_kb();
}

__attribute__ ((weak))
void matrix_scan_quantum(void) {
    matrix_scan_kb();
}

__attribute__ ((weak))
void matrix_init_kb(void) {
    matrix_init_user();
}

__attribute__ ((weak))
void matrix_scan_kb(void) {
    matrix_scan_user();
}

__attribute__ ((weak))
void matrix_init_user(void) {
}

__attribute__ ((weak))
void matrix_scan_user(void) {
}

inline
uint8_t matrix_rows(void) {
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void) {
    return MATRIX_COLS;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    // Matrix mask lets you disable switches in the returned matrix data. For example, if you have a
    // switch blocker installed and the switch is always pressed.
#ifdef MATRIX_MASKED
    return matrix[row] & matrix_mask[row];
#else
    return matrix[row];
#endif
}

void matrix_print(void)
{
    print_matrix_header();

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row); print(": ");
        print_matrix_row(row);
        print("\n");
    }
}

uint8_t matrix_key_count(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        count += matrix_bitpop(i);
    }
    return count;
}

static bool read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row)
{
    // Store last value of row prior to reading
    matrix_row_t last_row_value = current_matrix[current_row];

    // Clear data in matrix row
    current_matrix[current_row] = 0;

    // if pca9555 is in error state, clear matrix and skip reading
    if (need_init_pca9555)
    {
        return (last_row_value != current_matrix[current_row]);
    }

    // Select row and wait for row selecton to stabilize
    if (i2c_writeReg_byte(PCAL9555_ADDR_0, PCAL9555_REG_OUT_PORT1, ~(0x1 << current_row), PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_0, PCAL9555_REG_CFG_PORT1, ~(0x1 << current_row), PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_1, PCAL9555_REG_OUT_PORT1, ~(0x1 << current_row), PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_1, PCAL9555_REG_CFG_PORT1, ~(0x1 << current_row), PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS)
    {
        need_init_pca9555 = true;
        return (last_row_value != current_matrix[current_row]);
    }

    // For each col...
    uint8_t col0, col1;
    if (i2c_readReg(PCAL9555_ADDR_0, PCAL9555_REG_IN_PORT0, &col0, 1, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_readReg(PCAL9555_ADDR_1, PCAL9555_REG_IN_PORT0, &col1, 1, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS)
    {
        need_init_pca9555 = true;
        return (last_row_value != current_matrix[current_row]);
    }

    col0 = ~col0;
    col1 = ~col1;
    current_matrix[current_row] |= (((matrix_row_t)col1) << MATRIX_COLS_PER_SIDE) | ((matrix_row_t)col0);

    // Unselect row
    if (i2c_writeReg_byte(PCAL9555_ADDR_0, PCAL9555_REG_CFG_PORT1, 0xff, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_1, PCAL9555_REG_CFG_PORT1, 0xff, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS)
    {
        current_matrix[current_row] = 0;
        need_init_pca9555 = true;
        return (last_row_value != current_matrix[current_row]);
    }

    return (last_row_value != current_matrix[current_row]);
}

bool PCA9555_init(void) {
    // initialize key pins
    return !(
        i2c_writeReg_byte(PCAL9555_ADDR_0, PCAL9555_REG_CFG_PORT0, 0xff, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_0, PCAL9555_REG_CFG_PORT1, 0xff, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_1, PCAL9555_REG_CFG_PORT0, 0xff, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS ||
        i2c_writeReg_byte(PCAL9555_ADDR_1, PCAL9555_REG_CFG_PORT1, 0xff, PCAL9555_TIMEOUT) != I2C_STATUS_SUCCESS
    );
}

void matrix_init(void) {
#ifndef OLED_DRIVER_ENABLE
    i2c_init();
#endif

    setPinOutput(D7);

    // initialize key pins
    need_init_pca9555 = !PCA9555_init();
    if (need_init_pca9555)
        writePinLow(D7);
    else
        writePinHigh(D7);

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        raw_matrix[i] = 0;
        matrix[i] = 0;
    }

    debounce_init(MATRIX_ROWS);

    matrix_init_quantum();
}

uint8_t matrix_scan(void)
{
    // initialize key pins
  if (need_init_pca9555)
    need_init_pca9555 = !PCA9555_init();
  if (need_init_pca9555)
      writePinLow(D7);
  else
      writePinHigh(D7);

  bool changed = false;

  // Set row, read cols
  for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
    changed |= read_cols_on_row(raw_matrix, current_row);
  }

  debounce(raw_matrix, matrix, MATRIX_ROWS, changed);

  matrix_scan_quantum();
  return 1;
}

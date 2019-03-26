#include QMK_KEYBOARD_H
#include "action_layer.h"
#include "eeconfig.h"

extern keymap_config_t keymap_config;

#define _QWERTY 0
#define _LOWER 1
#define _RAISE 2
#define _ADJUST 16

enum custom_keycodes {
  QWERTY = SAFE_RANGE,
  LOWER,
  RAISE,
  ADJUST,
};

#define EISU LALT(KC_GRV)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [_QWERTY] = LAYOUT(  \
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    LOWER,                          LOWER,   KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, \
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_QUOT,                        KC_BSLS, KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQL,  \
    KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_PSCR,                        KC_DEL,  KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_LBRC, \
    KC_ESC,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    _______,                        _______, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RBRC, \
    KC_LCTL, KC_LGUI, KC_TAB,  KC_LALT,          KC_LCTL, KC_SPC,KC_LSFT,          KC_BSPC,KC_ENT, LOWER,            KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT  \
  ),

  [_LOWER] = LAYOUT(  \
    KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   LOWER,                          LOWER,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_MINS, \
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_QUOT,                        KC_BSLS, KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQL,  \
    KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_NLCK,                        KC_INS,  KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_LBRC, \
    KC_ESC,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    _______,                        _______, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RBRC, \
    KC_LCTL, KC_APP, KC_TAB,  KC_LALT,          KC_LCTL, KC_SPC,KC_LSFT,          KC_BSPC,KC_ENT, LOWER,             KC_HOME, KC_PGUP, KC_PGDN, KC_END  \
  )

#if 0
  [_LOWER] = LAYOUT( \
    KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F11,                         KC_F12,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,   KC_HOME, \
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_LBRC,                        KC_RBRC, KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_END,  \
    KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_QUOT,                        KC_BSLS, KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_PGUP, \
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    _______,                        _______, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_PGDN, \
    KC_LCTL, KC_APP,  KC_LALT, KC_NLCK,          LOWER,   KC_BSPC,KC_SPC,          KC_SPC,KC_ENT,  KC_INS,           KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT  \
  )
#endif
};

#ifdef AUDIO_ENABLE
float tone_qwerty[][2]     = SONG(QWERTY_SOUND);
#endif

void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case QWERTY:
      if (record->event.pressed) {
         print("mode just switched to qwerty and this is a huge string\n");
        set_single_persistent_default_layer(_QWERTY);
      }
      return false;
      break;
    case LOWER:
      if (record->event.pressed) {
        layer_on(_LOWER);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_LOWER);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      }
      return false;
      break;
    case RAISE:
      if (record->event.pressed) {
        layer_on(_RAISE);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_RAISE);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      }
      return false;
      break;
    case ADJUST:
      if (record->event.pressed) {
        layer_on(_ADJUST);
      } else {
        layer_off(_ADJUST);
      }
      return false;
      break;
  }
  return true;
}

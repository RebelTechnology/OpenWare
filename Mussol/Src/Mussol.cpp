#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "device.h"
#include "errorhandlers.h"

extern "C"{
void usbd_audio_mute_callback(int16_t gain){
}

void usbd_audio_gain_callback(int16_t gain){
}
}

#include <adc/adc.h>
#include <conversions/conversions.h>
#include <spi/spi.h>
#include <uart/uart.h>
#include <stdlib.h>

#include "../../src/devices.h"

#define NUM_SAMPLES_PER_CHANNEL 10
#define PGM_AUTO1_REG 0x8000

char* names[16] = {"THM BATT1", "THM BATT1", "THM PAY CONN", "VMON 5V", "IMON X PLUS", "IMON X MINUS", "IMON Y PLUS", "IMON Y MINUS", "IMON 5V", "THM 5V TOP", "VMON 3V3", "IMON 3V3", "THM 3V3 TOP", "VMON PACK", "IMON PACK", "IMON PAY LIM"};

void print_voltage(char* name, uint8_t channel, uint16_t raw_data) {
    double ch_voltage = adc_raw_to_ch_vol(raw_data);
    double voltage = adc_raw_to_circ_vol(raw_data, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);
    // String is left-justified, minimum 40 width
    print("%-15s, %u, 0x%.3x, %.3f V, %.3f V\n",
            name, channel, raw_data, ch_voltage, voltage);
}

/*
void read_current(char* name, uint8_t channel, double sense_res, double ref_vol) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double ch_voltage = adc_raw_to_ch_vol(raw_data);
    double current = adc_raw_to_circ_cur(raw_data, sense_res, ref_vol);
    print("%-15s, %u, 0x%.3x, %.3f V, %.3f A\n",
            name, channel, raw_data, ch_voltage, current);
}

void read_therm(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double ch_voltage = adc_raw_to_ch_vol(raw_data);
    double temp = adc_raw_to_therm_temp(raw_data);
    print("%-15s, %u, 0x%.3x, %.3f V, %.3f C\n",
            name, channel, raw_data, ch_voltage, temp);
} */

/* Compute the average error between subsequent values in an array data of length len
    Args:
        data(uint16_t*):        An array of numbers
        len(uint8_t):           Length of the array
    Returns:
        average_error(float):   Average error between values in the array data
*/
float compute_average_error(uint16_t* data, uint8_t len){
    float total_error = 0;
    for (uint8_t i = 0; i < len - 1; i++){
        total_error += abs((int)(data[i]) - (int)(data[i+1]));
    }
    float average_error = total_error / (float)(len-1);
    return average_error;
}

/* compute the average */
uint16_t compute_average(uint16_t* data, uint8_t len){
    uint32_t total = 0;
    for (uint8_t i = 0; i < len; i++){
        total += data[i];
    }
    total /= len;
    return (uint16_t)(total);
}

/* Set the AUTO1 channels on the ADC

    Args:
        channels(uint16_t):     Channel selection, where a '1' corresponds to a channel to be enabled and vice-versa
    Returns: Nothing
*/
void set_adc_auto1_channels(uint16_t channels){

    (&adc)->auto_channels = channels;

    uint16_t f1 = PGM_AUTO1_REG;//cmd of changing mode to auto-mode- where it fetches all channels
    uint16_t f2 = (&adc)->auto_channels;
    send_adc_frame(&adc, f1);
    send_adc_frame(&adc, f2);
}

/*  Measure the steady state noise on all channels. This is the noise present when the ADC isn't switching channels or doing anything fancy.
    Affected by:
        - Ambient noise on the PCB
        - Jitter in the ADC readings

    A good baseline is 1 LSB error, or 1.2 mV
*/
void measure_steady_state_noise(void){
    uint16_t data[NUM_SAMPLES_PER_CHANNEL] = {0};
    uint16_t errors[16];

    // select a channel
    for(uint8_t i = 0; i < 16; i++){
        set_adc_auto1_channels(_BV(i));

        // take NUM_SAMPLES_PER_CHANNEL measurements
        for (uint8_t j = 0; j < NUM_SAMPLES_PER_CHANNEL; j++){
            fetch_all_adc_channels(&adc);
            data[j] = (&adc)->channel_data[i];
            print_voltage(names[i], i, data[j]);
        }
        errors[i] = (uint16_t)(compute_average_error(data, NUM_SAMPLES_PER_CHANNEL));
    }
    print("\n STEADY STATE NOISE PER CHANNEL:\n");
    for(uint8_t i = 0; i < 16; i++){
        print_voltage(names[i], i, errors[i]);
    }
}


/* Get the average reading for a channel. Assumed to be steady-state */
uint16_t measure_settling_noise_avg_helper(uint8_t channel){
    uint16_t data[NUM_SAMPLES_PER_CHANNEL] = {0};
    uint16_t channel_avg = 0;

    set_adc_auto1_channels(_BV(channel));
    for (uint8_t i = 0; i < NUM_SAMPLES_PER_CHANNEL; i++){
        fetch_all_adc_channels(&adc);
        data[i] = (&adc)->channel_data[channel];
        print_voltage(names[channel], channel, data[i]);
    }
    channel_avg = compute_average(data, NUM_SAMPLES_PER_CHANNEL);
    
    return channel_avg;
}

/*  Measure ADC settling noise. This noise is caused by switching from an input at a high voltage to an input at low voltage, and vice versa. 
    Affected by:
        - Any capacitances on the input of the ADC
        - Slew rate and response time of the unity-gain buffer

    Sets the ADC to take measurements from channel_low to channel_high, settling error is measured against channel_high. channel_high > channel_low.
*/
void meausure_settling_noise(uint8_t channel_low, uint8_t channel_high){
    uint16_t channel_high_avg = 0;

    uint16_t differences[NUM_SAMPLES_PER_CHANNEL] = {0};
    uint16_t avg_diff = 0;

    channel_high_avg = measure_settling_noise_avg_helper(channel_high);

    for (uint8_t i = 0; i < NUM_SAMPLES_PER_CHANNEL; i++){
        set_adc_auto1_channels(_BV(channel_low) | _BV(channel_high));
        fetch_all_adc_channels(&adc);
        differences[i] = abs((&adc)->channel_data[channel_high] - channel_high_avg);
        print_voltage("Settling noise", i, differences[i]);
    }
    
    avg_diff = (uint16_t)(compute_average(differences, NUM_SAMPLES_PER_CHANNEL));

    print_voltage("Avg noise", 0, avg_diff);

}

// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    init_adc(&adc);
    print("ADC Initialized\n");

    print("\nStarting test\n\n");

    print("\nSPI SPEED %ul kHz", (uint32_t)((F_CPU/64)/1000));

    // compute the average steady-state noise for each channel
    print("\nMeasuring Steady State Noise\n");
    measure_steady_state_noise();
    // compute the average settling noise for two channels
    // print("\nMeasuring settling noise\n");
    // meausure_settling_noise(0, 1);

    set_spi_clk_freq(SPI_FOSC_128);
    print("\nSPI SPEED %ul kHz", (uint32_t)((F_CPU/128)/1000));

    // compute the average steady-state noise for each channel
    print("\nMeasuring Steady State Noise\n");
    measure_steady_state_noise();
    // compute the average settling noise for two channels
    // print("\nMeasuring settling noise\n");
    // meausure_settling_noise(0, 1);

    return 0;
}

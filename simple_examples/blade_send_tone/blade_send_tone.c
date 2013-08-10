/* Simple program to produce a tone at N Hz from TX LO Center of a bladeRF
 * Peter Fetterer (kb3gtn)
 *
 * Some Theory:
 *    The transmiter takes complex samples.  These samples have 2 parts.  A in-phase component (I)
 *  and a Quadature (-90 degrees) phase component.  Each sampling being a point on a x+jy  plane.
 *
 *  Some reading: http://www.ieee.li/pdf/essay/quadrature_signals.pdf
 *
 *  Since our goal is to make a tone that could be positive in frequency or negitive in frequency from
 *  our TX LO reference frequency.  From your reading above you should have gotten that e^jf  is basiclly
 *  just a sinewave in the real domain. (see Representing complex signas as real signals in paper above)
 *
 *  So given that e^jf = cos(f)/2 + jsin(f)/2    (f is radians per second or samples in our case)
 *  we want to generate a I of 0.5*cos(f) and a Q of 0.5*sin(f) to make a single tone of COS(f) in real domain.
 *
 *  If you plot X=cos(Phase) and Y=sin(Phase) for all 0 -> 2*pi, you see that each X/Y pair propogates around in a circle.
 *  If you are spinning in the counter-clock wise direction you have a positive frequency, because the generated signal
 *  transistions more radians per sample than the reference.  If the spin is clock-wise, the frequency produced will
 *  be less then the reference frequency. (aka negitive frequency)
 *
 *  So in summation, to make a positive frequency tone, make I-ch a cosine wave of the desired frequency, and Q-ch
 *  a sinewave of the desired frequency.   Invert Q to make the frequency negitive..
 *
 *  If you want a cosine wave at reference frequency, just set I to 1 and Q to 0, for all samples
 *  If you want a sine wave at reference frequency, just set I to 0 and Q to 1, for all samples
 *  cos(0) = 1,  sin(0) = 0
 *  sine_wave is +90 degrees, thus cos(pi/2)=0 sin(pi/2)=1
 *
 ***********************************************************
 * Compile using:
 * gcc blade_send_tone.c -o blade_send_tone -O3 -ffast-math -mfpmath=387 -lm -lbladeRF
 *
 */

#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <math.h>

/* for allocating memory for storing our samples we want to send to the bladeRF */
#define samples_per_buffer 1024
/* int16_t count per buffer */
#define sample_buffer_size samples_per_buffer*2
/* sample rate in Hz */
#define sample_rate 8000000
/* we will have a fixed LO Frequency for this test.. 1.2 GHz */
#define LO_FREQ 1200000000

/* global int for process state */
int isRunning = 1;

/* stuff for computing sin and cos fast on GCC platforms */
/* should compile down and use the fsincos asm instruction on intel platforms */
struct Sin_cos {
    double sin;
    double cos;
};

/* fast math synthisizable Sin_Cos ( intel ASM fsincos instruction ) */
/* computes the sin and cos in for val in radians */
/* falls back on OS math.h routine if hardware not capable, or in port ot other arch */
/* with GCC you will need the following flags to get it to go:
 *   -O3 -ffast-math -mfpmath=387 */
/* Computing it using the OS sin()/cos() functions can take 200+ clock cycles.
 *    this method takes ~3, if its working. */
struct Sin_cos fsincos(double val);


struct sample_generation_state {
  double radian_rate;  /* frequency in radians / sample */
  double phase;        /* what was the phase we generated was */
};

void generate_samples( int16_t *sample_buffer, struct sample_generation_state *state, int sample_count );
void setup_bladerf_common(struct bladerf *blade);
 
/* process return code from bladeRF calls */ 
void test_rc( int rc ) {
  if (rc < 0) {
      printf("BladeRF api call returned error %d, exiting..\n", rc );
      printf("Error: %s\n", bladerf_strerror(rc) );
      exit(-1); /* terminate program */
  }
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum) {
   printf("Caught signal %d, shutting down.\n",signum);
   /* signal to stop application */
   isRunning = 0;
}

/* prints the fancy /-\| sequence showing it's alive..  */
void update_spinner( int *state ) {
    printf("[1K");
    switch (*state) {
        case 0: printf("|"); *state = 1; break;
        case 1: printf("/"); *state = 2; break;
        case 2: printf("-"); *state = 3; break;
        case 3: printf("\\"); *state= 0; break;
        default: state = 0;
    }
    printf("[1D");
    fflush(stdout);
} 


/* this program takes 2 inputs from the command line, the device to use, and frequency to generate */
int main( int arg_count, char **arg_array ) {
    int rc;  /* return code from bladeRF calls */

    /* register our ctrl-c handler with OS*/
    signal(SIGINT, signal_callback_handler);

    /* this program expects 2 arguments, the device to use, and the frequency to generate */
    /* 3 because arg_count includes program name as first arguments.. (1 starting index) */
    if ( arg_count != 3 ) {
        /* This program was called none, or more than one argument */
        /* arg_array[0] is the executable name */
        /* gently remind use of the correct usage.. */
        printf("correct usage: %s <device to query>  <frequency_Hz>", arg_array[0] );
        printf(" where device is something like /dev/bladeRF0\n");
        printf(" where frequency_Hz is the offset from carrier +/-%f\n", sample_rate/4.0 );
        /* terminate execution and return -1 (generic error) to OS */
        printf(" DEBUG: Got %d arguments..\n", arg_count );
        exit(-1);
    }

    /* convert arg_array[2] (frequency) to from a text string to a real floating point number */
    double frequency;
    /* simple stdlib method for string to double conversion.
       returns 0 if it can't figure out what frequency you put in.. */
    frequency = strtod( arg_array[2], NULL );

    /* need an empty pointer for bladerf_open to assign to a bladerf data structure it creates */
    struct bladerf *blade = NULL;

    /* call to open the device we want, returns a pointer to a data structure we use
     * for all following calls..  Sorta like a device handle (windows talk) */
    /* Tell libbladeRF to try to open the device in arg_array[1] */
    blade = bladerf_open( arg_array[1] );

    /* if the open fails, blade will remain NULL or 0 */
    if ( blade == NULL ) {
        /* failed to open the device, tell use, and exit to OS */
        printf("%s failed to open the device %s\n", arg_array[0], arg_array[1] );
        exit(-1);
    }

    /* ok parameter count is good. */
    /* print what we got in and will attempt to open */
    printf("%s using device at %s\n", arg_array[0], arg_array[1] );
    printf("generating user frequency of %.3f Hz\n", frequency);

    /* got here, device must have been opened successfully */
    printf("Device %s opened successfully\n", arg_array[1] );

    rc = bladerf_is_fpga_configured( blade );
    test_rc(rc);

    if (!rc) {
        fprintf( stderr, "Error: bladeRF FPGA hasn't been loaded yet!\n");
        bladerf_close( blade );
        exit( EXIT_FAILURE );
    }

    /***********************************************************
     * Now we get to do stuff with the bladeRF we have opened  *
     ***********************************************************/

    /* settings configuration tracking object, used for alot of bladeRF calls */
    bladerf_module bm = TX;

    /* enable the bladeRF module, fills in bm struct above with inital data */
    test_rc( bladerf_enable_module( blade, bm, true) );

    /* setup LO freqency on bladeRF */
    test_rc( bladerf_set_frequency ( blade, bm, LO_FREQ ) );

    /* XXX
     *
     * Setup gains on TX side
     *
     * I've just pixed some arbitraty values here as I don't have a good feel
     * yet of what's a good value for example code -- mIKEjONES or bpadalino
     * would be the Nuand guys on IRC to ask on IRC. (Or perhaps tnt, horizon,
     * or BzztPloink?)
     *
     * To be honest, I got this configuration from bpadalino... I'm a C guy
     * new to the RF/SDR realm, so I can't speak much to the rationale for this
     * setup
     */
    test_rc( bladerf_set_loopback( blade, LNA_BYPASS) );

    /* tx VGA1 -30 to -4 in 1 dB steps */
    test_rc( bladerf_set_txvga1( blade, -10 ) );
    /* tx VGA2 0 to 25 in 1 dB steps */
    test_rc( bladerf_set_txvga2( blade, 10 ) );

    /* sets up the lms chip on the bladeRF*/
    setup_bladerf_common( blade );
 

    /* for this example, we will just setup for samplerate and bandwidth. */
    unsigned int actual_samplerate;
    test_rc( bladerf_set_sample_rate( blade, bm, sample_rate, &actual_samplerate ) );
    printf("requested %f Msps, was givin %f Msps\n", sample_rate / 1000000.0, actual_samplerate / 1000000.0 );

    /* set baseband bandwidth to sample_rate/2 Hz ( I Bandwidth = Q Bandwidth ) */
    unsigned int actual_bandwidth;
    test_rc( bladerf_set_bandwidth( blade, bm, sample_rate/2, &actual_bandwidth ) );
    printf("requested %f MHz BW, was givin %f MHz\n", sample_rate/2000000.0, actual_bandwidth / 1000000.0 );

    /* create a buffer of samples to send */
    /* samples are 16bit I followed by 16bits Q  (2's compliment signed numbers) */
    int16_t *sample_buffer = (int16_t*) malloc( sizeof(int16_t)*sample_buffer_size );

    /* sample generation state data structure */
    struct sample_generation_state gen_state;

    /* initalize with values */
    /* freq = cycles/second * 2*pi -> radians/second / (samples/second) -> radians / sample_period -> radian_rate */
    gen_state.radian_rate = ((frequency*6.28318530) / (sample_rate));  /* angular radian change per sample */
    gen_state.phase = 0.0;  /* starting at phase 0 */
    printf(" radian_rate = %f\n", gen_state.radian_rate );

    /******************************/
    /* now the magical main loop  */
    /******************************/

    /* isRunning defines if the loop keeps going, gets set to 0 to exit and shutdown */
    printf(" Main Loop Starting.. sending samples..\n");
    sleep(1);

    /* when debuging, print header stuff for sample table */
    //printf(" PHASE       [ I  ,  Q ]  ( i sample, q sample )\n");

    /* this runs until a signal (ctrl-c) is received by the is application */
    /* this causes isRunning to become 0, initially it is set to 1 */
    int loop_count = 0;
    int spinner_state = 0;
    int samples_written = 0;
    while (isRunning) {
        generate_samples( sample_buffer, &gen_state, samples_per_buffer );
        // send can return an error if there is a board problem, so we check it.
        samples_written = bladerf_send_c16( blade, sample_buffer, samples_per_buffer );
        if ( samples_written != samples_per_buffer ) {
            if ( samples_written < 0 ) {
                test_rc( samples_written );
            } else {
                printf("Under-run, short write of %d samples..\n", samples_written );
            }
        }
            
        // do it again, untill isRunning goes to zero..  (on ctrl-c)
        if ( loop_count > 1000 ) {
            update_spinner( &spinner_state );
            loop_count=0;
	    // stop when debugging..
            //isRunning = 0;
        } else {
            loop_count++;
        }
    }

    /***********************************************************
     * Clean up time...
     ***********************************************************/
    /* disable and close the blade RF */
    bladerf_enable_module( blade, bm, false);
    bladerf_close( blade );

    /* free sample_buffer we created */
    free(sample_buffer);

    printf("Shutdown Complete.\n");
    return 0;
}

/* compute next samples for this buffer based on state information */
void generate_samples( int16_t *sample_buffer, struct sample_generation_state *state, int sample_count ) {
    int s;
    struct Sin_cos i_q_values;  /* sin/cos lookup results */
    // for each I/Q sample do..
    for ( s = 0; s < sample_count; s=s+2 ) {
        // update state to this sample from last sample.
        state->phase = state->phase + state->radian_rate;
        /* clamp to 0 -> 2*pi range */
        if ( state->phase > 6.28318530 ) {
            state->phase = state->phase - 6.2831853;
        }
        // compute new I/Q value given new phase.
        // values results are double in the -1 -> +1 range
        i_q_values = fsincos( state->phase );
        // add to sample buffer
        /* make doubles into signed int by scaling by 2046 */
        /* dac samples are 12 bits in size.. we just hold them in a 16bit variable */
        sample_buffer[s]   = 0xa000 | (int16_t)( i_q_values.cos * 2000 ); 
        sample_buffer[s+1] = 0x5000 | (int16_t)( i_q_values.sin * 2000 );
 
        // print out sample information for debugging..
	// printf("phase=%f [ %-.6f, %-.6f ]    ( 0x%03x, 0x%03x )\n", state->phase, i_q_values.cos, i_q_values.sin, sample_buffer[s], sample_buffer[s+1] );

        // repeat until buffer is full
    }
    // return to calling function..
}

/* compute the values of sin(x) and cos(x) where val is in radians */
struct Sin_cos fsincos(double val) {
  struct Sin_cos r;
  r.sin = sin(val);
  r.cos = cos(val);
  return r;
}

/* directly stolen from gr-osmosdr */
void setup_bladerf_common( struct bladerf *blade ) {
    gpio_write( blade, 0x57 ); /* enable LMS RX & TX, select lower band */
    lms_spi_write( blade, 0x5a, 0xa0 ); /* polarity of the IQSel signal */
    /* values are taken from LMS6002D FAQ 5.27 */
    lms_spi_write( blade, 0x05, 0x3e ); /* enable the tx and rx modules */
    lms_spi_write( blade, 0x47, 0x40 ); /* Improving Tx spurious emission performance */
    lms_spi_write( blade, 0x59, 0x29 ); /* Improving ADC's performance */
    lms_spi_write( blade, 0x64, 0x36 ); /* Common Mode Voltage For ADC's */
    lms_spi_write( blade, 0x79, 0x37 ); /* Higher LNA Gain */
    return;
}


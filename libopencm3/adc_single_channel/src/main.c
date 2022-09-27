/**
 * @file   main.c
 * @brief  Single channel ADC example for LibOpenCM3 with STM32.
 * @author ZiTe (honmonoh@gmail.com)
 * @copyright MIT License
 */

#include "main.h"

int main(void)
{
  rcc_setup();
  adc_setup();
  usart_setup();

  printf("ADC single channel ready.\r\n");

  while (1)
  {
    uint16_t adc_value = get_adc_value(0);
    printf("%4d\r\n", adc_value);
    delay(2000000);
  }

  return 0;
}

static uint16_t get_adc_value(int channel)
{
  /* Setup channel. */
  uint8_t adc_channel[16];
  adc_channel[0] = channel;
  adc_set_regular_sequence(ADC1, 1, adc_channel);

  /* Software start conversion. */
#if defined(STM32F1)
  adc_start_conversion_direct(ADC1);
#else
  adc_start_conversion_regular(ADC1);
#endif

  /* Wait for ADC end of conversion. */
  while (!adc_eoc(ADC1))
  {
  }

  return adc_read_regular(ADC1); /* Read ADC value. */
}

static void adc_setup(void)
{
/* Set to input analog. */
#if defined(STM32F1)
  gpio_set_mode(GPIO_ADC_PORT,
                GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_ANALOG,
                GPIO_ADC_A0_PIN);
#else
  gpio_mode_setup(GPIO_ADC_PORT,
                  GPIO_MODE_ANALOG,
                  GPIO_PUPD_NONE,
                  GPIO_ADC_A0_PIN);
#endif

  /* Setup ADC. */
  adc_power_off(ADC1);

  adc_disable_scan_mode(ADC1);
  adc_disable_external_trigger_regular(ADC1);
  adc_set_single_conversion_mode(ADC1);
  adc_set_right_aligned(ADC1);
  adc_set_sample_time_on_all_channels(ADC1, ADC_SIMPLE_TIME);

  adc_power_on(ADC1);
  delay(800000); /* Wait a bit. */

#if defined(STM32F1)
  /* Self-calibration. */
  adc_reset_calibration(ADC1);
  adc_calibrate(ADC1);
#endif
}

static void rcc_setup(void)
{
#if defined(STM32F1)
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
#elif defined(STM32F4)
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
#endif

  rcc_periph_clock_enable(RCC_USART_TX_GPIO);
  rcc_periph_clock_enable(RCC_USART2);
  rcc_periph_clock_enable(RCC_ADC_GPIO);
  rcc_periph_clock_enable(RCC_ADC1);
}

static void usart_setup(void)
{
  /* Set USART-Tx pin to alternate function. */
#if defined(NUCLEO_F103RB)
  gpio_set_mode(GPIO_USART_TX_PORT,
                GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO_USART_TX_PIN);
#else
  gpio_mode_setup(GPIO_USART_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USART_TX_PIN);
  gpio_set_af(GPIO_USART_TX_PORT, GPIO_USART_AF, GPIO_USART_TX_PIN);
#endif

  /* Config USART params. */
  usart_set_baudrate(USART2, USART_BAUDRATE);
  usart_set_databits(USART2, 8);
  usart_set_stopbits(USART2, USART_STOPBITS_1);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART2, USART_MODE_TX);

  usart_enable(USART2);
}

static void delay(uint32_t value)
{
  for (uint32_t i = 0; i < value; i++)
  {
    __asm__("nop"); /* Do nothing. */
  }
}

/* For printf(). */
int _write(int file, char *ptr, int len)
{
  int i;

  if (file == 1)
  {
    for (i = 0; i < len; i++)
    {
      usart_send_blocking(USART2, ptr[i]);
    }
    return i;
  }

  errno = EIO;
  return -1;
}
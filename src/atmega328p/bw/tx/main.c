#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../../common/spi.c"
#include "../../common/nrf24l01p.c"
#include "../../common/uart.c"


/* timer1a compare on match handler */

static volatile uint8_t is_timer1_irq;

ISR(TIMER1_COMPA_vect)
{
  TCCR1B = 0;
  is_timer1_irq = 1;
}


int main(void)
{
  uint16_t counter;

  /* setup spi first */
  spi_setup_master();
  spi_set_sck_freq(SPI_SCK_FREQ_FOSC2);

  uart_setup();

  /* setup timer1, normal mode, interrupt on match 0x8000 */
  OCR1A = 0x8000;
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  TIMSK1 = 1 << 1;

  /* enable interrupts */
  sei();

  nrf24l01p_setup();

  /* sparkfun usb serial board configuration */
  /* NOTE: nrf24l01p_enable_crc8(); for nrf24l01p board */
  /* nrf24l01p_enable_crc16(); */
  nrf24l01p_disable_crc();
  /* auto ack disabled */
  /* auto retransmit disabled */
  /* 4 bytes payload */
  /* 1mbps, 0dbm */
  /* nrf24l01p_set_rate(NRF24L01P_RATE_1MBPS); */
  nrf24l01p_set_rate(NRF24L01P_RATE_2MBPS);
  /* nrf24l01p_set_rate(NRF24L01P_RATE_250KBPS); */
  /* channel 2 */
  nrf24l01p_set_chan(2);
  /* 5 bytes addr width */
  /* nrf24l01p_set_addr_width(NRF24L01P_ADDR_WIDTH_5); */
  nrf24l01p_set_addr_width(NRF24L01P_ADDR_WIDTH_3);
  /* rx address */
  nrf24l01p_cmd_buf[0] = 0xe7;
  nrf24l01p_cmd_buf[1] = 0xe7;
  nrf24l01p_cmd_buf[2] = 0xe7;
  nrf24l01p_cmd_buf[3] = 0xe7;
  nrf24l01p_cmd_buf[4] = 0xe7;
  nrf24l01p_write_reg40(NRF24L01P_REG_RX_ADDR_P0);
  /* tx address */
  nrf24l01p_cmd_buf[0] = 0xe7;
  nrf24l01p_cmd_buf[1] = 0xe7;
  nrf24l01p_cmd_buf[2] = 0xe7;
  nrf24l01p_cmd_buf[3] = 0xe7;
  nrf24l01p_cmd_buf[4] = 0xe7;
  nrf24l01p_write_reg40(NRF24L01P_REG_TX_ADDR);
  /* enable tx no ack command */
  nrf24l01p_enable_tx_noack();

  nrf24l01p_powerdown_to_standby();

  uart_write((uint8_t*)"tx side\r\n", 9);

 redo_transmit:

  counter = 0;
  is_timer1_irq = 0;

  uart_write((uint8_t*)"press space\r\n", 13);
  uart_read_uint8();
  uart_write((uint8_t*)"starting\r\n", 10);

  nrf24l01p_standby_to_tx();

  if (nrf24l01p_is_tx_empty() == 0) nrf24l01p_flush_tx();

  /* clock source disabled, safe to access 16 bits counter */
  TCNT1 = 0;

  /* prescaler set to 1024 */
  /* interrupt every 2.01s (match on 0x8000) */
  TCCR1B = 5 << 0;

  while (1)
  {
#define CONFIG_CHECK 0
#if CONFIG_CHECK
    uint8_t i;
    for (i = 0; i < NRF24L01P_PAYLOAD_WIDTH; ++i)
      nrf24l01p_cmd_buf[i] = 0x2a + i;
#endif

    nrf24l01p_write_tx_noack();
    while (nrf24l01p_is_tx_irq() == 0) ;
    ++counter;
    if (is_timer1_irq == 1) break ;
  }

  /* print counter */
  uart_write((uint8_t*)"counter: ", 9);
  uart_write((uint8_t*)uint16_to_string(counter), 4);
  uart_write((uint8_t*)"\r\n", 2);

  goto redo_transmit;

  return 0;
}

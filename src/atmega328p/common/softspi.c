#include <stdint.h>
#include <avr/io.h>


/* default pins */
#define SOFTSPI_CLK_DDR DDRD
#define SOFTSPI_CLK_PORT PORTD
#define SOFTSPI_CLK_MASK (1 << 3)
#define SOFTSPI_MOSI_DDR DDRD
#define SOFTSPI_MOSI_PORT PORTD
#define SOFTSPI_MOSI_MASK (1 << 4)

#ifndef SOFTSPI_DONT_USE_MISO
#define SOFTSPI_DONT_USE_MISO 0
#endif

#if (SOFTSPI_DONT_USE_MISO == 0)
#define SOFTSPI_MISO_DDR DDRD
#define SOFTSPI_MISO_PIN PIND
#define SOFTSPI_MISO_MASK (1 << 5)
#endif

static void softspi_setup_master(void)
{
  SOFTSPI_CLK_DDR |= SOFTSPI_CLK_MASK;
  SOFTSPI_MOSI_DDR |= SOFTSPI_MOSI_MASK;

#if (SOFTSPI_DONT_USE_MISO == 0)
  SOFTSPI_MISO_DDR |= SOFTSPI_MISO_MASK;
#endif
}

static inline void softspi_clk_low(void)
{
  SOFTSPI_CLK_PORT &= ~SOFTSPI_CLK_MASK;
}

static inline void softspi_clk_high(void)
{
  SOFTSPI_CLK_PORT |= SOFTSPI_CLK_MASK;
}

static inline void softspi_mosi_low(void)
{
  SOFTSPI_MOSI_PORT &= ~SOFTSPI_MOSI_MASK;
}

static inline void softspi_mosi_high(void)
{
  SOFTSPI_MOSI_PORT |= SOFTSPI_MOSI_MASK;
}

static inline void softspi_write_bit(uint8_t x, uint8_t m)
{
  /* dac7554 samples at clock falling edge */

  /* 5 insns per bit */

  softspi_clk_high();
  if (x & m) softspi_mosi_high(); else softspi_mosi_low();
  softspi_clk_low();
}

static void softspi_write_uint8(uint8_t x)
{
  /* transmit msb first, sample at clock falling edge */

  softspi_write_bit(x, (1 << 7));
  softspi_write_bit(x, (1 << 6));
  softspi_write_bit(x, (1 << 5));
  softspi_write_bit(x, (1 << 4));
  softspi_write_bit(x, (1 << 3));
  softspi_write_bit(x, (1 << 2));
  softspi_write_bit(x, (1 << 1));
  softspi_write_bit(x, (1 << 0));
}

static inline void softspi_write_uint16(uint16_t x)
{
  softspi_write_uint8((uint8_t)(x >> 8));
  softspi_write_uint8((uint8_t)(x & 0xff));
}

#if (SOFTSPI_DONT_USE_MISO == 0)

static inline void softspi_read_bit(uint8_t* x, uint8_t i)
{
  /* read at falling edge */

  softspi_clk_high();
#if 0
  /* no need, atmega328p clock below 50mhz */
  /* softspi_wait_clk(); */
#endif
  softspi_clk_low();

  if (SOFTSPI_MISO_PIN & SOFTSPI_MISO_MASK) *x |= 1 << i;
}

static uint8_t softspi_read_uint8(void)
{
  /* receive msb first, sample at clock falling edge */

  /* must be initialized to 0 */
  uint8_t x = 0;

  softspi_read_bit(&x, 7);
  softspi_read_bit(&x, 6);
  softspi_read_bit(&x, 5);
  softspi_read_bit(&x, 4);
  softspi_read_bit(&x, 3);
  softspi_read_bit(&x, 2);
  softspi_read_bit(&x, 1);
  softspi_read_bit(&x, 0);

  return x;
}

static inline uint16_t softspi_read_uint16(void)
{
  /* msB ordering */
  const uint8_t x = softspi_read_uint8();
  return ((uint16_t)x << 8) | (uint16_t)softspi_read_uint8();
}

#endif /* SOFTSPI_DONT_USE_MISO == 0 */

#include <stdint.h>
#include <avr/io.h>

static inline void set_baud_rate(long baud)
{
  uint16_t UBRR0_value = ((F_CPU / 16 + baud / 2) / baud - 1);
  UBRR0H = UBRR0_value >> 8;
  UBRR0L = UBRR0_value;
}

static void uart_setup(void)
{
  /* #define CONFIG_FOSC (F_CPU * 2) */
  /* const uint16_t x = CONFIG_FOSC / (16 * BAUDS) - 1; */
#if 0 /* (bauds == 9600) */
  const uint16_t x = 206;
#elif 0 /* (bauds == 115200) */
  const uint16_t x = 16;
#elif 0 /* (bauds == 500000) */
  const uint16_t x = 3;
#elif 0 /* (bauds == 1000000) */
  const uint16_t x = 1;
#endif

  set_baud_rate(9600);

  /* baud doubler off  - Only needed on Uno XXX */
  UCSR0A &= ~(1 << U2X0);

  UCSR0B = (1 << RXEN0) | (1 << TXEN0);

  /* default to 8n1 framing */
  UCSR0C = (3 << 1);
}

static void uart_write(uint8_t* s, uint8_t n)
{
  for (; n; --n, ++s)
  {
    /* wait for transmit buffer to be empty */
    while (!(UCSR0A & (1 << UDRE0))) ;
    UDR0 = *s;
  }

  /* wait for last byte to be sent */
  while ((UCSR0A & (1 << 6)) == 0) ;
}

static uint8_t uart_read_uint8(void)
{
  while ((UCSR0A & (1 << 7)) == 0) ;
  return UDR0;
}

static inline uint8_t nibble(uint32_t x, uint8_t i)
{
  return (x >> (i * 4)) & 0xf;
}

static inline uint8_t hex(uint8_t x)
{
  return (x >= 0xa) ? 'a' + x - 0xa : '0' + x;
}


static uint8_t hex_buf[8];

static uint8_t* uint8_to_string(uint8_t x)
{
  hex_buf[1] = hex(nibble(x, 0));
  hex_buf[0] = hex(nibble(x, 1));

  return hex_buf;
}

static uint8_t* uint16_to_string(uint16_t x)
{
  hex_buf[3] = hex(nibble(x, 0));
  hex_buf[2] = hex(nibble(x, 1));
  hex_buf[1] = hex(nibble(x, 2));
  hex_buf[0] = hex(nibble(x, 3));

  return hex_buf;
}

static uint8_t* uint32_to_string(uint32_t x)
{
  hex_buf[7] = hex(nibble(x, 0));
  hex_buf[6] = hex(nibble(x, 1));
  hex_buf[5] = hex(nibble(x, 2));
  hex_buf[4] = hex(nibble(x, 3));
  hex_buf[3] = hex(nibble(x, 4));
  hex_buf[2] = hex(nibble(x, 5));
  hex_buf[1] = hex(nibble(x, 6));
  hex_buf[0] = hex(nibble(x, 7));

  return hex_buf;
}

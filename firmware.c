#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// ADAS 1000 Register Addresses
const char NOP       = 0x00;   // No Operation Register
const char ECGCTL    = 0x01;   // ECG control - reset value 0x000000
const char FRMCTL    = 0x0A;   // Controls what shows up in FRAME
const char LOFFCTL   = 0x02;   // Lead-off control 
const char PACEAMPTH = 0x07;   // Show PACEAMPTH or Pace amplitude threshold // reset value 0x242424
const char LADATA    = 0x11;   // LA or Lead I data 
const char LLDATA    = 0x12;   // LL or Lead II data
const char RADATA    = 0x13;   // RA or Lead III data
const char V1DATA    = 0x14;   // V1 or V1’ data
const char V2DATA    = 0x15;   // V2 or V2’ data
const char PACEDATA  = 0x1A;   // Read pace detection data/status
const char FRAMES    = 0x40;   // Frame header - reset value 0x800000

// ADAS 1000 ECG Control Register Details
char ECG_EN    = 0x1F;   // Enables LA, LL, RA, V1, V2 Channels (1 = Enable) (5 Bit)
char CHCONFIG  = 0x00;   // 0 = Single Ended Input; 1 = Differential Input
char GAIN      = 0x00;   // Overall gain - 00 = x1.4; 01 = x2.1; 10 = x2.8; 11 = x4.2 (user cal req)
char VREFBUF   = 0x00;   // VREF buffer enable
char CLKEXT    = 0x00;   // External Clock is Used (0 = XTAL; 1 = Internal)
char MASTER    = 0x01;   // Determines Master or Slave in Gang Mode (0 = Slave; 1 = Master)
char GANG      = 0x00;   // Enables Gang Mode (0 = single; 1 = gang) 
char HP        = 0x01;   // High Power / Noise Setting (0 = Low Power; 1 = Low Noise)
char CNVEN     = 0x01;   // Enables ADC conversion and filters
char PWREN     = 0x01;   // Enables Power

// ADAS 1000 Frame Control Register Details (0 = included; 1 = excluded)
char ECG_CHN   = 0x00;   // LA, LL, RA, V1, V2 (5 Bit)
char PACEDIS   = 0x01;   // Pace Detection
char RESPMDIS  = 0x01;   // Respiration Magnitude
char RESPPHDIS = 0x01;   // Respiration Phase
char LOFFDIS   = 0x01;   // Lead Off Status
char GPIODIS   = 0x01;   // GPIO Word
char CRCDIS    = 0x00;   // CRC Word
char ADIS      = 0x00;   // Autodisable PACEDIS, RESPMDIS, LOFFDIS if flags are not set
char RDYRPT    = 0x00;   // Indicates DATA READY
char DATAFMT   = 0x00;   // 0 = Digital Lead/Vector Format; 1 = Electrode Format
char SKIP      = 0x00;   // 00 = output every frame; 01 = output every other frame; 1x = output every 4th frame
char FRMRATE   = 0x00;   // 00 = 2kHz; 01 = 16kHz, 10 = 128 kHz, 11 = 31.25 Hz

#define SPI_INSTANCE  0           /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

uint8_t       m_tx_buf[4];                        /**< TX buffer. */
uint8_t       m_rx_buf[4];                        /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf); /**< Transfer length. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
    NRF_LOG_INFO("Transfer completed.");
    if (m_rx_buf[0] != 0)
    {
        NRF_LOG_INFO(" Received:");
        NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
    }
}

/**
 * @brief A NOP read to delay a SPI cycle
 * @param spi - SPI instance
 */

void ADAS_1000_read_noop(nrf_drv_spi_t const * const spi) {
  nrf_delay_us(50);
  APP_ERROR_CHECK(nrf_drv_spi_transfer(spi, 0x00000000, m_length, m_rx_buf, m_length));
  printf("0x");
  for (int i = 0; i < 4; i++) {
    printf("%02x", (unsigned char) m_rx_buf[i]);
  }
  printf("\n");
  return;
}

/**
 * @brief Reads out a register value on ADAS1000 - FOR DEBUGGING
 * @param spi - SPI instance
 * @param reg - Register to read
 */
void ADAS_1000_read_reg(nrf_drv_spi_t const * const spi, char reg) {
  /* R/W:  [31]
     Reg:  [30:24]
     Data: [23:0] */
  int payload = (0b0 << 31) | (reg << 24) | 0x000000;
  printf("Reading Reg 0x%02x: ", reg);
  for (int i = 0; i < 4; i++) {
    m_tx_buf[i] = (payload >> (24 - (8 * i))) & 0xFF;
  }
  APP_ERROR_CHECK(nrf_drv_spi_transfer(spi, m_tx_buf, m_length, m_rx_buf, m_length));
  ADAS_1000_read_noop(spi);
  printf("0x");
  for (int i = 0; i < 4; i++) {
    printf("%02x", (unsigned char) m_rx_buf[i]);
  }
  printf("\n");
  return;
}

/**
 * @brief Reads out a register value on ADAS1000
 * @param spi   - SPI instance
 * @param reg   - Register to write
 * @param value - Value to write
 */
void ADAS_1000_write_reg(nrf_drv_spi_t const * const spi, char reg, int value) {
  /* R/W:  [31]
     Reg:  [30:24]
     Data: [23:0] */
  int payload = (0b1 << 31) | (reg << 24) | (value & 0x00FFFFFF);
//  printf("Writing: 0x%02x to Reg 0x%02x ", (unsigned int) value, (unsigned char) reg);
  for (int i = 0; i < 4; i++) {
    m_tx_buf[i] = (payload >> (24 - (8 * i))) & 0xFF;
//    printf("%02x", (unsigned char) m_tx_buf[i]);
  }
  printf("\n");
  APP_ERROR_CHECK(nrf_drv_spi_transfer(spi, m_tx_buf, m_length, m_rx_buf, m_length));
  return;
}


/**
 * @brief Configures the ECG Control Register
 * @param spi - SPI instance
 */
void ADAS_1000_ecgctl_setup(nrf_drv_spi_t const * const spi) {
  int ecg_ctl = 0x00000000 | ECG_EN << 19 | CHCONFIG << 10 | GAIN << 8 | VREFBUF << 7 | 
                CLKEXT << 6 | MASTER << 5 | GANG << 4 | HP << 3 | CNVEN << 2 | PWREN << 1;
  ADAS_1000_write_reg(spi, ECGCTL, ecg_ctl);
  return;
}

/**
 * @brief Configures the Frame Control Register
 * @param spi - SPI instance
 */
void ADAS_1000_frmctl_setup(nrf_drv_spi_t const * const spi) {
  int frm_ctl = 0x00078000 | ECG_CHN << 19 | PACEDIS << 14 | RESPMDIS << 13 | RESPPHDIS << 12 | LOFFDIS << 11 | 
                GPIODIS << 10 | CRCDIS << 9 | ADIS << 7 | RDYRPT << 6 | DATAFMT << 4 | SKIP << 2 | FRMRATE;
  ADAS_1000_write_reg(spi, FRMCTL, frm_ctl);
  return;
}

/**
 * @brief Reads out an entire Frame from the ADAS1000
 * @param spi - SPI instance
 */
void ADAS_1000_frame_readout(nrf_drv_spi_t const * const spi, uint8_t frm_len) {
  return;
}

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin    = SPI_SS_PIN;
    spi_config.miso_pin  = SPI_MISO_PIN;
    spi_config.mosi_pin  = SPI_MOSI_PIN;
    spi_config.sck_pin   = SPI_SCK_PIN;
    spi_config.mode      = NRF_DRV_SPI_MODE_3;
    spi_config.frequency = NRF_DRV_SPI_FREQ_8M

    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

    printf("\n---Setting up Frame Control---\n");
    ADAS_1000_frmctl_setup(&spi);
    ADAS_1000_read_reg(&spi, FRMCTL);

    printf("\n---Setting up ECG Control---\n");
    ADAS_1000_ecgctl_setup(&spi);
    ADAS_1000_read_reg(&spi, ECGCTL);

    printf("\n---Setting up Lead Off Control---\n");
    ADAS_1000_write_reg(&spi, LOFFCTL, 0x000000);
    ADAS_1000_read_reg(&spi, LOFFCTL);
    
    // Start Frame Readout
    ADAS_1000_read_reg(&spi, FRAMES);
    while (1)
    {
        // Reset rx buffer and transfer done flag
        memset(m_rx_buf, 0, m_length);
        spi_xfer_done = false;
        ADAS_1000_read_noop(&spi);
        while (!spi_xfer_done)
        {
            __WFE();
        }

        NRF_LOG_FLUSH();

        bsp_board_led_invert(BSP_BOARD_LED_0);
        nrf_delay_ms(500);
    }
}

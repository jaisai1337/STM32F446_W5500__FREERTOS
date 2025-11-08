#include "stm32f4xx.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "dhcp.h"
#include "dns.h"
#include "uart.h"
#include <stdio.h>
#include <stdbool.h>

/* ---------------- GPIO & SPI Definitions ---------------- */
#define SPI1_SCK_PIN    (5U)
#define SPI1_MISO_PIN   (6U)
#define SPI1_MOSI_PIN   (7U)
#define SPI1_CS_PIN     (4U)
#define SPI1_RESET_PIN  (0U)   // PB0

#define GPIOAEN         (1U << 0)
#define GPIOBEN         (1U << 1)
#define SPI1EN          (1U << 12)

#define SPI1_CS_LOW()     (GPIOA->BSRR = (1U << (SPI1_CS_PIN + 16U)))
#define SPI1_CS_HIGH()    (GPIOA->BSRR = (1U << SPI1_CS_PIN))
#define SPI1_RESET_LOW()  (GPIOB->BSRR = (1U << (SPI1_RESET_PIN + 16U)))
#define SPI1_RESET_HIGH() (GPIOB->BSRR = (1U << SPI1_RESET_PIN))

/* ---------------- Configurable Settings ---------------- */
#define USE_DHCP     1  // Set to 1 to use DHCP, 0 for static IP
#define LAB          0  // Set to 1 for lab network settings
#define DHCP_SOCKET  7
#define DNS_SOCKET   6
static uint8_t DHCP_buffer[548];
static uint8_t DNS_buffer[512];
volatile bool ip_assigned = false;

/* ---------------- Network Info ---------------- */
wiz_NetInfo netInfo = {
   // 50:9A:4C:1B:9B:7F, // MAC Address
#if LAB
    .mac = {0x50, 0x9A, 0x4C, 0x1B, 0x9B, 0x8F},
    //.mac = {0xA8, 0x4D, 0xA4, 0xA3, 0x7E, 0x57},
    .ip  = {192, 168, 131, 242},
    .sn  = {255, 255, 254, 0},
    .gw  = {192, 168, 130, 1},
    .dns = {192, 168, 36, 53},
#else
    .mac = {0xA8, 0x4D, 0xA4, 0xA3, 0x7E, 0x57},
    .ip  = {10, 8, 88, 149},
    .sn  = {255, 255, 254, 0},
    .gw  = {10, 8, 88, 1},
    .dns = {192, 168, 36, 53},
#endif
#if USE_DHCP
    .dhcp = NETINFO_DHCP
#else
    .dhcp = NETINFO_STATIC
#endif
};

/* ---------------- Helper Delay ---------------- */
static void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 4000; i++);
}

/* ---------------- SPI Init ---------------- */
void SPI1_Init(void)
{
    RCC->AHB1ENR |= GPIOAEN | GPIOBEN;
    RCC->APB2ENR |= SPI1EN;

    /* PA5,6,7 -> AF5 (SPI1) */
    GPIOA->MODER &= ~(0x3F << (SPI1_SCK_PIN * 2));
    GPIOA->MODER |=  (0x2A << (SPI1_SCK_PIN * 2));
    GPIOA->AFR[0]  &= ~((0xF << (SPI1_SCK_PIN * 4)) |
                        (0xF << (SPI1_MISO_PIN * 4)) |
                        (0xF << (SPI1_MOSI_PIN * 4)));
    GPIOA->AFR[0]  |=  (5 << (SPI1_SCK_PIN * 4)) |
                        (5 << (SPI1_MISO_PIN * 4)) |
                        (5 << (SPI1_MOSI_PIN * 4));

    /* CS -> PA4 output */
    GPIOA->MODER &= ~(3U << (SPI1_CS_PIN * 2));
    GPIOA->MODER |=  (1U << (SPI1_CS_PIN * 2));
    SPI1_CS_HIGH();

    /* RESET -> PB0 output */
    GPIOB->MODER &= ~(3U << (SPI1_RESET_PIN * 2));
    GPIOB->MODER |=  (1U << (SPI1_RESET_PIN * 2));
    SPI1_RESET_HIGH();

    /* SPI1 setup */
    SPI1->CR1 = 0;
    SPI1->CR1 |= (1U << 2);    // Master
    SPI1->CR1 |= (0x1 << 3);   // fPCLK/4
    SPI1->CR1 |= (1U << 9) | (1U << 8); // SSM, SSI
    SPI1->CR1 |= (1U << 6);    // Enable
}

/* ---------------- SPI Transfer ---------------- */
uint8_t W5500_ReadByte(void)
{
    uint8_t tx = 0xFF;
    uint8_t rx;
    while (!(SPI1->SR & (1U << 1)));
    *(volatile uint8_t *)&SPI1->DR = tx;
    while (!(SPI1->SR & (1U << 0)));
    rx = *(volatile uint8_t *)&SPI1->DR;
    return rx;
}

void W5500_WriteByte(uint8_t byte)
{
    while (!(SPI1->SR & (1U << 1)));
    *(volatile uint8_t *)&SPI1->DR = byte;
    while (!(SPI1->SR & (1U << 0)));
    (void)*(volatile uint8_t *)&SPI1->DR;
}

void W5500_ReadBurst(uint8_t* pBuf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        pBuf[i] = W5500_ReadByte();
}

void W5500_WriteBurst(uint8_t* pBuf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        W5500_WriteByte(pBuf[i]);
}

/* ---------------- Chip Select ---------------- */
void W5500_Select(void)   { SPI1_CS_LOW(); }
void W5500_Unselect(void) { SPI1_CS_HIGH(); }

/* ---------------- DHCP Callbacks ---------------- */
void Callback_IPAssigned(void) { ip_assigned = true; }
void Callback_IPConflict(void) { ip_assigned = false; }

/* ---------------- W5500 Init ---------------- */
int W5500_Init(void)
{
    uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    if (ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) != 0) {
        //printf("WIZCHIP memory init failed!\r\n");
        uart2_write_string("WIZCHIP memory init failed!\r\n");
    }
    //printf("Resetting W5500...\r\n");
    uart2_write_string("Resetting W5500...\r\n");
    SPI1_RESET_LOW();
    delay_ms(100);
    SPI1_RESET_HIGH();
    delay_ms(100);

    reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
    reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
    reg_wizchip_spiburst_cbfunc(W5500_ReadBurst, W5500_WriteBurst);

    if (ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1) {
        //printf("WIZCHIP init failed!\r\n");
        uart2_write_string("WIZCHIP init failed!\r\n");
        return -1;
    }

    uint8_t ver = getVERSIONR();
    if (ver != 0x04) {
        //printf("W5500 not detected! Version: 0x%02X\r\n", ver);
        uart2_write_string("W5500 not detected! Version: 0x");
        uart2_write_hex(ver);
        uart2_write_string("\r\n");
        return -2;
    }
    //printf("✅ W5500 detected (ver 0x%02X)\r\n", ver);
    uart2_write_string("W5500 detected ver 0x");
    uart2_write_hex(ver);
    uart2_write_string("\r\n");
    /* PHY link check */
    //printf("Checking PHY link...\r\n");
    uart2_write_string("Checking PHY link...\r\n");
    uint8_t link = PHY_LINK_OFF;
    for (uint8_t retries = 20; retries > 0; retries--) {
        ctlwizchip(CW_GET_PHYLINK, &link);
        if (link == PHY_LINK_ON) {
            //printf("✅ PHY Link UP\r\n");
            uart2_write_string("PHY Link UP\r\n");
            break;
        }
        delay_ms(500);
    }
    if (link != PHY_LINK_ON) {
        //printf("❌ No PHY link detected.\r\n");
        uart2_write_string("No PHY link detected.\r\n");
        return -3;
    }

#if USE_DHCP
    //printf("Starting DHCP...\r\n");
    uart2_write_string("Starting DHCP...\r\n");
    setSHAR(netInfo.mac);
    DHCP_init(DHCP_SOCKET, DHCP_buffer);
    reg_dhcp_cbfunc(Callback_IPAssigned, Callback_IPAssigned, Callback_IPConflict);

    uint16_t dhcp_retries = 5;
    while (!ip_assigned && dhcp_retries--) {
        DHCP_run();
        // uart2_write_int(dhcp_retries);
        // uart2_write_string("\n");
        delay_ms(500);
    }

    if (!ip_assigned) {
        //printf("❌ DHCP failed. Using static IP.\r\n");
        uart2_write_string("DHCP failed. Using static IP.\r\n");
        ctlnetwork(CN_SET_NETINFO, (void*)&netInfo);
    } else {
        getIPfromDHCP(netInfo.ip);
        getGWfromDHCP(netInfo.gw);
        getSNfromDHCP(netInfo.sn);
        getDNSfromDHCP(netInfo.dns);
        ctlnetwork(CN_SET_NETINFO, (void*)&netInfo);
        //printf("DHCP Success!\r\n");
        uart2_write_string("DHCP Success!\r\n");
    }
#else
    //printf("Using static IP config.\r\n");
    uart2_write_string("Using static IP config.\r\n");
    ctlnetwork(CN_SET_NETINFO, (void*)&netInfo);
#endif

    /* DNS Setup */
    DNS_init(DNS_SOCKET, DNS_buffer);

    wiz_NetInfo info;
    ctlnetwork(CN_GET_NETINFO, &info);
    // printf("\r\n==== Network Configuration ====\r\n");
    // printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
    //        info.mac[0], info.mac[1], info.mac[2],
    //        info.mac[3], info.mac[4], info.mac[5]);
    // printf("IP  : %d.%d.%d.%d\r\n", info.ip[0], info.ip[1], info.ip[2], info.ip[3]);
    // printf("GW  : %d.%d.%d.%d\r\n", info.gw[0], info.gw[1], info.gw[2], info.gw[3]);
    // printf("SN  : %d.%d.%d.%d\r\n", info.sn[0], info.sn[1], info.sn[2], info.sn[3]);
    // printf("DNS : %d.%d.%d.%d\r\n", info.dns[0], info.dns[1], info.dns[2], info.dns[3]);
    // printf("===============================\r\n");
    uart2_write_string("MAC : ");
    for (int i = 0; i < 6; i++) {
        uart2_write_hex(info.mac[i]);
        if (i < 5) uart2_write_char(':');
    }
    uart2_write_string("\r\n");
    uart2_write_ip(info.ip, "IP");
    uart2_write_ip(info.gw, "GW");
    uart2_write_ip(info.sn, "SN");
    uart2_write_ip(info.dns, "DNS");
    return 0;
}

/*****************************************************
Raspberri PI 3 GPIO definitions for FiC Makr2 configuration 
*****************************************************/


#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define BLOCK_SIZE      4 * 1024
#define PERIPHERAL_BASE 0x3F000000
#define GPIO_BASE       PERIPHERAL_BASE + 0x00200000

typedef struct {
  unsigned long         gpio_base;
  int                   memory_fd;
  void                  *map;
  volatile unsigned int *addr;
} rpi_gpio;

int MapGPIO(rpi_gpio *gpio);
int MapGPIO_r(rpi_gpio *gpio);
void UnmapGPIO(rpi_gpio *gpio);
void BlinkLED(rpi_gpio *gpio);
int write_config(rpi_gpio *gpio, FILE *fp1, FILE *fp2, FILE *fp3);
void init_test(rpi_gpio *gpio);
void write_test(rpi_gpio *gpio, int data);
void init_read(rpi_gpio *gpio );
int read_test(rpi_gpio *gpio );
//
void reset_fic(rpi_gpio gpio);
void comwrite(rpi_gpio gpio, int j);
void addwrite(rpi_gpio gpio, int add);
void write_hw(rpi_gpio gpio, int d);
void write_cont(rpi_gpio gpio, int d);
int read_hw(rpi_gpio gpio );
int read_cont(rpi_gpio gpio );
void read_fin(rpi_gpio gpio );
void write_fin(rpi_gpio gpio );


/* general RasPI GPIO definitions */
#define GPFSEL0 0
#define GPFSEL1 1
#define GPFSEL2 2
#define GPFSEL3 3
#define GPFSEL4 4
#define GPFSEL5 5
#define GPSET0 7
#define GPSET1 8
#define GPCLR0 10
#define GPCLR1 11
#define GPLEV0 13
#define GPLEV1 14

#define     CLRALL              0xffffffff             // Cler All
#define     INIFBUS             0x000000b0             // Init Port[29:4] at Config
#define     FAK              0x08000000                // fak: GPIO_27 Input
#define     FRQ              0x04000000                // frq: GPIO_26 Input
#define     FREQ                0x00000040             // req  : GPIO_06 output
#define     FSTB                0x00000010             // stb  : GPIO_04 output

#define FICMASK 0x02ffff50 // CD0-CD15 on GPSET0/GPCLR0

//   bit       Conf_IO_IN : Comment
//   //   31-28        x  x  0 : 
//   //      27    FAK  I  0 : 
//   //      26    FRQ  I  0 : 
//   //      25     CFSL  O  0 : 
//   //      24        x  I  x : 
//   //      23   DB[15]  B  0 : 
//   //      22   DB[14]  B  0 : 
//   //      21   DB[13]  B  0 : 
//   //      20   DB[12]  B  0 : 
//   //      19   DB[11]  B  0 : 
//   //      18   DB[10]  B  0 : 
//   //      17   DB[ 9]  B  0 : 
//   //      16   DB[ 8]  B  0 : 
//   //      15   DB[ 7]  B  0 : 
//   //      14   DB[ 6]  B  0 : 
//   //      13   DB[ 5]  B  0 : 
//   //      12   DB[ 4]  B  0 : 
//   //      11   DB[ 3]  B  0 : 
//   //      10   DB[ 2]  B  0 : 
//   //       9   DB[ 1]  B  0 : 
//   //       8   DB[ 0]  B  0 : 
//   //       7        x  x  0 : 
//   //       6    	 FREQ  O  1 : 
//   //       5      x     I  0 : 
//   //       4      FSTB  O  1 : 
//   //       3   I2C_SC  B  x : 
//   //       2   I2C_SD  B  x : 
//   //    1- 0        x  x  x : 
//
// Data Output
//  GPFSEL0:0x7e20_0000
//  //             09  08  07  06  05  04  03  02  01  00
//  //   = 32'b00_001_001_000_001_000_001_000_000_000_000
//  //   = 32'b0000_1001_0000_0100_0001_0000_0000_0000
//  //   = 0x0904_1000
//  
//   GPFSEL1:0x7e20_0004
//             19  18  17  16  15  14  13  12  11  10
//   = 32'b00_001_001_001_001_001_001_001_001_001_001
//   = 32'b0000_1001_0010_0100_1001_0010_0100_1001
//   = 0x0924_9249
//
//  GPFSEL2:0x7e20_0008
//  //             29  28  27  26  25  24  23  22  21  20
//  //   = 32'b00_000_000_000_000_001_000_001_001_001_001
//  //   = 32'b0000_0000_0000_0000_1000_0010_0100_1001
//  //   = 0x0000_8249
//

// Data Input
//  GPFSEL0:0x7e20_0000
//  //             09  08  07  06  05  04  03  02  01  00
//  //   = 32'b00_000_000_000_001_000_001_000_000_000_000
//  //   = 32'b0000_0000_0000_0100_0001_0000_0000_0000
//  //   = 0x0004_1000
//  
//   GPFSEL1:0x7e20_0004
//             19  18  17  16  15  14  13  12  11  10
//   = 32'b00_000_000_000_000_000_000_000_000_000_000
//   = 32'b0000_0000_0000_0000_0000_0000_0000_0000
//   = 0x0000_0000
//
//  GPFSEL2:0x7e20_0008
//  //             29  28  27  26  25  24  23  22  21  20
//  //   = 32'b00_000_000_000_000_001_000_000_000_000_000
//  //   = 32'b0000_0000_0000_0000_1000_0000_0000_0000
//  //   = 0x0000_8000
//

/* bits for FiC configuration */
#define GPFSEL0_WR 0x09041000
#define GPFSEL1_WR 0x09249249
#define GPFSEL2_WR 0x00008249
#define GPFSEL0_RD 0x00041000
#define GPFSEL1_RD 0x00000000
#define GPFSEL2_RD 0x00008000






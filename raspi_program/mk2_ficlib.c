#include "mk2_fic_rpi.h"
// #define DEBUG

/* Fic library */

void reset_fic(rpi_gpio gpio) {
	int data;
	data = FSTB; write_test(&gpio, data);  // req=0 stb=1 Reset pulse
	sleep(1);
	data = 0; write_test(&gpio, data);  // req=0 stb=1 Reset pulse
}

void comwrite(rpi_gpio gpio, int com) {
	int d,data;
	data = FREQ|FSTB|(com<<8); write_test(&gpio,data);  // req=1 stb=1 Write Start=1
	do {
		d= read_test(&gpio);
#ifdef DEBUG
		printf("CMD %d write %x %x\n", com, d,data);
#endif
	}while((FAK & d) == 0) ; // wait while fak=0
	if(com==1| com==4 ) data = 0;  //req=0, stb=0
		else data = FREQ;
	write_test(&gpio,data);
	do {
		d= read_test(&gpio);
#ifdef DEBUG
		printf("CMD %d write end %x\n",com, d);
#endif
	}while((FAK & read_test(&gpio)) == FAK) ; // wait while fak=1
}

void addwrite(rpi_gpio gpio, int add) {
		int data,d;
		data = FREQ | FSTB | (add<<8) ; write_test(&gpio,data);  // stb=1  Add 15:0
		do {
			d= read_test(&gpio);
		#ifdef DEBUG
			printf("Add 15:0 %x \n", d);
		#endif 
	    }while( (FAK & d ) == 0) ; // wait while fak=0

		data = FREQ; write_test(&gpio,data);  // stb=0
		do {
			d= read_test(&gpio);
		#ifdef DEBUG
			printf("Add 31:16 end %x \n", d);
		#endif 
		}while((FAK & d) == FAK) ; // wait while fak=1

}

void write_hw (rpi_gpio gpio, int d) {
		int data, rd;
		data = FREQ | FSTB | ((0xffff & d)<<8) ; write_test(&gpio,data);  // stb=1  data
		do {
			rd=read_test(&gpio);
		#ifdef DEBUG
			printf("Data Half word%x\n", rd);
		#endif 
	    }while( (FAK & rd ) == 0) ; // wait while fak=0
		data = 0; write_test(&gpio,data);  // req=0 stb=0 
		do {
			rd=read_test(&gpio);
		#ifdef DEBUG
			printf("Data Upper write end %x\n", rd);
		#endif 
		}while((FAK & read_test(&gpio)) == FAK) ; // wait while fak=1

}

void write_cont(rpi_gpio gpio, int d){
		int data,rd,rd1,rd2;
		int i;
		data = FREQ | FSTB| (0xffff & d)<<8 ; write_test(&gpio,data);  // stb=1  data
		rd=read_test(&gpio); rd1=0; rd2=1;
		do {
			rd2=rd1; rd1=rd; rd=read_test(&gpio);
		#ifdef DEBUG
			printf("Data %x \n", rd);
		#endif 
	    }while( ((FAK & rd ) == 0) || (rd1!=rd) || (rd2!=rd1) ) ; // wait while fak=0
		data = FREQ; write_test(&gpio,data);  // stb=0
		rd=read_test(&gpio); rd1=0; rd2=1;
		do {
			rd2=rd1; rd1=rd; rd=read_test(&gpio);
		#ifdef DEBUG
		   printf("Data end %x\n", rd);
		#endif 
		}while(( (FAK & rd) == FAK) || (rd1!=rd) ||(rd2!=rd1) ) ; // wait while fak=1
}

int read_hw(rpi_gpio gpio) {
		int data,d,rd,d1,d2;
		init_read(&gpio );
		data = FREQ|FSTB ; write_test(&gpio,data);  // stb=1  reading data
		d=read_test(&gpio); d1=0; d2=1;
		do {
			d2=d1; d1=d; d=read_test(&gpio); 
		#ifdef DEBUG
			printf("Read Data %x \n", d);
		#endif 
	    }while( ( ( FAK & d) == 0) || (d1!=d)||(d2!=d1)) ; // wait while fak=0 
		#ifdef DEBUG
		printf("Read result %x\n", d );
		#endif 
		rd = 0xffff & (d>>8);
		data = 0; write_test(&gpio,data);  // stb=0
		d2=d1; d1=d; d=read_test(&gpio); 
		do {
			d2=d1; d1=d; d=read_test(&gpio);
		#ifdef DEBUG
			printf("Read data Upper end %x\n", d);
		#endif 
		}while( ((FAK & d) == FAK)|| (d1!=d)||(d2!=d1) ) ; // wait while fak=1

		init_test(&gpio);
		return rd;
}

int read_cont(rpi_gpio gpio) {
		int data,d,d1,rd, d2;
		data = FREQ | FSTB ; write_test(&gpio,data);  // stb=1  reading data
		d=read_test(&gpio); d1=0; d2=1;
		do {
			d2=d1; d1=d; d=read_test(&gpio);
		#ifdef DEBUG
			printf("Read Data %x \n", d);
		#endif 
	   	}while( ( (FAK & d) == 0) || (d1!=d)||(d2!=d1)) ; // wait while fak=0 
		rd = d;
		data = FREQ; write_test(&gpio,data);  // stb=0
		d2=d1; d1=d; d=read_test(&gpio);
		do {
			d2=d1; d1=d; d=read_test(&gpio);
		#ifdef DEBUG
			printf("Read data end %x\n", d);
		#endif 
		}while(( (FAK & d) == FAK)||(d1!=d) ||(d2!=d1)) ; // wait while fak=1
		return (0xffff&(rd>>8));
}
void write_fin(rpi_gpio gpio){
    int data,d;
    data = 0x000; write_test(&gpio,data);  // req=0, stb=0
    do {
	        d=read_test(&gpio);
		#ifdef DEBUG
	        printf("Write Request end %x\n",d);
		#endif 
    }while((FRQ & d) == FRQ) ; // wait while req=1
}

void read_fin(rpi_gpio gpio) {
	int data, d;
	data = 0x000 ; write_test(&gpio,data);  // stb=1  reading data
	do {
		d= read_test(&gpio);
		#ifdef DEBUG
			printf("Read request end %x \n", d);
		#endif 
	}while((FRQ & d) == FRQ) ; // wait while req=1
	init_test(&gpio);
}

/* Fic Basic library */

int MapGPIO(rpi_gpio *gpio) {
// Edited for Nyacom tool
	gpio->memory_fd = open("/dev/gpiomem", O_RDWR|O_SYNC);

	if(gpio->memory_fd < 0) {
		perror("Failed to open /dev/gpiomem, try change permission.");
		return 1;
	}

	gpio->map = mmap(
		NULL,
		BLOCK_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		gpio->memory_fd,
		gpio->gpio_base
	);

	if(gpio->map == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	gpio->addr = (volatile unsigned int *)gpio->map;
	return 0;
}

void UnmapGPIO(rpi_gpio *gpio) {
	munmap(gpio->map, BLOCK_SIZE);
	close(gpio->memory_fd);
}

void init_test(rpi_gpio *gpio ) {
	*(gpio->addr +GPFSEL0)  = 0;
	*(gpio->addr +GPFSEL1)  = 0;  
	*(gpio->addr +GPFSEL2)  = 0;

	*(gpio->addr +GPFSEL0)  = GPFSEL0_WR;
	*(gpio->addr +GPFSEL1)  = GPFSEL1_WR;  
	*(gpio->addr +GPFSEL2)  = GPFSEL2_WR;
}
int read_test(rpi_gpio *gpio ) {
		int idata;
		idata = *(gpio->addr + GPLEV0); 
		return(idata);
		// fprintf(stderr, "SW: %x Count:%x \n", (idata>>22)&3, (idata>>20)&3); 
}
void init_read(rpi_gpio *gpio ) {
    *(gpio->addr +GPFSEL0)  = GPFSEL0_RD;
    *(gpio->addr +GPFSEL1)  = GPFSEL1_RD;
    *(gpio->addr +GPFSEL2)  = GPFSEL2_RD;
}


void write_test(rpi_gpio *gpio, int data ) {
	*(gpio->addr + GPCLR0) = (~data & FICMASK);
	*(gpio->addr + GPSET0)  = data & FICMASK;													// set bits
}

/* linux/arch/arm/mach-s3c2440/mach-jz2440.c
 *
 * Copyright (c) 2004,2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * http://www.fluff.org/ben/jz2440/
 *
 * Thanks to Dimity Andric and TomTom for the loan of an JZ2440.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dm9000.h>

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mmc/host.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>

#include <mach/idle.h>
#include <mach/fb.h>
#include <plat/iic.h>

#include <plat/s3c2410.h>
#include <plat/s3c2440.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>

#include <plat/nand.h>
#include <plat/common-smdk.h>
#include <plat/mci.h>


static struct map_desc jz2440_iodesc[] __initdata = {
	/* ISA IO Space map (memory space selected by A24) */

	{
		.virtual	= (u32)S3C24XX_VA_ISA_WORD,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_WORD + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}
};

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg jz2440_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	/* IR port */
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
	}
};

/* LCD driver info */

//AT043TN24
#define LCD_WIDTH        480
#define LCD_HEIGHT       272
#define LCD_PIXCLOCK     100000

#define LCD_RIGHT_MARGIN 1   //HFPD
#define LCD_LEFT_MARGIN  1   //HBPD
#define LCD_HSYNC_LEN    40  //HSPW

#define LCD_UPPER_MARGIN 1   //VBPD
#define LCD_LOWER_MARGIN 1   //VFPD
#define LCD_VSYNC_LEN    9   //VSPW

static struct s3c2410fb_display jz2440_lcd_cfg __initdata = {
	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,

	.type		= S3C2410_LCDCON1_TFT,
	.width		= LCD_WIDTH,
	.height 	= LCD_HEIGHT,
	.pixclock	= LCD_PIXCLOCK,
	.xres		= LCD_WIDTH,
	.yres		= LCD_HEIGHT,
	.bpp		= 16,
	.left_margin	= LCD_LEFT_MARGIN + 1,
	.right_margin	= LCD_RIGHT_MARGIN + 1,
	.hsync_len	= LCD_HSYNC_LEN + 1,
	.upper_margin	= LCD_UPPER_MARGIN + 1,
	.lower_margin	= LCD_LOWER_MARGIN + 1,
	.vsync_len	= LCD_VSYNC_LEN + 1,
};

static struct s3c2410fb_mach_info jz2440_fb_info __initdata = {
	.displays       = &jz2440_lcd_cfg,
	.num_displays   = 1,
	.default_display = 0,

	.gpccon         = 0xaaaaaaaa,
	.gpccon_mask	= 0xffffffff,
	.gpcup          = 0xffffffff,
	.gpcup_mask	= 0xffffffff,

	.gpdcon         = 0xaaaaaaaa,
	.gpdcon_mask	= 0xffffffff,
	.gpdup          = 0xffffffff,
	.gpdup_mask	= 0xffffffff,
};


/* DM9000AEP 10/100 ethernet controller */
#define MACH_JZ2440_DM9K_BASE S3C2410_CS4
static struct resource jz2440_dm9k_resource[] = {
	[0] = {
		.start = MACH_JZ2440_DM9K_BASE,
		.end   = MACH_JZ2440_DM9K_BASE + 3,
		.flags = IORESOURCE_MEM
	},
	[1] = {
		.start = MACH_JZ2440_DM9K_BASE + 4,
		.end   = MACH_JZ2440_DM9K_BASE + 7,
		.flags = IORESOURCE_MEM
	},
	[2] = {
		.start = IRQ_EINT7,
		.end   = IRQ_EINT7,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	}
};

static struct dm9000_plat_data jz2440_dm9k_pdata = {
	.flags		= (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM),
	.dev_addr = {0x08, 0x90, 0x00, 0xA0, 0x90, 0x90},
};

static struct platform_device jz2440_device_eth = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(jz2440_dm9k_resource),
	.resource	= jz2440_dm9k_resource,
	.dev		= {
		.platform_data	= &jz2440_dm9k_pdata,
	},
};

static struct mtd_partition jz2440_nand_part[] = {
	[0] = {
		.name	= "u-boot",
		.offset	= 0x00000000,
		.size	= 0x00080000, /* 512KB */
	},
	[1] = {
		.name	= "param",
		.offset = 0x00080000,
		.size	= 0x00080000, /* 512KB */
	},
	[2] = {
		.name	= "kernel",
		.offset = 0x00100000,
		.size	= 0x00500000, /* 5MB */
	},
	[3] = {
		.name	= "file-system",
		.offset = 0x00600000,
		.size	= 1024 * 1024 * 200, /* 200MB */
	},
	[4] = {
		.name	= "nand",
		.offset = 0x00000000,
		.size	= 1024 * 1024 * 256, /* 256MB */
	},
};

static struct s3c2410_nand_set jz2440_nand_sets[] = {
	[0] = {
		.name		= "NAND",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(jz2440_nand_part),
		.partitions	= jz2440_nand_part,
	},
};

static struct s3c2410_platform_nand jz2440_nand_info = {
	.tacls		= 20,
	.twrph0		= 60,
	.twrph1		= 20,
	.nr_sets	= ARRAY_SIZE(jz2440_nand_sets),
	.sets		= jz2440_nand_sets,
	.ignore_unset_ecc = 1,
};

static struct s3c24xx_mci_pdata jz2440_mmc_cfg = {
   .gpio_detect   = S3C2410_GPG8,
   .gpio_wprotect = S3C2410_GPH8,
   .ocr_avail     = MMC_VDD_32_33|MMC_VDD_33_34,
};

static struct platform_device *jz2440_devices[] __initdata = {
	&s3c_device_usb,
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
	&jz2440_device_eth,
	&s3c_device_rtc,
	&s3c_device_nand,
	&s3c_device_sdi,
};

static void __init jz2440_map_io(void)
{
	s3c24xx_init_io(jz2440_iodesc, ARRAY_SIZE(jz2440_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(jz2440_uartcfgs, ARRAY_SIZE(jz2440_uartcfgs));
	s3c_device_nand.dev.platform_data = &jz2440_nand_info;
	s3c_device_sdi.dev.platform_data = &jz2440_mmc_cfg;
}

static void __init jz2440_machine_init(void)
{
	s3c24xx_fb_set_platdata(&jz2440_fb_info);
	s3c_i2c0_set_platdata(NULL);

	platform_add_devices(jz2440_devices, ARRAY_SIZE(jz2440_devices));
}

MACHINE_START(JZ2440, "JZ2440")
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,

	.init_irq	= s3c24xx_init_irq,
	.map_io		= jz2440_map_io,
	.init_machine	= jz2440_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END

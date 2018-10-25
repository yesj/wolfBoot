/* nrf52.c
 *
 * Copyright (C) 2018 wolfSSL Inc.
 *
 * This file is part of wolfBoot.
 *
 * wolfBoot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfBoot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#include <stdint.h>

/* Assembly helpers */
#define DMB() __asm__ volatile ("dmb")


/* Instantiation */
#define CLOCK_CONTROL_BASE (0x40000000)
#define NVMC_BASE (0x4001E000)


/* Flash write/erase control */
#define NVMC_CONFIG *((volatile uint32_t *)(NVMC_BASE + 0x504))
#define NVMC_ERASEPAGE *((volatile uint32_t *)(NVMC_BASE + 0x508))
#define NVMC_READY *((volatile uint32_t *)(NVMC_BASE + 0x400))
#define NVMC_CONFIG_REN 0
#define NVMC_CONFIG_WEN 1
#define NVMC_CONFIG_EEN 2

#define FLASH_PAGE_SIZE (4096)

/* Clock control */
#define TASKS_HFCLKSTART   *((volatile uint32_t *)(CLOCK_CONTROL_BASE + 0x000))
#define TASKS_HFCLKSTOP    *((volatile uint32_t *)(CLOCK_CONTROL_BASE + 0x004))
#define TASKS_HFCLKSTARTED *((volatile uint32_t *)(CLOCK_CONTROL_BASE + 0x100))

static void flash_wait_complete(void)
{
    while (NVMC_READY == 0)
        ;
}

int hal_flash_write(uint32_t address, const uint8_t *data, int len)
{
    int i;
    int words = (len + 3) / 4;
    uint32_t *src = (uint32_t *)data;
    uint32_t *dst = (uint32_t *)address;
    for (i = 0; i < words; i ++) {
        NVMC_CONFIG = NVMC_CONFIG_WEN;
        flash_wait_complete();
        dst[i] = src[i];
        flash_wait_complete();
    }
    return 0;
}

void hal_flash_unlock(void)
{
}

void hal_flash_lock(void)
{
}


int hal_flash_erase(uint32_t address, int len)
{
    uint32_t end = address + len;
    uint32_t p;
    for (p = address; p <= end; p += FLASH_PAGE_SIZE) {
        NVMC_CONFIG = NVMC_CONFIG_EEN;
        flash_wait_complete();
        NVMC_ERASEPAGE = p;
        flash_wait_complete();
    }
    return 0;
}

void hal_init(void)
{
    TASKS_HFCLKSTART = 1;
    while(TASKS_HFCLKSTARTED == 0)
        ;
}

void hal_prepare_boot(void)
{
    TASKS_HFCLKSTOP = 1;
}


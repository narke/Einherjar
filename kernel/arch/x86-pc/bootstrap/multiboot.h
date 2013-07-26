#ifndef _MULTIBOOT_H_
#define _MULTIBOOT_H_

/**
 * @file multiboot.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 *
 * Multiboot info
 */

/** The Multiboot information */
typedef struct multiboot_info
{
  unsigned long flags;
  unsigned long mem_lower;
  unsigned long mem_upper;
  unsigned long boot_device;
  unsigned long cmdline;
  unsigned long mods_count;
  unsigned long mods_addr;
} multiboot_info_t;

#endif // _MULTIBOOT_H_

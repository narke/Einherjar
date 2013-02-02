/**
 * @file startup.c
 * @author Konstantin Tcholokachvili
 * @date 2013
 * Roentgenium's kernel's main file
 */

#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

#include <hardware/input_output/screen/vga.h>


/**
 * The kernel entry point. All starts from here!
 */

void roentgenium_main(void)
{
   extern unsigned int magic;

   //extern void *mbd;

   if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
   {
      /* Error */
      ;
   }
   
   // VGA scren setup
   vga_clear();
   vga_set_attributes(FG_BRIGHT_BLUE | BG_BLACK );
   vga_set_position(34, 0);
   
   vga_display_string("Roentgenium\n");

}

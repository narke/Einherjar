/**
 * @file startup.c
 * @author Konstantin Tcholokachvili
 * @date 2013
 * Roentgenium's kernel's main file
 */

#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

/**
 * The kernel entry point. All starts from here!
 * @param magic
 * @param address of grub bootloader's header
 * @return Nothing
 */

void roentgenium_main(void)
{
   extern unsigned int magic;

   extern void *mbd;

   if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
   {
      /* Error */
      ;
   }

   /* Print a letter to screen to see everything is working: */
   volatile unsigned char *videoram = (unsigned char *)0xB8000;
   videoram[0] = 65; /* character 'A' */
   videoram[1] = 0x07; /* light grey (7) on black (0). */
}

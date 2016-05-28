OUTPUT_FORMAT(
    "elf32-i386",  /* Default       */
    "elf32-i386",  /* Big endian    */
    "elf32-i386"   /* Little endian */
)
OUTPUT_ARCH(i386)
ENTRY(loader)

SECTIONS
{
    /**
     * 可执行程序的地址空间从 1MB (2^20 B) 往上开始
     */
    . = 0x9000;

    PROVIDE(start = .);

    .text : AT(0x9000) {  /* AT 指定物理地址 */
        *(.text .text.*)
    }

    PROVIDE(etext = .);  /* 定义符号 etext, 其值为代码段之后的地址 */

    .rodata : {
        *(.rodata .rodata.*)
    }

    . = ALIGN(0x1000);  /* 将数据段按页对齐 */

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    PROVIDE(end = .);  /* 定义符号 end, 其值为程序/数据段之后的地址 */
}

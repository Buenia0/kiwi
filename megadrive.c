#include <stdio.h>

unsigned char ROM[0x400000];
unsigned char RAM[0x10000];

const int CLOCK_NTSC = 53693175;
const int CPL_M68K = (int)((double)CLOCK_NTSC / 7.0 / 60.0 / 262.0);

void set_rom(unsigned char *buffer, size_t size)
{
    memcpy(ROM, buffer, size);
}

unsigned int read_memory(unsigned int address)
{
    unsigned int range = (address & 0xff0000) >> 16;

    if (range >= 0 && range <= 0x3f)
    {
        // ROM
        return ROM[address];
    }
    else if (range == 0xa0)
    {
        // Z80 space
        return 0;
    }
    else if (range == 0xa1)
    {
        // I/O and registers
        if (address >= 0xa10000 && address < 0xa10020)
        {
            return io_read_memory(address & 0x1f);
        }
        return 0;
    }
    else if (range >= 0xc0 && range <= 0xdf)
    {
        // VDP
    }
    else if (range >= 0xe0 && range <= 0xff)
    {
        return RAM[address & 0xffff];
        // RAM
    }
    printf("read(%x)\n", address);
    return 0;
}

void write_memory(unsigned int address, unsigned int value)
{
    unsigned int range = (address & 0xff0000) >> 16;

    if (range >= 0 && range <= 0x3f)
    {
        // ROM
        ROM[address] = value;
        return;
    }
    else if (range == 0xa0)
    {
        // Z80 space
        return;
    }
    else if (range == 0xa1)
    {
        // I/O and registers
        if (address >= 0xa10000 && address < 0xa10020)
        {
            io_write_memory(address & 0x1f, value);
            return;
        }
        return;
    }
    else if (range >= 0xc0 && range <= 0xdf)
    {
        // VDP
        return;
    }
    else if (range >= 0xe0 && range <= 0xff)
    {
        // RAM
        RAM[address & 0xffff] = value;
        return;
    }
    printf("write(%x, %x)\n", address, value);
    return;
}

unsigned int m68k_read_memory_8(unsigned int address)
{
    //printf("read(8, %x)\n", address);

    return read_memory(address);
}
unsigned int m68k_read_memory_16(unsigned int address)
{
    //printf("read(16, %x)\n", address);

    unsigned int range = (address & 0xff0000) >> 16;

    if (range >= 0xc0 && range <= 0xdf)
    {
        vdp_read(address);
    }
    else
    {
        unsigned int word = read_memory(address)<<8 | read_memory(address+1);
        return word;
    }
}
unsigned int m68k_read_memory_32(unsigned int address)
{
    //printf("read(32, %x)\n", address);

    unsigned int longword = read_memory(address) << 24 |
                            read_memory(address+1) << 16 |
                            read_memory(address+2) << 8 |
                            read_memory(address+3);
    return longword;
}
void m68k_write_memory_8(unsigned int address, unsigned int value)
{
    //printf("write(8, %x, %x)\n", address, value);
    
    write_memory(address, value);

    return;
}
void m68k_write_memory_16(unsigned int address, unsigned int value)
{
    //printf("write(16, %x, %x)\n", address, value);
    unsigned int range = (address & 0xff0000) >> 16;

    if (range >= 0xc0 && range <= 0xdf)
    {
        vdp_write(address, value);
    }
    else
    {
        write_memory(address, (value>>8)&0xff);
        write_memory(address+1, (value)&0xff);
    }
}
void m68k_write_memory_32(unsigned int address, unsigned int value)
{
    //printf("write(32, %x, %x)\n", address, value);
    m68k_write_memory_16(address, (value>>16)&0xffff);
    m68k_write_memory_16(address+2, (value)&0xffff);

    return;
}

void frame()
{
    extern unsigned char vdp_reg[0x20];
    extern unsigned int vdp_status;
    vdp_status &= 0xfff7;
    int hint_counter = vdp_reg[10];
    int line;
    for (line=0; line < 224; line++)
    {
        vdp_set_hblank();
        m68k_execute(CPL_M68K - 404);
        vdp_clear_hblank();

        hint_counter--;
        if (hint_counter < 0)
        {
            hint_counter = vdp_reg[10];
            m68k_set_irq(4);
        }

        vdp_render_line(line);

        m68k_execute(CPL_M68K);
    }

    vdp_set_vblank();
    m68k_execute(CPL_M68K - 360);
    vdp_clear_vblank();

    for (;line < 262; line++)
    {
        vdp_set_hblank();
        m68k_execute(CPL_M68K - 404);
        vdp_clear_hblank();
        m68k_set_irq(6);

        m68k_execute(CPL_M68K);
    }

    vdp_render_sprites();
}

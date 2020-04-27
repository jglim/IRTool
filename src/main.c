#include <gb/gb.h>
#include <stdio.h>
#include <gb/hardware.h>
#include "gb-ir-bg.h"

#define VERSION 100

#define IR_INIT \
__asm__("push	hl ; save HL since we will trash it"); \
__asm__("ld	hl, #0xFF56 ; load RP_REG address at 0xFF56");

#define IR_UNINIT \
__asm__("pop	hl ; restore HL register");

// pulse commands require INIT_IR first so that HL registers point the right way
#define IR_ON \
__asm__("ld	(hl), #01 ; IR on");

#define IR_OFF \
__asm__("ld	(hl), #00 ; IR off");

#define IR_WAIT \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \
__asm__("nop"); \

// marks and spaces are composed of 33 on-off cycles
#define IR_MARK \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \
IR_ON; IR_WAIT; IR_OFF; IR_WAIT; \

// omitting last row = 32 cycles since we have to compensate for loop delay
#define IR_SPACE \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \
IR_OFF; IR_WAIT; IR_OFF; IR_WAIT; \



UWORD bkgPalette[] = {
	RGB_WHITE, RGB_ORANGE, RGB_RED, RGB_BLACK
};

UWORD spritePalette[] = {
	(UINT16)0, RGB_RED, RGB_BLACK, RGB_BLACK,
	(UINT16)0, RGB_YELLOW, RGB_YELLOW, RGB_YELLOW
};

unsigned char fnt[] =
{
0x7C,0x7C,0xC6,0xC6,0xCE,0xCE,0xDE,0xDE,
0xF6,0xF6,0xE6,0xE6,0x7C,0x7C,0x00,0x00,
0x30,0x30,0x70,0x70,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xFC,0xFC,0x00,0x00,
0x78,0x78,0xCC,0xCC,0x0C,0x0C,0x38,0x38,
0x60,0x60,0xCC,0xCC,0xFC,0xFC,0x00,0x00,
0x78,0x78,0xCC,0xCC,0x0C,0x0C,0x38,0x38,
0x0C,0x0C,0xCC,0xCC,0x78,0x78,0x00,0x00,
0x1C,0x1C,0x3C,0x3C,0x6C,0x6C,0xCC,0xCC,
0xFE,0xFE,0x0C,0x0C,0x1E,0x1E,0x00,0x00,
0xFC,0xFC,0xC0,0xC0,0xF8,0xF8,0x0C,0x0C,
0x0C,0x0C,0xCC,0xCC,0x78,0x78,0x00,0x00,
0x38,0x38,0x60,0x60,0xC0,0xC0,0xF8,0xF8,
0xCC,0xCC,0xCC,0xCC,0x78,0x78,0x00,0x00,
0xFC,0xFC,0xCC,0xCC,0x0C,0x0C,0x18,0x18,
0x30,0x30,0x30,0x30,0x30,0x30,0x00,0x00,
0x78,0x78,0xCC,0xCC,0xCC,0xCC,0x78,0x78,
0xCC,0xCC,0xCC,0xCC,0x78,0x78,0x00,0x00,
0x78,0x78,0xCC,0xCC,0xCC,0xCC,0x7C,0x7C,
0x0C,0x0C,0x18,0x18,0x70,0x70,0x00,0x00,
0x30,0x30,0x78,0x78,0xCC,0xCC,0xCC,0xCC,
0xFC,0xFC,0xCC,0xCC,0xCC,0xCC,0x00,0x00,
0x7E,0x7E,0x60,0x60,0x60,0x60,0x78,0x78,
0x60,0x60,0x60,0x60,0x60,0x60,0x00,0x00,
0x3C,0x3C,0x66,0x66,0xC0,0xC0,0xC0,0xC0,
0xC0,0xC0,0x66,0x66,0x3C,0x3C,0x00,0x00,
0xF8,0xF8,0x6C,0x6C,0x66,0x66,0x66,0x66,
0x66,0x66,0x6C,0x6C,0xF8,0xF8,0x00,0x00,
0x00,0x00,0x18,0x18,0x3C,0x3C,0x7E,0x7E,
0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


// default: 24deg fan1 mode2
volatile BYTE symbols[] = 
{
1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 
1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 
0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 
0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 

1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 
1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 
0, 0, 0, 0, 1, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // original test packet ends at the void
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

UBYTE irCursor = 0;

UBYTE lastKey = 0;
UBYTE loopIndex = 0;

UBYTE acFan = 3; // 0: auto, 1:1, 2:3, 3:3
UBYTE acTemperature = 24; // 16-30
UBYTE acMode = 0; // tbd
UBYTE acUpdateType = 0; // 0 -> update only, 1 -> toggle state

UBYTE selectedAcProperty = 1;

// write long pulse: 4 symbols of HIGH, 4 symbols of LOW
// used to start and end a packet
void wl()
{
	symbols[irCursor++] = 1;
	symbols[irCursor++] = 1;
	symbols[irCursor++] = 1;
	symbols[irCursor++] = 1;
	symbols[irCursor++] = 0;
	symbols[irCursor++] = 0;
	symbols[irCursor++] = 0;
	symbols[irCursor++] = 0;
}

// write medium pulse: 1 symbol of HIGH, 3 symbols of LOW
void wm()
{
	symbols[irCursor++] = 1;
	symbols[irCursor++] = 0;
	symbols[irCursor++] = 0;
	symbols[irCursor++] = 0;
}

// write short pulse: 1 symbol of HIGH, 1 symbol of LOW
void ws()
{
	symbols[irCursor++] = 1;
	symbols[irCursor++] = 0;
}

void irTransmitBuffer()
{
	__asm__("ld	b,b"); // debugger

	disable_interrupts();
	loopIndex = 0;
	while (loopIndex < 0xFF)
	{
		if (symbols[loopIndex])
		{
			// IR on (mark)
			IR_INIT;
			IR_MARK;
			IR_UNINIT;
		}
		else 
		{
			// IR off (space)
			IR_INIT;
			IR_SPACE;
			IR_UNINIT;
		}
		loopIndex++;
	}
	enable_interrupts();
}


void irResetPacket()
{
	// clear buffer and reset cursor
	loopIndex = 0;
	while (loopIndex < 0xFF)
	{
		symbols[loopIndex] = 0;
		loopIndex++;
	}
	irCursor = 0;
}

void irWriteUnknown()
{
	ws();ws();ws();ws();
}

void irWriteUpdateFlag()
{
	if (acUpdateType == 0)
	{
		wm(); // update
	}
	else
	{
		ws(); // toggle power state (default)
	}
}

void irWriteMode()
{
	/*
	SSS : FAN
	SMS : COOL
	MMS : DRY
	MMM : AUTO INCREASE (ignore)
	MSM : AUTO DECREASE (ignore)
	SMM : AUTO MAINTAIN (ignore)
	*/		

	if (acMode == 0)
	{
		// cool
		ws(); wm(); ws();
	}
	else if (acMode == 1)
	{
		// dry
		wm(); wm(); ws();
	}
	else if (acMode == 2)
	{
		// auto
		wm(); wm(); wm();
	}
	else if (acMode == 3)
	{
		// fan
		ws(); ws(); ws();
	}
}

void irWriteFan()
{
	if (acFan == 1)
	{
		ws(); wm(); ws(); ws();
	}
	else if (acFan == 2)
	{
		ws(); ws(); wm(); ws();
	}
	else if (acFan == 3)
	{
		ws(); wm(); wm(); ws();
	}
	else if (acFan == 0)
	{
		// auto
		wm(); wm(); wm(); wm();
	}
}

void irWriteTemperature()
{
	if (acTemperature == 16)
	{
		wm(); ws(); ws(); ws();
	}
	else if (acTemperature == 17)
	{
		ws(); wm(); ws(); ws();
	}
	else if (acTemperature == 18)
	{
		wm(); wm(); ws(); ws();
	}
	else if (acTemperature == 19)
	{
		ws(); ws(); wm(); ws();
	}
	else if (acTemperature == 20)
	{
		wm(); ws(); wm(); ws();
	}
	else if (acTemperature == 21)
	{
		ws(); wm(); wm(); ws();
	}
	else if (acTemperature == 22)
	{
		wm(); wm(); wm(); ws();
	}
	else if (acTemperature == 23)
	{
		ws(); ws(); ws(); wm();
	}
	else if (acTemperature == 24)
	{
		wm(); ws(); ws(); wm();
	}
	else if (acTemperature == 25)
	{
		ws(); wm(); ws(); wm();
	}
	else if (acTemperature == 26)
	{
		wm(); wm(); ws(); wm();
	}
	else if (acTemperature == 27)
	{
		ws(); ws(); wm(); wm();
	}
	else if (acTemperature == 28)
	{
		wm(); ws(); wm(); wm();
	}
	else if (acTemperature == 29)
	{
		ws(); wm(); wm(); wm();
	}
	else if (acTemperature == 30)
	{
		wm(); wm(); wm(); wm();
	}
}

void irCreatePacket()
{
	irResetPacket();

	// preamble
	wl();

	irWriteTemperature();
	irWriteFan();
	irWriteTemperature();
	irWriteFan();

	irWriteMode();
	irWriteUpdateFlag();
	irWriteUnknown();
	irWriteMode();
	irWriteUpdateFlag();
	irWriteUnknown();

	// terminate packet
	wl();
	ws();

}

void refreshNumbers()
{
	// temp
	if (acTemperature == 16)
	{
		set_sprite_tile(0, 1);
		set_sprite_tile(1, 6);
	}
	else if (acTemperature == 17)
	{
		set_sprite_tile(0, 1);
		set_sprite_tile(1, 7);
	}
	else if (acTemperature == 18)
	{
		set_sprite_tile(0, 1);
		set_sprite_tile(1, 8);
	}
	else if (acTemperature == 19)
	{
		set_sprite_tile(0, 1);
		set_sprite_tile(1, 9);
	}
	else if (acTemperature == 20)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 0);
	}
	else if (acTemperature == 21)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 1);
	}
	else if (acTemperature == 22)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 2);
	}
	else if (acTemperature == 23)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 3);
	}
	else if (acTemperature == 24)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 4);
	}
	else if (acTemperature == 25)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 5);
	}
	else if (acTemperature == 26)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 6);
	}
	else if (acTemperature == 27)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 7);
	}
	else if (acTemperature == 28)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 8);
	}
	else if (acTemperature == 29)
	{
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 9);
	}
	else if (acTemperature == 30)
	{
		set_sprite_tile(0, 3);
		set_sprite_tile(1, 0);
	}

	// mode
	if (acMode == 0)
	{
		// cool
		set_sprite_tile(2, 12);
	}
	else if (acMode == 1)
	{
		// dry
		set_sprite_tile(2, 13);
	}
	else if (acMode == 2)
	{
		// auto
		set_sprite_tile(2, 10);
	}
	else if (acMode == 3)
	{
		// fan
		set_sprite_tile(2, 11);
	}
	
	// fan
	if (acFan == 0)
	{
		set_sprite_tile(3, 10);
	}
	else if (acFan == 1)
	{
		set_sprite_tile(3, 1);
	}
	else if (acFan == 2)
	{
		set_sprite_tile(3, 2);
	}
	else if (acFan == 3)
	{
		set_sprite_tile(3, 3);
	}

	// cursor
	if (selectedAcProperty == 0)
	{
		move_sprite(4, 40, 120); // 40 84 128
	}
	else if (selectedAcProperty == 1)
	{
		move_sprite(4, 84, 120); // 40 84 128
	}
	else if (selectedAcProperty == 2)
	{
		move_sprite(4, 128, 120); // 40 84 128
	}
}

void beepFeedback()
{
	NR10_REG = 0x38U;
	NR11_REG = 0x70U;
	NR12_REG = 0xE0U;
	NR13_REG = 0x0AU;
	NR14_REG = 0xC6U;
	NR51_REG |= 0x11;
}

void main(void)
{	
	// initialize font
	SPRITES_8x8;
	// load 15 raw sprites
	set_sprite_data(0, 15, fnt);

	// init 2 palettes
	set_sprite_palette(0, 2, spritePalette);
	
	// set all sprites to use palette zero
	set_sprite_prop(0, 0);
	set_sprite_prop(1, 0);
	set_sprite_prop(2, 0);
	set_sprite_prop(3, 0);
	set_sprite_prop(4, 0);
	set_sprite_prop(5, 0);
	set_sprite_prop(6, 0);
	set_sprite_prop(7, 0);
	set_sprite_prop(8, 0);
	set_sprite_prop(9, 0);
	set_sprite_prop(10, 0);
	set_sprite_prop(11, 0);
	set_sprite_prop(12, 0);
	set_sprite_prop(13, 0);
	set_sprite_prop(14, 0);
	set_sprite_prop(15, 0);

	// set sprite images
	set_sprite_tile(4, 14);
	refreshNumbers();

	// move sprites to their locations
	move_sprite(0, 80, 110); // temp digit 1
	move_sprite(1, 88, 110); // temp digit 2
	move_sprite(2, 128, 110); // mode
	move_sprite(3, 41, 110); // fan, nudged 1px right

	// cursor "little arrow"
	//move_sprite(4, 128, 120); // 40 84 128
	SHOW_SPRITES;

	// initialize display
	set_bkg_data(0, gbirbg_tile_count, gbirbg_tile_data);
	VBK_REG = 1;
	VBK_REG = 0;
	set_bkg_tiles(0, 0, gbirbg_tile_map_width, gbirbg_tile_map_height, gbirbg_map_data);
	set_bkg_palette(0, 1, bkgPalette); 
	SHOW_BKG;
	DISPLAY_ON;

	// switch into CGB double clock speed
	disable_interrupts();
	cpu_fast();
	enable_interrupts();

	// initialize audio
	NR50_REG = 0xFF;
	NR51_REG = 0xFF;
	NR52_REG = 0x80;


	while(1) 
	{
		lastKey = joypad();
		// left/right for switching between aircon attributes
		if (lastKey == J_RIGHT)
		{
			if (selectedAcProperty == 0) { selectedAcProperty = 1; }
			else if (selectedAcProperty == 1) { selectedAcProperty = 2; }
			else if (selectedAcProperty == 2) { selectedAcProperty = 2; }
		}
		else if (lastKey == J_LEFT)
		{
			if (selectedAcProperty == 2) { selectedAcProperty = 1; }
			else if (selectedAcProperty == 1) { selectedAcProperty = 0; }
			else if (selectedAcProperty == 0) { selectedAcProperty = 0; }
		}
		// up/down for incrementing/decrementing aircon attributes
		else if (lastKey == J_UP)
		{
			if (selectedAcProperty == 0) 
			{ 
				acFan++;
				if (acFan > 3)
				{
					acFan = 3;
				}
			}
			else if (selectedAcProperty == 1) 
			{
				acTemperature++;
				if (acTemperature > 30)
				{
					acTemperature = 30;
				}
			}
			else if (selectedAcProperty == 2) 
			{
				acMode++;
				if (acMode > 3)
				{
					acMode = 3;
				}
			}
		}
		else if (lastKey == J_DOWN)
		{
			if (selectedAcProperty == 0) 
			{ 
				if (acFan != 0)
				{
					acFan--;
				}
			}
			else if (selectedAcProperty == 1) 
			{
				if (acTemperature != 16)
				{
					acTemperature--;
				}
			}
			else if (selectedAcProperty == 2) 
			{
				if (acMode != 0)
				{
					acMode--;
				}
			}
		}
		else if (lastKey == J_SELECT)
		{

		}
		else if (lastKey == J_START)
		{
	
		}
		else if (lastKey == J_A)
		{
			// commit changes AND change ac state
			acUpdateType = 1;
			beepFeedback();
			irCreatePacket();
			irTransmitBuffer();
		}
		else if (lastKey == J_B)
		{
			// commit changes only
			acUpdateType = 0;
			beepFeedback();
			irCreatePacket();
			irTransmitBuffer();
		}
		refreshNumbers();
		delay(200);
	}
}

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "chip8.h"

CHP chip8;

// To be run before each test
static void before_each()
{
    initialise_chip8(&chip8);
}

// Test 1
static void initialise_chip8_test()
{
    // This test ensures that the initialise_chip8() function works correctly
    // by correctly initialising the required values
	
    before_each();

    assert(chip8.PC == 0x200);
    assert(chip8.SP == 0x000);
    assert(chip8.DT == 0x000);
    assert(chip8.ST == 0x000);

    // Check font has been copied into memory
    for (int i = 0; i < sizeof(font); i++)
    {
        assert(chip8.memory[i] == font[i]);
    }
}

// Test 2
static void load_rom_success_test()
{
    // This test ensures that the load_rom() function works correctly
    // by successfully loading a chosen ROM into CHIP-8 memory, assuming
    // that there are no errors with opening the ROM file.
	
    before_each();

    const char* test_rom_path = "roms/test-rom.ch8";
	
    // Load ROM call
    load_rom(test_rom_path, &chip8);

    // Check that the ROM has been loaded into CHIP-8's memory 
	
    unsigned char test_buffer[4096]; // Create a test buffer with the size of CHIP-8 memory
    memset(test_buffer, 0, sizeof(test_buffer));

    FILE *fptr = fopen(test_rom_path, "rb");
    fread(test_buffer, sizeof(char), sizeof(chip8.memory) - 0x200, fptr);
    fclose(fptr);
	
    for (int i = 0; i < 4096 - 0x200; i++)
    {
        assert(test_buffer[i] == chip8.memory[0x200 + i]);
    }
}

// Test 3
static void load_rom_failure_test()
{
    // This test ensures that the load_rom() function fails when it receives an
    // invalid ROM.
	
    before_each();

    const char* invalid_rom_path = "invalid-rom.ch8";

    // Here we are making a reference to the original standard output so
    // we can refer to it later when standard output becomes redirected
    FILE *original = stdout;
    stdout = fopen("/tmp/stdoutput.txt", "w+");

    // Load ROM call
    load_rom(invalid_rom_path, &chip8);

    unsigned char output_buffer[100];

    fseek(stdout, 0L, SEEK_END);
    long int size = ftell(stdout);
    fseek(stdout, 0L, SEEK_SET);

    fread(output_buffer, size, 1, stdout);

    stdout = original;

    // Check that the standard output gives the correct response
    const char *invalid_rom_string = "Invalid ROM path: 'invalid-rom.ch8'\n\0";
	
    assert(strcmp((const char *)output_buffer, invalid_rom_string) == 0);
}

// Test 4
static void fetch_test()
{
    // This test ensures that the fetch() function returns the correct opcode
    // at the given program counter address.
	
    before_each();

    // Load an instruction into CHIP-8 memory starting from the beginning of program space
    chip8.memory[0x200]	= 0xF0;
    chip8.memory[0x200 + 1] = 0x15;

    // Check that the correct opcode is returned.
    assert(fetch(&chip8) == 0xF015);
}

// Test 5
static void decode_00EE_test()
{
    // This test ensures that when given the opcode 0x00EE, the decode() function
    // returns from a subroutine.
	
    before_each();

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Set up some fake values for CHIP-8 to test returning from a subroutine.
    chip8.stack[0] = 0xFFF;
    chip8.SP = 1;

    // Call decode() with the opcode 0x00EE
    decode(0x00EE, &chip8);

    // Check that values have been set correctly
    assert(chip8.SP == 0);
    assert(chip8.PC == 0xFFF);
}

// Test 6
static void decode_1NNN_test()
{
    // This test ensures that when given the opcode 1NNN, the decode() function
    // jumps the program counter to another memory address.
	
    before_each();

    // Set up some fake values for CHIP-8 to test jumping to another memory address.
    chip8.PC = 0x200;
    chip8.memory[0x500] = 0x20;
    chip8.memory[0x500 + 1] = 0x30;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x1NNN, where N is the new memory location.
    decode(0x1500, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500); // Account for cancelling PC increment
    assert(chip8.memory[chip8.PC] == 0x20);
    assert(chip8.memory[chip8.PC + 1] == 0x30);
}

// Test 7
static void decode_2NNN_test()
{
    // This test ensures that when given the opcode 2NNN, the decode() function
    // will call a subroutine at the memory location NNN.
	
    before_each();

    // Set up some fake values for CHIP-8 to test calling a subroutine
    chip8.PC = 0x200; 

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x2NNN, where N is location of the subroutine.
    decode(0x2500, &chip8);

    // Check that the values have been set correctly
    assert(chip8.stack[0] == 0x200 + 2);
    assert(chip8.SP == 1);
    assert(chip8.PC == 0x500);
}	

// Test 8
static void decode_3XNN_skip_test()
{
    // This test ensures that when given the opcode 3XNN, the decode() function
    // will skip one instruction if the value in VX is equal to NN.

    before_each();

    // Set up some fake values for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x3XNN, where X is the index of V and N is 
    // is the value to compare.
    decode(0x3545, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 4);	
}

// Test 9
static void decode_3XNN_no_skip_test()
{
    // This test ensures that when given the opcode 3XNN, the decode() function
    // will not skip an instruction if the value in VX is not equal to NN.
	
    before_each();

    // Set up some fake value for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x3XNN, where X is the index of V and N is
    // the value to compare.
    decode(0x3535, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 2);
}

// Test 10
static void decode_4XNN_skip_test()
{
    // This test ensures that when given the opcode 4XNN, the decode() function
    // will skip an instruction if the value in VX is not equal to NN.
	
    before_each();

    // Set up some fake value for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x4XNN, where X is the index of V and N is
    // the value to compare.
    decode(0x4535, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 4);
}

// Test 11
static void decode_4XNN_no_skip_test()
{
    // This test ensures that when given the opcode 4XNN, the decode() function
    // will not skip an instruction if the value in VX is equal to NN.
	
    before_each();

    // Set up some fake values for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;
	
    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x4XNN, where X is the index of V and N is 
    // is the value to compare.
    decode(0x4545, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 2);	
}

// Test 12
static void decode_5XY0_skip_test()
{
    // This test ensures that when given the opcode 5XY0, the decode() function
    // will skip an instruction if the values in VX and VY are equal.
	
    before_each();

    // Set up some fake values for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;
    chip8.V[6] = 0x45;
	
    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x5XY0, where X is the index of VX and Y
    // is the index of VY.
    decode(0x5560, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 4);
}

// Test 13
static void decode_5XY0_no_skip_test()
{
    // This test ensures that when given the opcode 5XY0, the decode() function
    // will not skip an instruction if the values in VX and VY are not equal.
	
    before_each();

    // Set up some fake values for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;
    chip8.V[6] = 0x40;
	
    // Increment the program counter to simulate the update function
    chip8.PC += 2;
	 
    // Call decode() with the opcode 0x5XY0, where X is the index of VX and Y
    // is the index of VY.
    decode(0x5560, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 2);
}

// Test 14
static void decode_9XY0_skip_test()
{
    // This test ensures that when given the opcode 9XY0, the decode() function
    // will skip an instruction if the values in VX and VY are not equal.
	
    before_each();

    // Set up some fake values for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;
    chip8.V[6] = 0x45;
	
    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x9XY0, where X is the index of VX and Y
    // is the index of VY.
    decode(0x9560, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 2);
}

// Test 15
static void decode_9XY0_no_skip_test()
{
    // This test ensures that when given the opcode 9XY0, the decode() function
    // will skip an instruction if the values in VX and VY are not equal.
	
    before_each();

    // Set up some fake values for CHIP-8 to test skipping
    chip8.PC = 0x500;
    chip8.V[5] = 0x45;
    chip8.V[6] = 0x40;
	
    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x9XY0, where X is the index of VX and Y
    // is the index of VY.
    decode(0x9560, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC == 0x500 + 4);
}

// Test 16
static void decode_6XNN_test()
{
    // This test ensures that when given the opcode 6XNN, the decode() function
    // sets the register VX to NN.
	
    before_each();

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x6XNN, where X is the index for V and NN
    // is the value to set VX to
    decode(0x6530, &chip8);

    // Check that the values have been set correctly
    assert(chip8.V[5] == 0x30);
}

// Test 17
static void decode_7XNN_test()
{
    // This test ensures that when given the opcode 7XNN, the decode() function
    // adds the value NN to the register VX.
	
    before_each();

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x7XNN, where X is the index for V and NN
    // is the value to add to VX.
    decode(0x7550, &chip8);

    // Check that the values have been set correctly
    assert(chip8.V[5] == 0x50);
}	

// Test 18
static void decode_8XY0_test()
{
    // This test ensures that when given the opcode 8XY0, the decode() function
    // sets VX to VY.
	
    before_each();

    // Set up some fake values to test setting VX to VY
    chip8.V[0] = 0x10;
    chip8.V[5] = 0xFF;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x8XY0, where X is the index for VX and Y is
    // the index for VY.
    decode(0x8050, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0xFF);
}

// Test 19
static void decode_8XY1_test()
{
    // This test ensures that when given the opcode 8XY1, the decode() function
    // sets VX to the binary OR of VX and VY.

    before_each();

    // Set up some fake values to test setting VX to the binary OR of 
    // VX and VY
    chip8.V[0] = 0x6C;
    chip8.V[5] = 0xE1;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x8XY1, where X is the index for VX and Y is
    // the index for VY.
    decode(0x8051, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0xED);
}

// Test 20
static void decode_8XY2_test()
{
    // This test ensures that when given the opcode 8XY2, the decode() function
    // sets VX to the binary AND of VX and VY.
	
    before_each();

    // Set up some fake values to test setting VX to the binary AND of 
    // VX and VY
    chip8.V[0] = 0x0F;
    chip8.V[5] = 0xF0;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x8XY2, where X is the index for VX and Y is
    // the index for VY.
    decode(0x8052, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0x00);
}

// Test 21
static void decode_8XY3_test()
{
    // This test ensures that when given the opcode 8XY3, the decode() function
    // sets VX to the binary XOR of VX and VY.
	
    before_each();

    // Set up and some fake values to test setting VX to the binary XOR of
    // VX and VY
    chip8.V[0] = 0x6C;
    chip8.V[5] = 0xE1;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x8XY3, where X is the index for VX and Y is
    // the index for VY.
    decode(0x8053, &chip8);
	
    // Check the values have been set correctly
    assert(chip8.V[0] == 0x8D);
}

// Test 22
static void decode_8XY4_no_overflow_test()
{
    // This test ensures that when given the opcode 8XY4, the decode() function
    // sets VX to the value of VX plus the value of VY.
    // This addition will affect the carry flag (VF).
    // This test should not overflow, so the carry flag should be zero.
	
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0x01;
    chip8.V[5] = 0x10;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x8XY4 where X is the index for VX and Y is
    // the index for VY.
    decode(0x8054, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0x11);
    assert(chip8.V[0xf] == 0);
}


// Test 23
static void decode_8XY4_overflow_test()
{
    // This test ensures that when given the opcode 8XY4, the decode() function
    // sets VX to the value of VX plus the value of VY.
    // This addition will affect the carry flag (VF).
    // This test should overflow, so the carry flag should be one.
	
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0xFF;
    chip8.V[5] = 0xFF;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call decode() with the opcode 0x8XY4 where X is the index for VX and Y is
    // the index for VY.
    decode(0x8054, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0xFE);
    assert(chip8.V[0xF] == 1);
}

// Test 24
static void decode_8XY5_no_underflow_test()
{
    // This test ensures that when given the opcode 8XY5, the decode() function
    // sets VX to the value of VX minus the value of VY,
    // This subtraction will affect the carry flag (VF).
    // This test should not underflow, so the carry flag should be one.
	
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0xFF;
    chip8.V[5] = 0x0F;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode() with the opcode 0x8XY5 where X is the index for VX and Y is
    // the index for VY.
    decode(0x8055, &chip8);

    // Check values have been set correctly
    assert(chip8.V[0] == 0xF0);
    assert(chip8.V[0xF] == 1);
}

// Test 25
static void decode_8XY5_underflow_test()
{
    // This test ensures that when given the opcode 8XY5, the decode() function
    // sets VX to the value of VX minus the value of VY,
    // This subtraction will affect the carry flag (VF).
    // This test should underflow, so the carry flag should be zero.
	
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0x0F;
    chip8.V[5] = 0x2F;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode() with the opcode 0x8XY5 where X is the index for VX and Y is
    // the index for VY.
    decode(0x8055, &chip8);

    // Check values have been set correctly
    assert(chip8.V[0] == 0xE0);
    assert(chip8.V[0xF] == 0);
}

// Test 26
static void decode_8XY6_shift_out_test()
{
    // This test ensures that when given the opcode 8XY6, the decode() function
    // sets VX to the value of VY shifted 1 bit to the right.
    // This test should result in a 1 bit being shifted out, so VF should
    // become 1.
    
    before_each();

	// Set up and some fake values to test
	chip8.V[0] = 0x00;
	chip8.V[5] = 0x0F;

	// Increment the program counter to simulate the update function
	chip8.PC += 2;

	// Call the decode() with the opcode 0x8XY5 where X is the index for VX and Y is
	// the index for VY.
	decode(0x8056, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0x07);
    assert(chip8.V[0xF] == 1);
}

// Test 27
static void decode_8XY6_no_shift_out_test()
{
    // This test ensures that when given the opcode 8XY6, the decode() function
    // sets VX to the value of VY shifted 1 bit to the right.
    // This test should not result in  1 bit being shifted out, so VF should
    // become 0.
    
    before_each();

	// Set up and some fake values to test
	chip8.V[0] = 0x00;
	chip8.V[5] = 0xF0;

	// Increment the program counter to simulate the update function
	chip8.PC += 2;

	// Call the decode() with the opcode 0x8XY6 where X is the index for VX and Y is
	// the index for VY.
	decode(0x8056, &chip8);

    // Check the values have been set correctly
    assert(chip8.V[0] == 0x78);
    assert(chip8.V[0xF] == 0);
}

// Test 30
static void decode_8XY7_no_underflow_test()
{
    // This test ensures that when given the opcode 8XY7, the decode() function
    // sets VX to the value of VY minus the value of VX,
    // This subtraction will affect the carry flag (VF).
    // This test should not underflow, so the carry flag should be one.
        
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0x0F;
    chip8.V[5] = 0xFF;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode() with the opcode 0x8XY7 where X is the index for VX and Y is
    // the index for VY.
    decode(0x8057, &chip8);

    // Check values have been set correctly
    assert(chip8.V[0] == 0xF0);
    assert(chip8.V[0xF] == 1);
}

// Test 31
static void decode_8XY7_underflow_test()
{
    // This test ensures that when given the opcode 8XY7, the decode() function
    // sets VX to the value of VX minus the value of VY,
    // This subtraction will affect the carry flag (VF).
    // This test should underflow, so the carry flag should be one.
	
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0x2F;
    chip8.V[5] = 0x0F;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode() with the opcode 0x8XY7 where X is the index for VX and Y is
    // the index for VY.
    decode(0x8057, &chip8);
	
    // Check values have been set correctly
    assert(chip8.V[0] == 0xE0);
    assert(chip8.V[0xF] == 0);
}

// Test 28
static void decode_8XYE_shift_out_test()
{
    // This test ensures that when given the opcode 8XYE, the decode() function
    // sets VX to the value of VY shifted 1 bit to the left.
    // This test should result in a 1 bit being shifted out, so VF should
    // become 1.
    
    before_each();

    // Set up and some fake values to test
    chip8.V[0] = 0x00;
    chip8.V[5] = 0xF0;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode() with the opcode 0x8XYE where X is the index for VX and Y is
    // the index for VY.
    decode(0x805E, &chip8);
    
    // Check the values have been set correctly
    assert(chip8.V[0] == 0xE0);
    assert(chip8.V[0xF] == 1);
}

// Test 29
static void decode_8XYE_no_shift_out_test()
{
    // This test ensures that when given the opcode 8XYE, the decode() function
    // sets VX to the value of VY shifted 1 bit to the left.
    // This test should not result in a 1 bit being shifted out, so VF should
    // become 0.
    
    before_each();

	// Set up and some fake values to test
	chip8.V[0] = 0x00;
	chip8.V[5] = 0x0F;

	// Increment the program counter to simulate the update function
	chip8.PC += 2;

	// Call the decode() with the opcode 0x8XYE where X is the index for VX and Y is
	// the index for VY.
	decode(0x805E, &chip8);
   
    // Check the values have been set correctly
    assert(chip8.V[0] == 0x1E);
    assert(chip8.V[0xF] == 0);
}

// Test 32
static void decode_ANNN_test()
{
    // This test ensures that when given the opcdoe ANNN, the decode() function
    // sets CHIP-8's index (I) to the value NNN.
    
    before_each();

    // Call the decode function with the opcode 0xANNN, where NNN is the value
    // to set the index to
    decode(0xA123, &chip8);

    // Check that the values have been set correctly
    assert(chip8.I == 0x123);
}

// Test 33
static void decode_BNNN_test()
{
    // This test ensures that when given the opcode BNNN, the decode() function
    // successfully jumps to the location NNN with the offset of the value in
    // the register V0.

    before_each();

    // Set up some fake values for testing
    chip8.V[0] = 0x02;
    
    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xBNNN, where NNN is the value
    // to jump to
    decode(0xB123, &chip8);
    
    // Check that the values have been set correctly
    assert(chip8.PC == 0x125);
}

// Test 33
static void decode_CXNN_test()
{

    // This test ensures that when given the opcode CXNN, the decode() function
    // will put a random value which has been binary ANDed with NN into VX

    before_each();

    // Set up some fake values for testing
    chip8.V[5] = 0xFF;

    // Increment the program counter to simulate the update function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xCXNN, where X is the index of 
    // V and NN is the value to binary AND with a random number
    decode(0xC511, &chip8);

    // Check that the values have been set correctly
    assert(chip8.PC != 0xFF);
}

// Test 34
static void decode_DXYN_no_on_test()
{
    // This test ensures that when given the opcode DXYN, the decode() function
    // will draw an N pixels tall sprite from the memory location that the I 
    // index register is holding to the screen, at the horizontal X coordinate
    // in VX and the Y coordinate in VY.
    // All pixels that are "on" in the sprite will flip the pixels on the screen
    // that it is drawn to. If any pixels on the screen were turned "off" by
    // this, the VF flag register is set to 1, otherwise it is set to 0.
    
    before_each();

    // Set up some initial values for testing
    chip8.I = 0x000;

    chip8.memory[0x000] =     0xF0;
    chip8.memory[0x000 + 1] = 0xF0;
    chip8.memory[0x000 + 2] = 0xF0;
    chip8.memory[0x000 + 3] = 0xF0;

    chip8.V[0] = 0x20;
    chip8.V[1] = 0x10;

    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            pixel_coords[i][j] = 0;
        }
    }

    // Increment the program counter to simulate the update function
    chip8.PC += 2;
        
    // Call the decode function with the opcode 0xDXYN, where X and Y are
    // indexes of V and N is the number of rows of data
    decode(0xD014, &chip8);
   
    // Check that the values have been set correctly
    assert(chip8.V[0xF] == 0);

    assert(pixel_coords[32][16] == 1); 
    assert(pixel_coords[33][16] == 1);
    assert(pixel_coords[34][16] == 1);
    assert(pixel_coords[35][16] == 1);
    assert(pixel_coords[32][17] == 1);
    assert(pixel_coords[33][17] == 1);
    assert(pixel_coords[34][17] == 1);
    assert(pixel_coords[35][17] == 1);
    assert(pixel_coords[32][18] == 1);
    assert(pixel_coords[33][18] == 1);
    assert(pixel_coords[34][18] == 1);
    assert(pixel_coords[35][18] == 1);
    assert(pixel_coords[32][19] == 1);
    assert(pixel_coords[33][19] == 1);
    assert(pixel_coords[34][19] == 1);
    assert(pixel_coords[35][19] == 1);
}

// Test 35
static void decode_DXYN_on_test()
{
    // This test ensures that when given the opcode DXYN, the decode() function
    // will draw an N pixels tall sprite from the memory location that the I 
    // index register is holding to the screen, at the horizontal X coordinate
    // in VX and the Y coordinate in VY.
    // All pixels that are "on" in the sprite will flip the pixels on the screen
    // that it is drawn to. If any pixels on the screen were turned "off" by
    // this, the VF flag register is set to 1, otherwise it is set to 0.
    
    before_each();

    // Set up some initial values for testing
    chip8.I = 0x000;

    chip8.memory[0x000] =     0xF0;
    chip8.memory[0x000 + 1] = 0xF0;
    chip8.memory[0x000 + 2] = 0xF0;
    chip8.memory[0x000 + 3] = 0xF0;

    chip8.V[0] = 0x20;
    chip8.V[1] = 0x10;

    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            pixel_coords[i][j] = 1;
        }
    }

    // Increment the program counter to simulate the update function
    chip8.PC += 2;
        
    // Call the decode function with the opcode 0xDXYN, where X and Y are
    // indexes of V and N is the number of rows of data
    decode(0xD014, &chip8);
   
    // Check that the values have been set correctly
    assert(chip8.V[0xF] == 1);

    assert(pixel_coords[32][16] == 0); 
    assert(pixel_coords[33][16] == 0);
    assert(pixel_coords[34][16] == 0);
    assert(pixel_coords[35][16] == 0);
    assert(pixel_coords[32][17] == 0);
    assert(pixel_coords[33][17] == 0);
    assert(pixel_coords[34][17] == 0);
    assert(pixel_coords[35][17] == 0);
    assert(pixel_coords[32][18] == 0);
    assert(pixel_coords[33][18] == 0);
    assert(pixel_coords[34][18] == 0);
    assert(pixel_coords[35][18] == 0);
    assert(pixel_coords[32][19] == 0);
    assert(pixel_coords[33][19] == 0);
    assert(pixel_coords[34][19] == 0);
    assert(pixel_coords[35][19] == 0);
}

// Test 36
static void decode_FX07_test()
{
    // This test ensures that when given the opcode FX07, the decode() function
    // sets VX to the current value of the delay timer.
    
    before_each();

    // Set up some initial values to test
    chip8.DT = 0xFF;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX07, where X is the index of 
    // V
    decode(0xF007, &chip8);

    // Check that values have been set correctly
    assert(chip8.V[0] == 0xFF);
}

// Test 37
static void decode_FX15_test()
{
    // This test ensures that when given the opcode FX15, the decode() function
    // sets the delay timer to the value in VX
    
    before_each();

    // Set up some initial values to test
    chip8.V[0] = 0xFF;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX15, where X is the index of
    // V
    decode(0xF015, &chip8);

    // Check that values have been set correctly
    assert(chip8.DT == 0xFF);
}

// Test 38
static void decode_FX18_test()
{
    // This test ensures that when given the opcode FX18, the decode() function
    // sets the sound timer to the value in VX
    
    before_each();

    // Set up some initial values to test
    chip8.V[0] = 0xFF;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX18, where X is the index of 
    // V
    decode(0xF018, &chip8);

    // Check that values have been set correctly
    assert(chip8.ST == 0xFF);
}

// Test 39
static void decode_FX1E_test()
{
    // This test ensures the when given the opcode FX1E, the value of VX is
    // added to the index (I)

    before_each();

    // Set up some initial values to test
    chip8.V[0] = 0x10;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX1E, where X is the index of
    // V
    decode(0xF01E, &chip8);

    // Check that values have been set correctly
    assert(chip8.I == 0x10);
}

// Test 40
static void decode_FX33_test()
{
    // This test ensures that when given the opcode FX33, the binary-coded
    // decimal version of V[x] will be put into the index (I)
    
    before_each();

    // Set up some values to test
    chip8.V[0] = 0xFF;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX33, where X is the index of
    // V
    decode(0xF033, &chip8);

    // Check that values have been set correctly
    assert(chip8.memory[chip8.I] == 2);
    assert(chip8.memory[chip8.I + 1] == 5);
    assert(chip8.memory[chip8.I + 2] == 5);
}

// Test 41
static void decode_FX55_test()
{
    // This test ensures that when given the opcode FX55, stores memory from 
    // the V registers into the index (I).

    before_each();

    // Set up some values to test
    chip8.V[0] = 0x01;
    chip8.V[1] = 0x02;
    chip8.V[2] = 0x03;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX55, where X is the index of
    // V
    decode(0xF355, &chip8);

    // Check that values have been set correctly
    assert(chip8.memory[chip8.I] == 0x01);
    assert(chip8.memory[chip8.I + 1] == 0x02);
    assert(chip8.memory[chip8.I + 2] == 0x03);
}

// Test 42
static void decode_FX65_test()
{
    // This test ensures that when given the opcode FX65, loads memory from 
    // the memory at index (i) into the V registers.

    before_each();

    // Set up some values to test
    chip8.memory[chip8.I] = 0x01;
    chip8.memory[chip8.I + 1] = 0x02;
    chip8.memory[chip8.I + 2] = 0x03;

    // Increment the program counter to simulate the update() function
    chip8.PC += 2;

    // Call the decode function with the opcode 0xFX65, where X is the index of
    // V
    decode(0xF365, &chip8);

    // Check that values have been set correctly
    assert(chip8.V[0] == 0x01);
    assert(chip8.V[1] == 0x02);
    assert(chip8.V[2] == 0x03);
}

int main()
{
	// Run each test
	initialise_chip8_test();
	load_rom_success_test();
	load_rom_failure_test();
	fetch_test();
    decode_00EE_test();
    decode_1NNN_test();
    decode_2NNN_test();
    decode_3XNN_skip_test();
    decode_3XNN_skip_test();
    decode_4XNN_skip_test();
    decode_4XNN_no_skip_test();
    decode_5XY0_skip_test();
    decode_5XY0_no_skip_test();
    decode_9XY0_skip_test();
    decode_9XY0_no_skip_test();
    decode_6XNN_test();
    decode_7XNN_test();
    decode_8XY0_test();
    decode_8XY1_test();
    decode_8XY2_test();
    decode_8XY3_test();
    decode_8XY4_no_overflow_test();
    decode_8XY4_overflow_test();
    decode_8XY5_no_underflow_test();
    decode_8XY5_underflow_test();
    decode_8XY6_shift_out_test();
    decode_8XY6_no_shift_out_test();
    decode_8XY7_no_underflow_test();
    decode_8XYE_shift_out_test();
    decode_8XYE_no_shift_out_test();
    decode_8XY7_underflow_test();
    decode_ANNN_test();
    decode_BNNN_test();
    decode_CXNN_test();
    decode_DXYN_no_on_test();
    decode_DXYN_on_test();
    decode_FX07_test();
    decode_FX15_test();
    decode_FX18_test();
    decode_FX1E_test();
    decode_FX33_test();
    decode_FX55_test();
    decode_FX65_test();

	printf("All tests passed.\n");

	return 0;
}

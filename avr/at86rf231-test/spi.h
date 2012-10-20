#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

uint8_t readSPI_register (uint8_t addr);

uint8_t writeSPI_register(uint8_t addr, uint8_t content);

uint8_t accessSPI_register(uint8_t write, uint8_t addr, uint8_t content);


uint8_t readFrame(uint8_t *frame);

void writeFrame(uint8_t *frame, uint8_t length);

#endif

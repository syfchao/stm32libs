/*
 * hd44780-basic.cpp
 *
 *  Created on: Aug 11, 2013
 *      Author: agu
 */

#include "hd44780/hd44780-basic.h"

const uint8_t Hd44780Basic::BAR_PATTERN[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00000000
	0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 11000000
	0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, // 00011000
	0x1f, 0x1f, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, // 11011000
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, // 00000011
	0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, // 11000011
	0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x1f, 0x1f, // 00011011
	0x1f, 0x1f, 0x00, 0x1f, 0x1f, 0x00, 0x1f, 0x1f, // 11011011
};

const uint8_t Hd44780Basic::ROW_INDEX_16[] = { 0x00, 0x40, 0x10, 0x50 };

const uint8_t Hd44780Basic::ROW_INDEX_20[] = { 0x00, 0x40, 0x14, 0x54 };

Hd44780Basic::Hd44780Basic(uint8_t row_count, uint8_t col_count) :
		_row_count(row_count), _col_count(col_count) {
	_cache_length = _row_count * _col_count + 1;
	_cache = (char *) malloc(sizeof(char) * _cache_length);
	this->setCache();
}

Hd44780Basic::~Hd44780Basic() {
	free(_cache);
}

void Hd44780Basic::setCache(uint8_t value) {
	memset(_cache, value, _cache_length);
}

void Hd44780Basic::setCache(uint8_t index, uint8_t value) {
	if (index >= _cache_length)
		return;

	_cache[index] = value;
}

void Hd44780Basic::putString(uint8_t address, char *p, uint8_t length) const {
	char *pp = p;

	this->setCursor(address);

	while (length--)
		transmit(true, *pp++);
}

void Hd44780Basic::putChar(uint8_t address, char c) const {
	this->putString(address, &c, 1);
}

void Hd44780Basic::init() {
	this->initHardware();

	this->setCGRam(BAR_PATTERN, 64);
	this->configureDisplay(true, false, false);
}

void Hd44780Basic::putCache() const {
	for (uint8_t r = 0; r < _row_count; r++)
		this->putString(
						*((_col_count <= 16 ?
								ROW_INDEX_16 : ROW_INDEX_20)
								+ r), _cache + _col_count * r, _col_count);
}

void Hd44780Basic::printf(uint8_t index, const char *__fmt, ...) {
	if (index >= _cache_length)
		return;

	va_list ap;
	va_start(ap, __fmt);
	vsnprintf(_cache + index, _cache_length - index, __fmt, ap);
	va_end(ap);
}

void Hd44780Basic::printf(const char *__fmt, ...) {
	va_list ap;
	va_start(ap, __fmt);
	vsnprintf(_cache, _cache_length, __fmt, ap);
	va_end(ap);
}

void Hd44780Basic::clear() const // 0x01
{
	transmit(false, 0x01);
	delayMicroseconds(2000);
}

void Hd44780Basic::rst() const // 0x02
{
	transmit(false, 0x02);
	delayMicroseconds(2000);
}

void Hd44780Basic::configureInput(bool ac, bool screen_move) const // 0x04
		{
	uint8_t cmd = 0x04;

	if (ac)
		cmd |= 0x02;
	if (screen_move)
		cmd |= 0x01;

	transmit(false, cmd);
}

void Hd44780Basic::configureDisplay(bool display_on, bool cursor,
		bool blink) const // 0x08
		{
	uint8_t cmd = 0x08;
	if (display_on)
		cmd |= 0x04;
	if (cursor)
		cmd |= 0x02;
	if (blink)
		cmd |= 0x01;

	transmit(false, cmd);
}

void Hd44780Basic::moveCursor(bool right) const // 0x10
		{
	uint8_t cmd = 0x10;
	if (right)
		cmd |= 0x04;
	transmit(false, cmd);
}

void Hd44780Basic::moveScreen(bool right) const // 0x11
		{
	uint8_t cmd = 0x11;
	if (right)
		cmd |= 0x04;
	transmit(false, cmd);
}

void Hd44780Basic::configureFunction(bool interface8, bool doubleline,
		bool font5x10) const // 0x20
		{
	uint8_t cmd = 0x20;
	if (interface8)
		cmd |= 0x10;
	if (doubleline)
		cmd |= 0x08;
	if (font5x10)
		cmd |= 0x04;
	transmit(false, cmd);
}

void Hd44780Basic::setCGRam(uint8_t const *pFont, uint8_t length) const {
	this->configureInput(true, false);
	transmit(false, 0x40);

	for (uint8_t i = 0; i < length; i++) {
		transmit(true, pFont[i]);
	}
}

void Hd44780Basic::setCursor(uint8_t address) const {
	transmit(false, address | 0x80);
}

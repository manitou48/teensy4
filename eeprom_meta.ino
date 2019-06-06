// Teensy 4  EEPROM meta data  wear leveling flash
#include <EEPROM.h>
#define FLASH_BASEADDR 0x601F0000
#define FLASH_SECTORS 15
#define E2END 0x437

static uint16_t sector_index[FLASH_SECTORS];

void ee_init() {
  uint32_t sector, addr, offset;

  for (sector = 0; sector < FLASH_SECTORS; sector++) {
    const uint16_t *p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
    const uint16_t *end = (uint16_t *)(FLASH_BASEADDR + (sector + 1) * 4096);
    uint16_t index = 0, val, last_val = 0xffff;
    do {
      val = *p++;
      if (val == 0xFFFF) break;
      index++;
      last_val = val;
    } while (p < end);
    sector_index[sector] = index;
    Serial.printf("sector %d  index %d", sector, index);
    if (index) {
      offset = last_val & 255;
      addr = 4 * sector + 4 * FLASH_SECTORS * (offset >> 2) + (offset & 3);
      Serial.printf("  last %d:%d", addr, last_val >> 8);
    }
    Serial.printf("\n");

  }
}

void eval(uint32_t addr) {
  uint32_t  sector, offset;

  sector = (addr >> 2) % FLASH_SECTORS;
  offset = (addr & 3) | (((addr >> 2) / FLASH_SECTORS) << 2);
  Serial.printf("addr %d sector %d  offset %d \n", addr, sector, offset);
}

void ee_read(uint32_t addr) {
  uint32_t  sector, offset, last_index, index = 0, cnt = 0;
  uint16_t *p, *end;
  uint8_t data = 0xFF;

  sector = (addr >> 2) % FLASH_SECTORS;
  offset = (addr & 3) | (((addr >> 2) / FLASH_SECTORS) << 2);
  p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
  end = p + sector_index[sector];
  while (p < end) {
    uint32_t val = *p++;
    if ((val & 255) == offset) {
      data = val >> 8;
      last_index = index;   // very last one is current
      cnt++;
    }
    index++;
  }
  *p--;
  Serial.printf("ee addr:val %d:%d  sector %d offset %d  p %0x  last index %d cnt %d\n",
                addr, EEPROM.read(addr), sector, offset, (uint32_t)p, last_index, cnt);
}

void sector_dump(uint32_t sector) {
  // 16-bit words  data|offset
  uint32_t   index = 0;
  uint16_t *p, *end;

  p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
  end = p + sector_index[sector];
  Serial.printf("sector %d  end %d  0x%0x to 0x%0x\n",
                sector, sector_index[sector], (uint32_t)p, (uint32_t)p + 4095 );
  while (p < end) {
    uint32_t val;
    Serial.printf("%04d  ", index);
    for (int i = 0; i < 16; i++) {
      val = *p++;
      Serial.printf("%04x ", val);
    }
    Serial.printf("\n");
    index += 16;
  }
}

void sector_report(uint32_t sector) {
  uint32_t offset, val, cnts[256];
  uint16_t *p, *end;
  uint8_t  vals[256];
  int i;

  for (i = 0; i < sizeof(vals); i++) cnts[i] = vals[i] = 0;
  Serial.printf("sector %d index %d\n", sector, sector_index[sector]);
  p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
  end = p + sector_index[sector];
  while (p < end) {
    val = *p++;
    cnts[val & 255]++;
    vals[val & 255] = val >> 8;  // last value
  }
  for (i = 0; i < sizeof(vals); i++) {
    uint32_t addr = 4 * sector + 4 * FLASH_SECTORS * (i >> 2) + (i & 3);
    if (cnts[i]) Serial.printf("offset %d  %d:%d  count %d\n", i, addr, vals[i], cnts[i]);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  uint32_t bytes = 4096 * (FLASH_SECTORS + 1);
  Serial.printf("EEPROM lth %d flash %d sectors %d bytes  0x%0x to 0x%0x\n",
                EEPROM.length(), FLASH_SECTORS + 1, bytes, FLASH_BASEADDR, FLASH_BASEADDR + bytes - 1);
  // EEPROM.write(1050,0x12);
  ee_init();
  eval(0);
  eval(60);
  eval(1025);
  eval(1026);
  eval(1027);
  eval(1050);
  eval(1079);
  ee_read(0);
  ee_read(1);
  ee_read(60);
  ee_read(61);
  ee_read(1050);
  sector_dump(0);
  sector_dump(7);
  sector_dump(10);
  sector_report(10);
#if 0
  for (int i = 0; i < 50; i++) {
    EEPROM.write(40, i);
    delay(5);
  }
  // need to update sector index table
#endif
}

void loop() {
}

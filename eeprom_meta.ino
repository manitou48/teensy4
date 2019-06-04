// Teensy 4  EEPROM meta data  wear leveling flash
#include <EEPROM.h>
#define FLASH_BASEADDR 0x601F0000
#define FLASH_SECTORS 15
#define E2END 0x437

static uint16_t sector_index[FLASH_SECTORS];

void ee_init() {
  uint32_t sector;

  for (sector = 0; sector < FLASH_SECTORS; sector++) {
    const uint16_t *p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
    const uint16_t *end = (uint16_t *)(FLASH_BASEADDR + (sector + 1) * 4096);
    uint16_t index = 0;
    do {
      if (*p++ == 0xFFFF) break;
      index++;
    } while (p < end);
    sector_index[sector] = index;
    Serial.printf("sector %d  index %d\n", sector, index);
  }
}

void eval(uint32_t addr) {
  uint32_t  sector, offset;
  uint16_t *p, *end;
  sector = (addr >> 2) % FLASH_SECTORS;
  offset = (addr & 3) | (((addr >> 2) / FLASH_SECTORS) << 2);
  p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
  end = p + sector_index[sector];
  Serial.printf("addr %d sector %d  offset %d  p %0x  end %0x\n",
                addr, sector, offset, (uint32_t) p, (uint32_t) end);
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
  Serial.printf("ee byte %d: %d  sector %d offset %d  p %0x  last index %d cnt %d\n",
                addr, EEPROM.read(addr), sector, offset, (uint32_t)p, last_index, cnt);
}

void sector_dump(uint32_t sector) {
  // 16-bit words  data|offset
  uint32_t   index = 0;
  uint16_t *p, *end;

  Serial.printf("sector %d  end %d\n", sector, sector_index[sector]);
  p = (uint16_t *)(FLASH_BASEADDR + sector * 4096);
  end = p + sector_index[sector];
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


void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.printf("EEPROM lth %d\n", EEPROM.length());
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
}

void loop() {
}

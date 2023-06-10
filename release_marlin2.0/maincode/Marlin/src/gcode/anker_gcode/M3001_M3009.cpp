/*
 * @Author       : winter
 * @Date         : 2022-05-24 14:08:20
 * @LastEditors: winter.tian
 * @LastEditTime: 2023-04-12 11:57:22
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"
#include "../gcode.h"
#include "../../module/settings.h"
#include "../../module/temperature.h"

#if ENABLED(ANKER_NOZZLE_BOARD)
#include "../../feature/anker/anker_nozzle_board.h"
#include "../../feature/interactive/uart_nozzle_tx.h"
#include "../../feature/interactive/uart_nozzle_rx.h"

#if 0
// #define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//16 Kbytes
// #define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//16 Kbytes
// #define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//16 Kbytes
// #define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//16 Kbytes
// #define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//64 Kbytes
// #define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//128 Kbytes
// #define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//128 Kbytes
// #define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//128 Kbytes
// #define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//128 Kbytes
// #define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//128 Kbytes
// #define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//128 Kbytes
// #define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//128 Kbytes

#define FLASH_TIMEOUT_VALUE 50000U /* 50 s */
#define FLASH_BOOTLOADER_ADDR_START ((uint32_t)0x08000000)
#define FLASH_BOOTLOADER_LENGTH_MAX ((uint32_t)0x0000C000)
#define FLASH_CHECK_ADDR_START ((uint32_t)0x080C0000)
#define FLASH_CHECK_ADDR_END ((uint32_t)0x080DFFFF)
typedef struct
{
  __IO uint32_t init_version_length_addr;
  __IO uint32_t init_version_crc_addr;
  __IO uint32_t last_version_length_addr;
  __IO uint32_t last_version_crc_addr;
  uint32_t init_version_length_value;
  uint32_t init_version_crc_value;
  uint32_t last_version_length_value;
  uint32_t last_version_crc_value;
} flash_check_info_t;
static flash_check_info_t flash_check_info = {FLASH_CHECK_ADDR_START, FLASH_CHECK_ADDR_START + 4, FLASH_CHECK_ADDR_START + 8, FLASH_CHECK_ADDR_START + 12};
#endif

#define FLASH_BOOTLOADER_ADDR_START ((uint32_t)0x08000000)
#define FLASH_BOOTLOADER_LENGTH_MAX ((uint32_t)0x0000C000)
static const unsigned int crc32tab[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
    0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
    0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
    0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
    0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
    0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
    0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
    0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
    0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
    0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
    0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
    0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
    0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
    0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
    0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
    0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
    0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
    0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
    0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
    0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
    0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
    0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
    0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
    0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
    0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
    0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
    0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
    0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
    0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
    0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
    0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
    0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
    0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
    0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
    0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
    0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
    0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
    0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
    0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
    0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
    0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
    0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
    0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
    0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
    0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
    0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
    0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
    0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL};
static unsigned int crc32(const unsigned char *buf, unsigned int size)
{
  unsigned int i, crc;
  crc = 0xFFFFFFFF;

  for (i = 0; i < size; i++)
    crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);

  return crc ^ 0xFFFFFFFF;
}

void GcodeSuite::M3001()
{
    if (IS_new_nozzle_board())
    {
        M3001_add_on();
        return;
    }

    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

    if (p_info->read_threshold_flag == 1)
    {
        p_info->tx_ring_buf_add((char *)"M3001");
    }
}

void GcodeSuite::M3002()
{
    if (IS_new_nozzle_board())
    {
        M3002_add_on();
        return;
    }
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

    if (p_info->read_threshold_flag == 1)
    {
        p_info->tx_ring_buf_add((char *)"M3002");
    }
}

// M3003 get nozzle board threshold
// M3003 V200 set nozzle board threshold
void GcodeSuite::M3003()
{
    if (IS_new_nozzle_board())
    {
        M3003_add_on();
        return;
    }
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
  char tmp_buf[ANKER_NOZZLE_BOARD_TX_BUF_SIZE] = {0};
  bool is_set_threshold = false;
  bool is_set_fireproof0 = false;
  bool is_set_fireproof1 = false;

  if (parser.seen('V'))
  {
    p_info->threshold = parser.value_int();
    is_set_threshold = true;
  }
  if (parser.seen('A'))
  {
    p_info->fireproof_adc0 = parser.value_int();
    is_set_fireproof0 = true;
  }
  if (parser.seen('B'))
  {
    p_info->fireproof_adc1 = parser.value_int();
    is_set_fireproof1 = true;
  }

  if ((!is_set_threshold) && (!is_set_fireproof0) && (!is_set_fireproof1))
  {
    SERIAL_ECHOPAIR("echo:M3003 V ", p_info->threshold, "\r\n");
    SERIAL_ECHOPAIR("echo:M3003 A0 ", p_info->fireproof_adc0, "\r\n");
    SERIAL_ECHOPAIR("echo:M3003 A1 ", p_info->fireproof_adc1, "\r\n");
  }

  if (is_set_threshold)
  {
    is_set_threshold = false;
    settings.save();
    SERIAL_ECHOPAIR("echo:M3003 V", p_info->threshold, "\r\n");

    p_info->serial_begin();

    memset(tmp_buf, 0, sizeof(tmp_buf));
    sprintf(tmp_buf, "M3003 %d", p_info->threshold);
    p_info->tx_ring_buf_add(tmp_buf);

    if (is_set_fireproof0 || is_set_fireproof1)
    {
      if (is_set_fireproof0)
      {
        is_set_fireproof0 = false;
        SERIAL_ECHOPAIR("echo:M3008 ", p_info->fireproof_adc0, "\r\n");
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "M3008 %d", p_info->fireproof_adc0);
        p_info->tx_ring_buf_add(tmp_buf);
      }

      if (is_set_fireproof1)
      {
        is_set_fireproof1 = false;
        SERIAL_ECHOPAIR("echo:M3009 ", p_info->fireproof_adc1, "\r\n");
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "M3009 %d", p_info->fireproof_adc1);
        p_info->tx_ring_buf_add(tmp_buf);
      }
    }

    if (p_info->threshold != 0)
    {
      p_info->read_threshold_flag = 0;
      memset(tmp_buf, 0, sizeof(tmp_buf));
      sprintf(tmp_buf, "M3010");
      p_info->tx_ring_buf_add(tmp_buf);
    }
    else
    {
      p_info->read_threshold_flag = 1;
    }

    p_info->power_reset();
  }
  else
  {
    SERIAL_ECHOPAIR("echo:M3003 V", p_info->threshold, "\r\n");
  }
}

// M3004 get nozzle board software version
void GcodeSuite::M3004()
{
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

  if (strlen(p_info->software_version) > 0)
    SERIAL_ECHOPAIR("echo:M3004 ", p_info->software_version, "\r\n");
  else
    SERIAL_ECHOPAIR("echo:M3004 0.0.0\r\n");
}

// M3005 get abnormal_reset_times com_error_times
void GcodeSuite::M3005()
{
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
  SERIAL_ECHOPAIR("echo:M3005,", p_info->heartbeat_abnormal_flag, "\r\n");
}

// M3009 bootloader check
void GcodeSuite::M3009()
{
  uint8_t read_crc_flag = 0;
  uint32_t length = 0, crc_value = 0, calc_crc = 0;

  if (parser.seen('L'))
  {
    length = parser.value_ulong();
    if (length > FLASH_BOOTLOADER_LENGTH_MAX)
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }
  }
  if (parser.seen('C'))
  {
    crc_value = parser.value_ulong();
    read_crc_flag = 0;
  }
  else
  {
    read_crc_flag = 1;
  }

  calc_crc = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
  if (read_crc_flag == 1)
  {
    SERIAL_ECHOPAIR("echo:M3009 success L", length, " C", calc_crc, "\r\n");
  }
  else
  {
    if (calc_crc == crc_value)
    {
      SERIAL_ECHOPAIR("echo:M3009 success L", length, " C", crc_value, "\r\n");
    }
    else
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
    }
  }
}

#if 0
static bool flash_sector_read(uint32_t addr, uint32_t *data)
{
  if ((addr < FLASH_CHECK_ADDR_START) || (addr > FLASH_CHECK_ADDR_END) || (data == NULL))
  {
    SERIAL_ECHOPAIR("flash_sector_read addr is error or data is NULL : ", addr, "\r\n");
    return false;
  }

  *data = *(__IO uint32_t *)addr;

  return true;
}
static bool flash_sector_erase(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t SectorError = 0;
  uint32_t tmp_addr = 0;
  uint32_t tmp_data = 0;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  HAL_FLASH_Unlock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  tmp_addr = FLASH_CHECK_ADDR_START;
  while (tmp_addr < FLASH_CHECK_ADDR_END)
  {
    if (flash_sector_read(tmp_addr, &tmp_data))
    {
      if (tmp_data != 0XFFFFFFFF)
      {
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInitStruct.Sector = FLASH_SECTOR_10;
        EraseInitStruct.NbSectors = 1;

        DISABLE_ISRS();
        status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
        ENABLE_ISRS();
        if (status != HAL_OK)
        {
          SERIAL_ECHOPAIR(" HAL_FLASHEx_Erase = ", status);
          SERIAL_ECHOPAIR(" GetError = ", HAL_FLASH_GetError());
          SERIAL_ECHOPAIR(" SectorError = ", SectorError);
          HAL_FLASH_Lock();
          return false;
        }
        else
        {
          break;
        }
      }
      else
      {
        tmp_addr += 4;
      }
      FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
    }
    else
    {
      SERIAL_ECHOPAIR("flash_sector_read failed!\r\n");
      HAL_FLASH_Lock();
      return false;
    }
  }

  HAL_FLASH_Lock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  return true;
}
static bool flash_sector_write(uint32_t addr, uint32_t data)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  if ((addr < FLASH_CHECK_ADDR_START) || (addr > FLASH_CHECK_ADDR_END))
  {
    SERIAL_ECHOPAIR("flash_sector_write addr is error : ", addr, "\r\n");
    return false;
  }

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  HAL_FLASH_Unlock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
  if (status != HAL_OK)
  {
    SERIAL_ECHOPAIR(" HAL_FLASH_Program = ", status);
    SERIAL_ECHOPAIR(" GetError = ", HAL_FLASH_GetError());
    SERIAL_ECHOPAIR(" address = ", addr);
    HAL_FLASH_Lock();
    return false;
  }

  HAL_FLASH_Lock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  return true;
}
void GcodeSuite::M3009()
{
  uint8_t version = 0;
  uint32_t length = 0, crc_value = 0;

  if (parser.seen('A'))
  {
    if ((!flash_sector_read(flash_check_info.init_version_length_addr, &(flash_check_info.init_version_length_value))) ||
        (!flash_sector_read(flash_check_info.init_version_crc_addr, &(flash_check_info.init_version_crc_value))) ||
        (!flash_sector_read(flash_check_info.last_version_length_addr, &(flash_check_info.last_version_length_value))) ||
        (!flash_sector_read(flash_check_info.last_version_crc_addr, &(flash_check_info.last_version_crc_value))))
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }

    SERIAL_ECHOPAIR("echo:M3009 A 1L", flash_check_info.init_version_length_value, " 1C", flash_check_info.init_version_crc_value,
                    " 2L", flash_check_info.last_version_length_value, " 2C", flash_check_info.last_version_crc_value, "\r\n");
  }
  else if (parser.seen('B'))
  {
    if (parser.seen('L'))
    {
      length = parser.value_ulong();
    }
    if (length <= FLASH_BOOTLOADER_LENGTH_MAX)
    {
      crc_value = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
      SERIAL_ECHOPAIR("echo:M3009 B L", length, " C", crc_value, "\r\n");
    }
    else
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }
  }
  else if (parser.seen('W'))
  {
    SERIAL_ECHOPAIR(parser.command_ptr, "\r\n");
    version = parser.value_byte();
    if (parser.seen('L'))
    {
      length = parser.value_ulong();
      SERIAL_ECHOPAIR("length = ",length,"\r\n");
      if (length > FLASH_BOOTLOADER_LENGTH_MAX)
      {
        SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
        return;
      }
    }
    if (parser.seen('C'))
    {
      crc_value = parser.value_ulong();
      SERIAL_ECHOPAIR("crc = ", crc_value,"\r\n");
    }

    if ((!flash_sector_read(flash_check_info.init_version_length_addr, &(flash_check_info.init_version_length_value))) ||
        (!flash_sector_read(flash_check_info.init_version_crc_addr, &(flash_check_info.init_version_crc_value))) ||
        (!flash_sector_read(flash_check_info.last_version_length_addr, &(flash_check_info.last_version_length_value))) ||
        (!flash_sector_read(flash_check_info.last_version_crc_addr, &(flash_check_info.last_version_crc_value))))
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }

    if (version == 1)
    {
      flash_check_info.init_version_length_value = length;
      flash_check_info.init_version_crc_value = crc_value;
      SERIAL_ECHOPAIR("length = ",flash_check_info.init_version_length_value, "crc = ", flash_check_info.init_version_crc_value,"\r\n");
    }
    else if (version == 2)
    {
      flash_check_info.last_version_length_value = length;
      flash_check_info.last_version_crc_value = crc_value;
    }

    if (!flash_sector_erase())
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }
    if ((!flash_sector_write(flash_check_info.init_version_length_addr, flash_check_info.init_version_length_value)) ||
        (!flash_sector_write(flash_check_info.init_version_crc_addr, flash_check_info.init_version_crc_value)) ||
        (!flash_sector_write(flash_check_info.last_version_length_addr, flash_check_info.last_version_length_value)) ||
        (!flash_sector_write(flash_check_info.last_version_crc_addr, flash_check_info.last_version_crc_value)))
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }

    SERIAL_ECHOPAIR("echo:M3009 W", version, " L", length, " C", crc_value, "\r\n");
  }
  else
  {
    if ((!flash_sector_read(flash_check_info.init_version_length_addr, &(flash_check_info.init_version_length_value))) ||
        (!flash_sector_read(flash_check_info.init_version_crc_addr, &(flash_check_info.init_version_crc_value))) ||
        (!flash_sector_read(flash_check_info.last_version_length_addr, &(flash_check_info.last_version_length_value))) ||
        (!flash_sector_read(flash_check_info.last_version_crc_addr, &(flash_check_info.last_version_crc_value))))
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }

    length = flash_check_info.init_version_length_value;
    if (length <= FLASH_BOOTLOADER_LENGTH_MAX)
    {
      crc_value = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
      if (crc_value == flash_check_info.init_version_crc_value)
      {
        SERIAL_ECHOPAIR("echo:M3009 R1 L", length, " C", crc_value, "\r\n");
      }
      else
      {
        length = flash_check_info.last_version_length_value;
        if (length <= FLASH_BOOTLOADER_LENGTH_MAX)
        {
          crc_value = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
          if (crc_value == flash_check_info.last_version_crc_value)
          {
            SERIAL_ECHOPAIR("echo:M3009 R2 L", length, " C", crc_value, "\r\n");
          }
          else
          {
            SERIAL_ECHOPAIR("echo:M3009 failure Rx L", length, " C", crc_value, "\r\n");
          }
        }
        else
        {
          SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
          return;
        }
      }
    }
    else
    {
      SERIAL_ECHOPAIR("echo:M3009 failure\r\n");
      return;
    }
  }
}
#endif

#endif

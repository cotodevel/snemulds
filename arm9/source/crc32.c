#include <malloc.h>
#include <stdlib.h>

#define CRC16_POLYNOMIAL 0xa001
#define CRC32_POLYNOMIAL 0xedb88320


void
init_crc_table (void *table, unsigned int polynomial)
// works for crc16 and crc32
{
  unsigned int crc, i, j;

  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
        if (crc & 1)
          crc = (crc >> 1) ^ polynomial;
        else
          crc >>= 1;

      if (polynomial == CRC32_POLYNOMIAL)
        ((unsigned int *) table)[i] = crc;
      else
        ((unsigned short *) table)[i] = (unsigned short) crc;
    }
}


static unsigned int *crc32_table = NULL;


static void
free_crc32_table (void)
{
  free (crc32_table);
  crc32_table = NULL;
}


unsigned int
crc32 (unsigned int crc, const void *buffer, unsigned int size)
{
  unsigned char *p = (unsigned char *) buffer;

  if (!crc32_table)
    {
      crc32_table = (unsigned int *) malloc (256 * 4);
      init_crc_table (crc32_table, CRC32_POLYNOMIAL);
    }

  crc = ~crc;
  while (size--)
    crc = (crc >> 8) ^ crc32_table[(crc ^ *p++) & 0xff];
    
  free_crc32_table();
    
  return ~crc;
}

// T4 DCP  AES SHA CRC32
//  tests on channel 0  non-blocking  no context switching

// InputOutput  or just Input
#define __IO volatile
#define __I  volatile
#define DCP_CH0SEMA_VALUE_MASK                   (0xFF0000U)
#define DCP_CH0STAT_ERROR_CODE_MASK              (0xFF0000U)
#define DCP_HASH_BLOCK_SIZE  128
#define DCP_STAT_OTP_KEY_READY_MASK              (0x10000000U)
#define DCP_KEY_INDEX_MASK                       (0x30U)
#define DCP_KEY_INDEX_SHIFT                      (4U)
#define DCP_KEY_INDEX(x)                         (((uint32_t)(((uint32_t)(x)) << DCP_KEY_INDEX_SHIFT)) & DCP_KEY_INDEX_MASK)


enum _generic_status
{
  kStatus_Success = 0,
  kStatus_Fail = 1,
  kStatus_ReadOnly = 2,
  kStatus_OutOfRange = 3,
  kStatus_InvalidArgument = 4,
  kStatus_Timeout = 5,
  kStatus_DCP_Again = 6,
};


typedef enum _dcp_ch_enable
{
  kDCP_chDisable = 0U,    /*!< DCP channel disable */
  kDCP_ch0Enable = 1U,    /*!< DCP channel 0 enable */
  kDCP_ch1Enable = 2U,    /*!< DCP channel 1 enable */
  kDCP_ch2Enable = 4U,    /*!< DCP channel 2 enable */
  kDCP_ch3Enable = 8U,    /*!< DCP channel 3 enable */
  kDCP_chEnableAll = 15U, /*!< DCP channel enable all */
} _dcp_ch_enable_t;

typedef enum _dcp_channel
{
  kDCP_Channel0 = (1u << 16), /*!< DCP channel 0. */
  kDCP_Channel1 = (1u << 17), /*!< DCP channel 1. */
  kDCP_Channel2 = (1u << 18), /*!< DCP channel 2. */
  kDCP_Channel3 = (1u << 19), /*!< DCP channel 3. */
} dcp_channel_t;


typedef enum _dcp_key_slot
{
  kDCP_KeySlot0 = 0U,     /*!< DCP key slot 0. */
  kDCP_KeySlot1 = 1U,     /*!< DCP key slot 1. */
  kDCP_KeySlot2 = 2U,     /*!< DCP key slot 2.*/
  kDCP_KeySlot3 = 3U,     /*!< DCP key slot 3. */
  kDCP_OtpKey = 4U,       /*!< DCP OTP key. */
  kDCP_OtpUniqueKey = 5U, /*!< DCP unique OTP key. */
  kDCP_PayloadKey = 6U,   /*!< DCP payload key. */
} dcp_key_slot_t;

typedef enum _dcp_swap
{
  kDCP_NoSwap = 0x0U,
  kDCP_KeyByteSwap = 0x40000U,
  kDCP_KeyWordSwap = 0x80000U,
  kDCP_InputByteSwap = 0x100000U,
  kDCP_InputWordSwap = 0x200000U,
  kDCP_OutputByteSwap = 0x400000U,
  kDCP_OutputWordSwap = 0x800000U,
} dcp_swap_t;

typedef enum _dcp_hash_algo_t
{
  kDCP_Sha1,   /*!< SHA_1 */
  kDCP_Sha256, /*!< SHA_256 */
  kDCP_Crc32,  /*!< CRC_32 */
} dcp_hash_algo_t;

enum _dcp_hash_digest_len
{
  kDCP_OutLenSha1 = 20u,
  kDCP_OutLenSha256 = 32u,
  kDCP_OutLenCrc32 = 4u,
};

enum _dcp_work_packet_bit_definitions
{
  kDCP_CONTROL0_DECR_SEMAPHOR = 1u << 1, /* DECR_SEMAPHOR */
  kDCP_CONTROL0_ENABLE_HASH = 1u << 6,   /* ENABLE_HASH */
  kDCP_CONTROL0_HASH_INIT = 1u << 12,    /* HASH_INIT */
  kDCP_CONTROL0_HASH_TERM = 1u << 13,    /* HASH_TERM */
  kDCP_CONTROL1_HASH_SELECT_SHA256 = 2u << 16,
  kDCP_CONTROL1_HASH_SELECT_SHA1 = 0u << 16,
  kDCP_CONTROL1_HASH_SELECT_CRC32 = 1u << 16,
};



typedef struct _dcp_hash_ctx_t
{
  uint32_t x[58];
} dcp_hash_ctx_t;

typedef union _dcp_hash_block
{
  uint32_t w[DCP_HASH_BLOCK_SIZE / 4]; /*!< array of 32-bit words */
  uint8_t b[DCP_HASH_BLOCK_SIZE];      /*!< byte array */
} dcp_hash_block_t;

typedef enum _dcp_hash_algo_state
{
  kDCP_StateHashInit = 1u, /*!< Init state. */
  kDCP_StateHashUpdate,    /*!< Update state. */
} dcp_hash_algo_state_t;

typedef struct _dcp_handle
{
  dcp_channel_t channel;  /*!< Specify DCP channel. */
  dcp_key_slot_t keySlot; /*!< For operations with key (such as AES encryption/decryption), specify DCP key slot. */
  uint32_t swapConfig;    /*!< For configuration of key, input, output byte/word swap options */
  uint32_t keyWord[4];
  uint32_t iv[4];
} dcp_handle_t;

typedef struct _dcp_hash_ctx_internal
{
  dcp_hash_block_t blk;        /*!< memory buffer. only full blocks are written to DCP during hash updates */
  size_t blksz;                /*!< number of valid bytes in memory buffer */
  dcp_hash_algo_t algo;        /*!< selected algorithm from the set of supported algorithms */
  dcp_hash_algo_state_t state; /*!< finite machine state of the hash software process */
  uint32_t fullMessageSize;    /*!< track message size */
  uint32_t ctrl0;              /*!< HASH_INIT and HASH_TERM flags */
  uint32_t runningHash[9];     /*!< running hash. up to SHA-256 plus size, that is 36 bytes. */
  dcp_handle_t *handle;
} dcp_hash_ctx_internal_t;

typedef struct _dcp_work_packet
{
  uint32_t nextCmdAddress;
  uint32_t control0;
  uint32_t control1;
  uint32_t sourceBufferAddress;
  uint32_t destinationBufferAddress;
  uint32_t bufferSize;
  uint32_t payloadPointer;
  uint32_t status;
} dcp_work_packet_t;


/** DCP - Register Layout Typedef */
typedef struct {
  __IO uint32_t CTRL;                              /**< DCP control register 0, offset: 0x0 */
  uint8_t RESERVED_0[12];
  __IO uint32_t STAT;                              /**< DCP status register, offset: 0x10 */
  uint8_t RESERVED_1[12];
  __IO uint32_t CHANNELCTRL;                       /**< DCP channel control register, offset: 0x20 */
  uint8_t RESERVED_2[12];
  __IO uint32_t CAPABILITY0;                       /**< DCP capability 0 register, offset: 0x30 */
  uint8_t RESERVED_3[12];
  __I  uint32_t CAPABILITY1;                       /**< DCP capability 1 register, offset: 0x40 */
  uint8_t RESERVED_4[12];
  __IO uint32_t CONTEXT;                           /**< DCP context buffer pointer, offset: 0x50 */
  uint8_t RESERVED_5[12];
  __IO uint32_t KEY;                               /**< DCP key index, offset: 0x60 */
  uint8_t RESERVED_6[12];
  __IO uint32_t KEYDATA;                           /**< DCP key data, offset: 0x70 */
  uint8_t RESERVED_7[12];
  __I  uint32_t PACKET0;                           /**< DCP work packet 0 status register, offset: 0x80 */
  uint8_t RESERVED_8[12];
  __I  uint32_t PACKET1;                           /**< DCP work packet 1 status register, offset: 0x90 */
  uint8_t RESERVED_9[12];
  __I  uint32_t PACKET2;                           /**< DCP work packet 2 status register, offset: 0xA0 */
  uint8_t RESERVED_10[12];
  __I  uint32_t PACKET3;                           /**< DCP work packet 3 status register, offset: 0xB0 */
  uint8_t RESERVED_11[12];
  __I  uint32_t PACKET4;                           /**< DCP work packet 4 status register, offset: 0xC0 */
  uint8_t RESERVED_12[12];
  __I  uint32_t PACKET5;                           /**< DCP work packet 5 status register, offset: 0xD0 */
  uint8_t RESERVED_13[12];
  __I  uint32_t PACKET6;                           /**< DCP work packet 6 status register, offset: 0xE0 */
  uint8_t RESERVED_14[28];
  __IO uint32_t CH0CMDPTR;                         /**< DCP channel 0 command pointer address register, offset: 0x100 */
  uint8_t RESERVED_15[12];
  __IO uint32_t CH0SEMA;                           /**< DCP channel 0 semaphore register, offset: 0x110 */
  uint8_t RESERVED_16[12];
  __IO uint32_t CH0STAT;                           /**< DCP channel 0 status register, offset: 0x120 */
  uint8_t RESERVED_17[12];
  __IO uint32_t CH0OPTS;                           /**< DCP channel 0 options register, offset: 0x130 */
  uint8_t RESERVED_18[12];
  __IO uint32_t CH1CMDPTR;                         /**< DCP channel 1 command pointer address register, offset: 0x140 */
  uint8_t RESERVED_19[12];
  __IO uint32_t CH1SEMA;                           /**< DCP channel 1 semaphore register, offset: 0x150 */
  uint8_t RESERVED_20[12];
  __IO uint32_t CH1STAT;                           /**< DCP channel 1 status register, offset: 0x160 */
  uint8_t RESERVED_21[12];
  __IO uint32_t CH1OPTS;                           /**< DCP channel 1 options register, offset: 0x170 */
  uint8_t RESERVED_22[12];
  __IO uint32_t CH2CMDPTR;                         /**< DCP channel 2 command pointer address register, offset: 0x180 */
  uint8_t RESERVED_23[12];
  __IO uint32_t CH2SEMA;                           /**< DCP channel 2 semaphore register, offset: 0x190 */
  uint8_t RESERVED_24[12];
  __IO uint32_t CH2STAT;                           /**< DCP channel 2 status register, offset: 0x1A0 */
  uint8_t RESERVED_25[12];
  __IO uint32_t CH2OPTS;                           /**< DCP channel 2 options register, offset: 0x1B0 */
  uint8_t RESERVED_26[12];
  __IO uint32_t CH3CMDPTR;                         /**< DCP channel 3 command pointer address register, offset: 0x1C0 */
  uint8_t RESERVED_27[12];
  __IO uint32_t CH3SEMA;                           /**< DCP channel 3 semaphore register, offset: 0x1D0 */
  uint8_t RESERVED_28[12];
  __IO uint32_t CH3STAT;                           /**< DCP channel 3 status register, offset: 0x1E0 */
  uint8_t RESERVED_29[12];
  __IO uint32_t CH3OPTS;                           /**< DCP channel 3 options register, offset: 0x1F0 */
  uint8_t RESERVED_30[524];
  __IO uint32_t DBGSELECT;                         /**< DCP debug select register, offset: 0x400 */
  uint8_t RESERVED_31[12];
  __I  uint32_t DBGDATA;                           /**< DCP debug data register, offset: 0x410 */
  uint8_t RESERVED_32[12];
  __IO uint32_t PAGETABLE;                         /**< DCP page table register, offset: 0x420 */
  uint8_t RESERVED_33[12];
  __I  uint32_t VERSION;                           /**< DCP version register, offset: 0x430 */
} DCP_Type;

#define DCP                                      ((DCP_Type *)0x402FC000)

static void dcp_reverse_and_copy(uint8_t *src, uint8_t *dest, size_t src_len)
{
  for (int i = 0; i < src_len; i++)
  {
    dest[i] = src[src_len - 1 - i];
  }
}


static uint32_t dcp_get_channel_status( dcp_channel_t channel)
{
  uint32_t statReg = 0;
  uint32_t semaReg = 0;
  uint32_t status = kStatus_Fail;

  switch (channel)
  {
    case kDCP_Channel0:
      statReg = DCP->CH0STAT;
      semaReg = DCP->CH0SEMA;
      break;

    case kDCP_Channel1:
      statReg = DCP->CH1STAT;
      semaReg = DCP->CH1SEMA;
      break;

    case kDCP_Channel2:
      statReg = DCP->CH2STAT;
      semaReg = DCP->CH2SEMA;
      break;

    case kDCP_Channel3:
      statReg = DCP->CH3STAT;
      semaReg = DCP->CH3SEMA;
      break;

    default:
      break;
  }

  if (!((semaReg & DCP_CH0SEMA_VALUE_MASK) || (statReg & DCP_CH0STAT_ERROR_CODE_MASK)))
  {
    status = kStatus_Success;
  }

  return status;
}

static void dcp_clear_status()
{
  volatile uint32_t *dcpStatClrPtr = &DCP->STAT + 2u;
  *dcpStatClrPtr = 0xFFu;
}

static void dcp_clear_channel_status( uint32_t mask)
{
  volatile uint32_t *chStatClrPtr;

  if (mask & kDCP_Channel0)
  {
    chStatClrPtr = &DCP->CH0STAT + 2u;
    *chStatClrPtr = 0xFFu;
  }
  if (mask & kDCP_Channel1)
  {
    chStatClrPtr = &DCP->CH1STAT + 2u;
    *chStatClrPtr = 0xFFu;
  }
  if (mask & kDCP_Channel2)
  {
    chStatClrPtr = &DCP->CH2STAT + 2u;
    *chStatClrPtr = 0xFFu;
  }
  if (mask & kDCP_Channel3)
  {
    chStatClrPtr = &DCP->CH3STAT + 2u;
    *chStatClrPtr = 0xFFu;
  }
}


uint32_t DCP_WaitForChannelComplete( dcp_handle_t *handle)
{
  /* wait if our channel is still active */
  while ((DCP->STAT & (uint32_t)handle->channel) == handle->channel)
  {
  }

  if (dcp_get_channel_status(handle->channel) != kStatus_Success)
  {
    dcp_clear_status();
    dcp_clear_channel_status(handle->channel);
    return kStatus_Fail;
  }

  return kStatus_Success;
}


static uint32_t dcp_schedule_work( dcp_handle_t *handle, dcp_work_packet_t *dcpPacket)
{
  uint32_t status;

  /* check if our channel is active */
  if ((DCP->STAT & (uint32_t)handle->channel) != handle->channel)
  {
    noInterrupts();

    /* re-check if our channel is still available */
    if ((DCP->STAT & (uint32_t)handle->channel) == 0)
    {
      volatile uint32_t *cmdptr = NULL;
      volatile uint32_t *chsema = NULL;

      switch (handle->channel)
      {
        case kDCP_Channel0:
          cmdptr = &DCP->CH0CMDPTR;
          chsema = &DCP->CH0SEMA;
          break;

        case kDCP_Channel1:
          cmdptr = &DCP->CH1CMDPTR;
          chsema = &DCP->CH1SEMA;
          break;

        case kDCP_Channel2:
          cmdptr = &DCP->CH2CMDPTR;
          chsema = &DCP->CH2SEMA;
          break;

        case kDCP_Channel3:
          cmdptr = &DCP->CH3CMDPTR;
          chsema = &DCP->CH3SEMA;
          break;

        default:
          break;
      }

      if (cmdptr && chsema)
      {
        /* set out packet to DCP CMDPTR */
        *cmdptr = (uint32_t)dcpPacket;

        /* set the channel semaphore */
        *chsema = 1u;
      }
      status = kStatus_Success;
    }

    else
    {
      status = kStatus_DCP_Again;
    }
    interrupts();
  }

  else
  {
    return kStatus_DCP_Again;
  }

  return status;
}

static uint32_t dcp_hash_update_non_blocking(
  dcp_hash_ctx_internal_t *ctxInternal, dcp_work_packet_t *dcpPacket, const uint8_t *msg, size_t size)
{
  dcpPacket->control0 = ctxInternal->ctrl0 | (ctxInternal->handle->swapConfig & 0xFC0000u) |
                        kDCP_CONTROL0_ENABLE_HASH | kDCP_CONTROL0_DECR_SEMAPHOR;
  if (ctxInternal->algo == kDCP_Sha256)
  {
    dcpPacket->control1 = kDCP_CONTROL1_HASH_SELECT_SHA256;
  }
  else if (ctxInternal->algo == kDCP_Sha1)
  {
    dcpPacket->control1 = kDCP_CONTROL1_HASH_SELECT_SHA1;
  }
  else if (ctxInternal->algo == kDCP_Crc32)
  {
    dcpPacket->control1 = kDCP_CONTROL1_HASH_SELECT_CRC32;
  }
  else
  {
    return 1;
  }
  dcpPacket->sourceBufferAddress = (uint32_t)msg;
  dcpPacket->destinationBufferAddress = 0;
  dcpPacket->bufferSize = size;
  dcpPacket->payloadPointer = (uint32_t)ctxInternal->runningHash;

  return dcp_schedule_work( ctxInternal->handle, dcpPacket);
}

void dcp_hash_update(dcp_hash_ctx_internal_t *ctxInternal, const uint8_t *msg, size_t size)
{
  uint32_t completionStatus;
  dcp_work_packet_t dcpWork = {0};

  do
  {
    completionStatus = dcp_hash_update_non_blocking( ctxInternal, &dcpWork, msg, size);
  } while (completionStatus == kStatus_DCP_Again);

  completionStatus = DCP_WaitForChannelComplete(ctxInternal->handle);

  ctxInternal->ctrl0 = 0; /* clear kDCP_CONTROL0_HASH_INIT and kDCP_CONTROL0_HASH_TERM flags */
  return; // (completionStatus);
}

void dcp_hash_process_message_data(
  dcp_hash_ctx_internal_t *ctxInternal,
  const uint8_t *message,
  size_t messageSize)
{
  /* if there is partially filled internal buffer, fill it to full block */
  if (ctxInternal->blksz > 0)
  {
    size_t toCopy = DCP_HASH_BLOCK_SIZE - ctxInternal->blksz;
    memcpy(&ctxInternal->blk.b[ctxInternal->blksz], message, toCopy);
    message += toCopy;
    messageSize -= toCopy;

    /* process full internal block */
    dcp_hash_update(ctxInternal, &ctxInternal->blk.b[0], DCP_HASH_BLOCK_SIZE);
  }

  /* process all full blocks in message[] */
  uint32_t fullBlocksSize = ((messageSize >> 6) << 6); /* (X / 64) * 64 */
  if (fullBlocksSize > 0)
  {
    dcp_hash_update( ctxInternal, message, fullBlocksSize);
    message += fullBlocksSize;
    messageSize -= fullBlocksSize;
  }

  /* copy last incomplete message bytes into internal block */
  memcpy(&ctxInternal->blk.b[0], message, messageSize);
  ctxInternal->blksz = messageSize;

}


void DCP_HASH_Init(dcp_handle_t *handle, dcp_hash_ctx_t *ctx, dcp_hash_algo_t algo)
{
  dcp_hash_ctx_internal_t *ctxInternal;
  ctxInternal = (dcp_hash_ctx_internal_t *)ctx;
  ctxInternal->algo = algo;
  ctxInternal->blksz = 0u;
  for (int i = 0; i < sizeof(ctxInternal->blk.w) / sizeof(ctxInternal->blk.w[0]); i++)
  {
    ctxInternal->blk.w[i] = 0u;  // bug was 0
  }
  ctxInternal->state = kDCP_StateHashInit;
  ctxInternal->fullMessageSize = 0;
  ctxInternal->handle = handle;

}

void DCP_HASH_Update(dcp_hash_ctx_t *ctx, const uint8_t *input, size_t inputSize)
{
  bool isUpdateState;
  dcp_hash_ctx_internal_t *ctxInternal;
  size_t blockSize;

  ctxInternal = (dcp_hash_ctx_internal_t *)ctx;
  ctxInternal->fullMessageSize += inputSize;
  /* if we are still less than DCP_HASH_BLOCK_SIZE bytes, keep only in context */
  if ((ctxInternal->blksz + inputSize) <= blockSize)
  {
    memcpy((&ctxInternal->blk.b[0]) + ctxInternal->blksz, input, inputSize);
    ctxInternal->blksz += inputSize;
    return ;
  }
  else
  {
    isUpdateState = ctxInternal->state == kDCP_StateHashUpdate;
    if (!isUpdateState)
    {
      /* start NEW hash */
      ctxInternal->ctrl0 = kDCP_CONTROL0_HASH_INIT;
      ctxInternal->state = kDCP_StateHashUpdate;
    }
    else
    {
      //      dcp_hash_restore_running_hash(ctxInternal);  // context switch
    }
  }

  /* process input data */
  dcp_hash_process_message_data(ctxInternal, input, inputSize);
  //  dcp_hash_save_running_hash(ctxInternal);  // context

}

void DCP_HASH_Finish(dcp_hash_ctx_t *ctx, uint8_t *output)
{
  size_t algOutSize = 0;
  dcp_hash_ctx_internal_t *ctxInternal;

  ctxInternal = (dcp_hash_ctx_internal_t *)ctx;

  if (ctxInternal->state == kDCP_StateHashInit)
  {
    ctxInternal->ctrl0 = kDCP_CONTROL0_HASH_INIT;//dcp_hash_engine_init( ctxInternal);

  }
  else
  {
    // dcp_hash_restore_running_hash(ctxInternal);  // context
  }

  size_t outSize = 0u;

  /* compute algorithm output length */
  switch (ctxInternal->algo)
  {
    case kDCP_Sha256:
      outSize = kDCP_OutLenSha256;
      break;
    case kDCP_Sha1:
      outSize = kDCP_OutLenSha1;
      break;
    case kDCP_Crc32:
      outSize = kDCP_OutLenCrc32;
      break;
    default:
      break;
  }
  algOutSize = outSize;

  /* flush message last incomplete block, if there is any, and add padding bits */
  //dcp_hash_finalize( ctxInternal);
  ctxInternal->ctrl0 |= kDCP_CONTROL0_HASH_TERM;
  dcp_hash_update(ctxInternal, &ctxInternal->blk.b[0], ctxInternal->blksz);

  /* Reverse and copy result to output[] */
  dcp_reverse_and_copy((uint8_t *)ctxInternal->runningHash, &output[0], algOutSize);

  memset(ctx, 0, sizeof(dcp_hash_ctx_t));

}

void dcp_init() {
  volatile uint32_t *p;
  CCM_CCGR0 |= CCM_CCGR0_DCP(CCM_CCGR_ON);  // DCP on

  DCP->CTRL = 0xF0800000u; /* reset value */
  DCP->CTRL = 0x30800000u; /* default value */

  dcp_clear_status();
  // clear channel status
  dcp_clear_channel_status(kDCP_Channel0 | kDCP_Channel1 | kDCP_Channel2 | kDCP_Channel3);

  DCP->CTRL = 0x00C00000;  // enable caching  writes
  DCP->CHANNELCTRL = kDCP_ch0Enable;
}

void prhash(unsigned char *h, int n) {
  int i;

  for (i = 0; i < n; i++) {
    Serial.printf("%02x", h[i]);
    if (i % 4 == 3) Serial.printf(" ");
  }
  Serial.printf("\n");
}

void do_sha256() {
  dcp_handle_t m_handle;
  dcp_hash_ctx_t hashCtx;
  uint8_t msg[16 * 1024], hash[32];
  static const uint8_t message[] = "hello";  // hash 2cf24dba ...

  m_handle.channel = kDCP_Channel0;
  m_handle.keySlot = kDCP_KeySlot0;
  m_handle.swapConfig = kDCP_NoSwap;

  DCP_HASH_Init( &m_handle, &hashCtx, kDCP_Sha256);
  DCP_HASH_Update( &hashCtx, message, 5);
  DCP_HASH_Finish( &hashCtx, hash);
  prhash(hash, 32);

  uint32_t t = micros();
  DCP_HASH_Init( &m_handle, &hashCtx, kDCP_Sha256);
  DCP_HASH_Update( &hashCtx, msg, sizeof(msg));
  DCP_HASH_Finish( &hashCtx, hash);
  t = micros() - t;
  Serial.printf("SHA256 %d bytes %d us  %.3f MBs\n", sizeof(msg), t, (float)sizeof(msg) / t);
  prhash(hash, 32);
}

void do_crc32() {
  dcp_handle_t m_handle;
  dcp_hash_ctx_t hashCtx;
  uint8_t msg[16 * 1024], hash[4];
  static const uint8_t message[] = "abcdbcdecdefdefgefghfghighijhijk";
  static const unsigned char crc32[] = {0x7f, 0x04, 0x6a, 0xdd};  // crc

  m_handle.channel = kDCP_Channel0;
  m_handle.keySlot = kDCP_KeySlot0;
  m_handle.swapConfig = kDCP_NoSwap;

  DCP_HASH_Init( &m_handle, &hashCtx, kDCP_Crc32);
  DCP_HASH_Update( &hashCtx, message, sizeof(message) - 1);
  DCP_HASH_Finish( &hashCtx, hash);
  Serial.printf("memcmp %d\n", memcmp(hash, crc32, 4));
  prhash(hash, 4);

  uint32_t t = micros();
  DCP_HASH_Init( &m_handle, &hashCtx, kDCP_Crc32);
  DCP_HASH_Update( &hashCtx, msg, sizeof(msg));
  DCP_HASH_Finish( &hashCtx, hash);
  t = micros() - t;
  Serial.printf("CRC32 %d bytes %d us  %.3f MBs\n", sizeof(msg), t, (float)sizeof(msg) / t);
  prhash(hash, 4);
}

uint32_t dcp_aes_set_sram_based_key( dcp_handle_t *handle, const uint8_t *key)
{
  DCP->KEY = DCP_KEY_INDEX(handle->keySlot);
  /* move the key by 32-bit words */
  int i = 0;
  size_t keySize = 16u;
  while (keySize)
  {
    keySize -= sizeof(uint32_t);
    DCP->KEYDATA = ((uint32_t *)(uintptr_t)key)[i];
    i++;
  }
  return kStatus_Success;
}

uint32_t DCP_AES_EncryptCbcNonBlocking(
  dcp_handle_t *handle,
  dcp_work_packet_t *dcpPacket,
  const uint8_t *plaintext,
  uint8_t *ciphertext,
  size_t size,
  const uint8_t *iv)
{
  /* Size must be 16-byte multiple */
  if ((size < 16u) || (size % 16u))
  {
    return kStatus_InvalidArgument;
  }

  dcpPacket->control0 =
    0x322u | (handle->swapConfig & 0xFC0000u); /* CIPHER_INIT | CIPHER_ENCRYPT | ENABLE_CIPHER | DECR_SEMAPHORE */
  dcpPacket->control1 = 0x10u;                   /* CBC */
  dcpPacket->sourceBufferAddress = (uint32_t)plaintext;
  dcpPacket->destinationBufferAddress = (uint32_t)ciphertext;
  dcpPacket->bufferSize = (uint32_t)size;

  if (handle->keySlot == kDCP_OtpKey)
  {
    dcpPacket->payloadPointer = (uint32_t)iv;
    dcpPacket->control0 |= (1u << 10);   /* OTP_KEY */
    dcpPacket->control1 |= (0xFFu << 8); /* KEY_SELECT = OTP_KEY */
  }
  else if (handle->keySlot == kDCP_OtpUniqueKey)
  {
    dcpPacket->payloadPointer = (uint32_t)iv;
    dcpPacket->control0 |= (1u << 10);   /* OTP_KEY */
    dcpPacket->control1 |= (0xFEu << 8); /* KEY_SELECT = UNIQUE_KEY */
  }
  else if (handle->keySlot == kDCP_PayloadKey)
  {
    /* In this case payload must contain key & iv in one array. */
    /* Copy iv into handle right behind the keyWord[] so we can point payload to keyWord[]. */
    memcpy(handle->iv, iv, 16);
    dcpPacket->payloadPointer = (uint32_t)&handle->keyWord[0];
    dcpPacket->control0 |= (1u << 11); /* PAYLOAD_KEY */
  }
  else
  {
    dcpPacket->payloadPointer = (uint32_t)iv;
    dcpPacket->control1 |= ((uint32_t)handle->keySlot << 8); /* KEY_SELECT = keySlot */
  }

  return dcp_schedule_work( handle, dcpPacket);
}

uint32_t DCP_AES_DecryptCbcNonBlocking(
  dcp_handle_t *handle,
  dcp_work_packet_t *dcpPacket,
  const uint8_t *ciphertext,
  uint8_t *plaintext,
  size_t size,
  const uint8_t *iv)
{
  /* Size must be 16-byte multiple */
  if ((size < 16u) || (size % 16u))
  {
    return kStatus_InvalidArgument;
  }

  dcpPacket->control0 = 0x222u | (handle->swapConfig & 0xFC0000u); /* CIPHER_INIT | ENABLE_CIPHER | DECR_SEMAPHORE */
  dcpPacket->control1 = 0x10u;                                     /* CBC */
  dcpPacket->sourceBufferAddress = (uint32_t)ciphertext;
  dcpPacket->destinationBufferAddress = (uint32_t)plaintext;
  dcpPacket->bufferSize = (uint32_t)size;

  if (handle->keySlot == kDCP_OtpKey)
  {
    dcpPacket->payloadPointer = (uint32_t)iv;
    dcpPacket->control0 |= (1u << 10);   /* OTP_KEY */
    dcpPacket->control1 |= (0xFFu << 8); /* OTP_KEY */
  }
  else if (handle->keySlot == kDCP_OtpUniqueKey)
  {
    dcpPacket->payloadPointer = (uint32_t)iv;
    dcpPacket->control0 |= (1u << 10);   /* OTP_KEY */
    dcpPacket->control1 |= (0xFEu << 8); /* UNIQUE_KEY */
  }
  else if (handle->keySlot == kDCP_PayloadKey)
  {
    /* in this case payload must contain KEY + IV together */
    /* copy iv into handle struct so we can point payload directly to keyWord[]. */
    memcpy(handle->iv, iv, 16);
    dcpPacket->payloadPointer = (uint32_t)&handle->keyWord[0];
    dcpPacket->control0 |= (1u << 11); /* PAYLOAD_KEY */
  }
  else
  {
    dcpPacket->payloadPointer = (uint32_t)iv;
    dcpPacket->control1 |= ((uint32_t)handle->keySlot << 8); /* KEY_SELECT */
  }

  return dcp_schedule_work( handle, dcpPacket);
}

uint32_t DCP_AES_SetKey( dcp_handle_t *handle, const uint8_t *key, size_t keySize)
{
  uint32_t status = kStatus_Fail;

  if ((kDCP_OtpKey == handle->keySlot) || (kDCP_OtpUniqueKey == handle->keySlot))
  {
    /* for AES OTP and unique key, check and return read from fuses status */
    if ((DCP->STAT & DCP_STAT_OTP_KEY_READY_MASK) == DCP_STAT_OTP_KEY_READY_MASK)
    {
      status = kStatus_Success;
    }
  }
  else
  {
    /* only work with aligned key[] */
    if (0x3U & (uintptr_t)key)
    {
      return kStatus_InvalidArgument;
    }

    /* keySize must be 16. */
    if (keySize != 16U)
    {
      return kStatus_InvalidArgument;
    }

    /* move the key by 32-bit words */
    int i = 0;
    while (keySize)
    {
      keySize -= sizeof(uint32_t);
      handle->keyWord[i] = ((uint32_t *)(uintptr_t)key)[i];
      i++;
    }

    if (kDCP_PayloadKey != handle->keySlot)
    {
      /* move the key by 32-bit words to DCP SRAM-based key storage */
      status = dcp_aes_set_sram_based_key(handle, key);
    }
    else
    {
      /* for PAYLOAD_KEY, just return Ok status now */
      status = kStatus_Success;
    }
  }
  return status;
}

void DCP_AES_EncryptCbc(
  dcp_handle_t *handle,
  const uint8_t *plaintext,
  uint8_t *ciphertext,
  size_t size,
  const uint8_t iv[16])
{
  uint32_t completionStatus = kStatus_Fail;
  dcp_work_packet_t dcpWork = {0};

  do
  {
    completionStatus = DCP_AES_EncryptCbcNonBlocking(handle, &dcpWork, plaintext, ciphertext, size, iv);
  } while (completionStatus == kStatus_DCP_Again);

  if (completionStatus != kStatus_Success)
  {
    return ; //completionStatus;
  }

  DCP_WaitForChannelComplete( handle);
}

void DCP_AES_DecryptCbc(
  dcp_handle_t *handle,
  const uint8_t *ciphertext,
  uint8_t *plaintext,
  size_t size,
  const uint8_t iv[16])
{
  uint32_t completionStatus = kStatus_Fail;
  dcp_work_packet_t dcpWork = {0};

  do
  {
    completionStatus = DCP_AES_DecryptCbcNonBlocking( handle, &dcpWork, ciphertext, plaintext, size, iv);
  } while (completionStatus == kStatus_DCP_Again);

  if (completionStatus != kStatus_Success)
  {
    return; // completionStatus;
  }

  DCP_WaitForChannelComplete( handle);
}


void do_aes() {
  static const uint8_t keyAes128[] __attribute__((aligned)) =
  { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
  };
  static const uint8_t plainAes128[] =
  { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
  };
  static const uint8_t ive[] =
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
  };

  static const uint8_t cipherAes128[] =
  { 0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
    0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d
  };
  uint8_t cipher[16], output[16], inmsg[1024], cipherout[1024];
  dcp_handle_t m_handle;

  m_handle.channel = kDCP_Channel0;
  m_handle.swapConfig = kDCP_NoSwap;
  m_handle.keySlot = kDCP_KeySlot0;    // could use OTP key
  DCP_AES_SetKey(&m_handle, keyAes128, 16);
  DCP_AES_EncryptCbc(&m_handle, plainAes128, cipher, 16, ive);
  DCP_AES_DecryptCbc(&m_handle, cipher, output, 16, ive);
  Serial.printf("memcmp %d\n", memcmp(output, plainAes128, 16));
  uint32_t t = micros();
  DCP_AES_EncryptCbc(&m_handle, inmsg, cipherout, sizeof(inmsg), ive);
  t = micros() - t;
  Serial.printf("AES %d bytes %d us  %.3f MBs\n", sizeof(inmsg), t, (float)sizeof(inmsg) / t);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  dcp_init();
  do_sha256();
  do_crc32();
  do_aes();
}

void loop() {

}

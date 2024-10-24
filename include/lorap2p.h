#if (HAS_LORA_P2P)

#ifndef _LORAP2P_H
#define _LORAP2P_H

#include "globals.h"
#include "rcommand.h"
#include <SPI.h>
#include <LoRa.h>
#include <mbedtls/base64.h>

extern TaskHandle_t lorap2pSendTask;

esp_err_t lora_p2p_init(void);
void lora_p2p_task(void *param);

void lora_p2p_enqueuedata(MessageBuffer_t *message);
void lora_p2p_queuereset(void);
uint32_t lora_p2p_queuewaiting(void);


#endif // _LORAP2P_H

#endif // HAS_LORA_P2P
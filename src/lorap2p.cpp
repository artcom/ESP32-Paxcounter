#ifdef HAS_LORA_P2P

#include "lorap2p.h"

static QueueHandle_t LoraP2PSendQueue;
TaskHandle_t lorap2pTask;

Ticker lorap2pTimer;

esp_err_t lora_p2p_init(void) {
  Serial.println("LoRa P2P Sender");

  //SPI LoRa pins
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  // Set the LoRa pins
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa P2P failed!");
    while (1);
  }

  Serial.println("LoRa started");

  _ASSERT(SEND_QUEUE_SIZE > 0);
  LoraP2PSendQueue = xQueueCreate(SEND_QUEUE_SIZE, sizeof(MessageBuffer_t));
  if (LoraP2PSendQueue == 0) {
    ESP_LOGE(TAG, "Could not create LoRa P2P send queue. Aborting.");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "LoRa P2P send queue created, size %d Bytes",
           SEND_QUEUE_SIZE * PAYLOAD_BUFFER_SIZE);

  ESP_LOGI(TAG, "Starting LoRa P2P loop...");
  xTaskCreatePinnedToCore(lora_p2p_task, "lorap2ploop", 4096, (void *)NULL, 5,
                          &lorap2pTask, 1);
  return ESP_OK;
}

void lora_p2p_task(void *param) {
  MessageBuffer_t msg;

  while (1) {
    // fetch next or wait for payload to send from queue
    // do not delete item from queue until it is transmitted
    if (xQueuePeek(LoraP2PSendQueue, &msg, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    
    // Convert the message to an integer directly
    unsigned long messageInt = 0;
    for (size_t i = 0; i < msg.MessageSize; ++i) {
      messageInt = (messageInt << 8) | msg.Message[i];
    }

    Serial.printf("Sending count: %lu\n", messageInt);

    // send packet
    LoRa.beginPacket();
    LoRa.print(messageInt);
    LoRa.endPacket();

    xQueueReceive(LoraP2PSendQueue, &msg, (TickType_t)0);
  }
}

// enqueue outgoing messages in MQTT send queue
void lora_p2p_enqueuedata(MessageBuffer_t *message) {
  if (xQueueSendToBack(LoraP2PSendQueue, (void *)message, (TickType_t)0) != pdTRUE)
    ESP_LOGW(TAG, "LORA_P2P sendqueue is full");
}

void lora_p2p_queuereset(void) { xQueueReset(LoraP2PSendQueue); }

uint32_t lora_p2p_queuewaiting(void) {
  return uxQueueMessagesWaiting(LoraP2PSendQueue);
}

#endif // HAS_LORA_P2P

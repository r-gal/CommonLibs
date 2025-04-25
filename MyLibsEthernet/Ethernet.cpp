
 #include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#include "Ethernet.hpp"
#include "Ethernet_TX.hpp"

#include "RngClass.hpp"

#include "EthernetMac.hpp"
#if USE_MDNS == 1
#include "EthernetMDNS.hpp"
#endif

ETH_DMADescTypeDef __attribute((section(".RAM_D2"))) DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef __attribute((section(".RAM_D2"))) DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

ETH_HandleTypeDef heth;

Ethernet_c* Ethernet_c::ownPtr = nullptr;

 #if ETH_STATS == 1
CommandEth_c commandEth;
#endif

#define TIMEOUT_MS 500

void vFunctionTimeoutTimerCallback( TimerHandle_t xTimer )
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if(EthernetPhy_c::taskToNotifyOnTimeout != nullptr)
  {
    xTaskNotifyFromISR( EthernetPhy_c::taskToNotifyOnTimeout, EMAC_IF_TIMEOUT_EVENT, eSetBits, &( xHigherPriorityTaskWoken ) );
  }

}

Ethernet_c::Ethernet_c(void)
{
  ownPtr = this;
  xMacInitStatus = eMACInit;
  timeoutTimer = xTimerCreate("",pdMS_TO_TICKS(TIMEOUT_MS),pdTRUE,( void * ) 0,vFunctionTimeoutTimerCallback);                 
  xTimerStart(timeoutTimer,0);

}
/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
void Ethernet_c::Init(void)
{
  #if ETH_STATS == 1
  memset(&stats,0,sizeof(EthStats_st));
  #endif
  phy.ProvideData(&heth,ethernetRx.GetRxTask());
  ethernetTx.ProvideData(&heth);
  ethernetRx.ProvideData(&heth,this);
}

void Ethernet_c::InitInternal(void)
{  
  if(InterfaceInitialise())
  {
    LinkStateChanged(1);
  }

}

void Ethernet_c::HardwareInit(void)
{
  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */


  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;

  heth.Init.MACAddr = MAC_c::GetOwnMac();
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END ETH_Init 2 */
}

void Ethernet_c::BuffersInit(void)
{

}

bool Ethernet_c::InterfaceInitialise( void )
{
  HAL_StatusTypeDef xHalEthInitStatus;

  if( xMacInitStatus == eMACInit )
  {
    /*
     * Initialize ETH Handler
     * It assumes that Ethernet GPIO and clock configuration
     * are already done in the ETH_MspInit()
     */
    heth.Instance = ETH;
    heth.Init.MACAddr = MAC_c::GetOwnMac();// ucMACAddress ;
    heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
    heth.Init.TxDesc = DMATxDscrTab;
    heth.Init.RxDesc = DMARxDscrTab;
    heth.Init.RxBuffLen = 1524;
  

    /* Make sure that all unused fields are cleared. */
    memset( &( DMATxDscrTab ), '\0', sizeof( DMATxDscrTab ) );
    memset( &( DMARxDscrTab ), '\0', sizeof( DMARxDscrTab ) );

    xHalEthInitStatus = HAL_ETH_Init( &( heth ) );

    /* Only for inspection by debugger. */
    ( void ) xHalEthInitStatus;

    #if USE_MDNS == 1

    //HAL_ETH_SetSourceMACAddrMatch( &( heth ),ETH_MAC_ADDRESS1,MDNS_c::GetMac());
    uint8_t* pMACAddr = MDNS_c::GetMac();

    /* Set MAC addr bits 32 to 47 */
    heth.Instance->MACA1HR = (((uint32_t)(pMACAddr[5]) << 8) | (uint32_t)pMACAddr[4]);
    /* Set MAC addr bits 0 to 31 */
    heth.Instance->MACA1LR = (((uint32_t)(pMACAddr[3]) << 24) | ((uint32_t)(pMACAddr[2]) << 16) |
                                     ((uint32_t)(pMACAddr[1]) << 8) | (uint32_t)pMACAddr[0]);

  /* Enable address and set source address bit */
    heth.Instance->MACA1HR |= (ETH_MACA1HR_AE );
    #endif


    if( ethernetTx.InterfaceInit() == false )
    {
      xMacInitStatus = eMACFailed;
    }

    if( ethernetRx.InterfaceInit() == false )
    {
      xMacInitStatus = eMACFailed;
    }

    

    /* Configure the MDIO Clock */
    HAL_ETH_SetMDIOClockRange( &( heth ) );

    /* Initialize the MACB and set all PHY properties */
    phy.Init();

    /* Force a negotiation with the Switch or Router and wait for LS. */
    bool result = phy.UpdateConfig( pdTRUE );
    if(result == true)
    {
      xMacInitStatus = eMACPass;
      stats.cntConfigSucc++;
    }
    else
    {
      stats.cntConfigFail++;
    }
    
  }
  
  if( xMacInitStatus != eMACPass )
  {
      /* EMAC initialisation failed, return pdFAIL. */
      return false;
  }
  else
  {   
     LINK_EVENT_et linkStatus = phy.CheckLinkStatus();
      if(( linkStatus == LINK_ON )  || (linkStatus == LINK_CHANGED_UP))
      {
          //xETH.Instance->DMAIER |= ETH_DMA_ALL_INTS;
          return true;
          //FreeRTOS_printf( ( "Link Status is high\n" ) );
          stats.cntLinkUp++;
      }
      else
      {
          /* For now pdFAIL will be returned. But prvEMACHandlerTask() is running
           * and it will keep on checking the PHY and set 'ulLinkStatusMask' when necessary. */
          return false;
      }
  }


}



void Ethernet_c::ResetTimout(void)
{
  xTimerReset(timeoutTimer,pdMS_TO_TICKS(TIMEOUT_MS));

}
void Ethernet_c::Timeout()
{
  LINK_EVENT_et linkStatus = phy.CheckLinkStatus();

  if(linkStatus == LINK_CHANGED_UP)
  {
    stats.cntLinkUp++;
    bool result = phy.UpdateConfig(pdFALSE);
    if(result == true)
    {
      stats.cntConfigSucc++;
      LinkStateChanged(1);
    }
    else
    {
      stats.cntConfigFail++;
      /* something wrong, try again after timeout */

    }
    
  }
  else if(linkStatus == LINK_CHANGED_DOWN)
  {
    stats.cntLinkDown++;
    LinkStateChanged(0);
    phy.UpdateConfig(pdFALSE);
    
  }

}

void Ethernet_c::LinkStateChanged(uint8_t newState)
{
   tcpLinkEventSig_c* sig_p = new tcpLinkEventSig_c;
   sig_p->linkState = newState;
   sig_p->Send();

}

/**
* @brief ETH MSP Initialization
* This function configures the hardware resources used in this example
* @param heth: ETH handle pointer
* @retval None
*/

#ifdef STM32F427xx
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(heth->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspInit 0 */

  /* USER CODE END ETH_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ETH_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(ETH_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

  /* USER CODE BEGIN ETH_MspInit 1 */

  /* USER CODE END ETH_MspInit 1 */
  }

}

/**
* @brief ETH MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param heth: ETH handle pointer
* @retval None
*/
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
  if(heth->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspDeInit 0 */

  /* USER CODE END ETH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ETH_CLK_DISABLE();

    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN ETH_MspDeInit 1 */

  /* USER CODE END ETH_MspDeInit 1 */
  }

}

#endif

#ifdef STM32H725xx

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(ethHandle->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspInit 0 */

  /* USER CODE END ETH_MspInit 0 */
    /* ETH clock enable */
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ETH_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ETH_RESET_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(ETH_RESET_GPIO_Port,ETH_RESET_Pin,GPIO_PIN_SET);

    HAL_NVIC_SetPriority(ETH_IRQn, 11, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

  /* USER CODE BEGIN ETH_MspInit 1 */

  /* USER CODE END ETH_MspInit 1 */
  }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* ethHandle)
{

  if(ethHandle->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspDeInit 0 */

  /* USER CODE END ETH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ETH1MAC_CLK_DISABLE();
    __HAL_RCC_ETH1TX_CLK_DISABLE();
    __HAL_RCC_ETH1RX_CLK_DISABLE();

    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN ETH_MspDeInit 1 */

  /* USER CODE END ETH_MspDeInit 1 */
  }
}
#endif

bool Ethernet_c::GetLinkState(void)
{
  return phy.GetLinkStatus();

}



#ifdef __cplusplus
 extern "C" {
#endif
void ETH_IRQHandler( void )
{
//HAL_GPIO_WritePin(TEST5_PORT,TEST5_PIN,GPIO_PIN_SET);
    HAL_ETH_IRQHandler( &heth );
  //  HAL_GPIO_WritePin(TEST5_PORT,TEST5_PIN,GPIO_PIN_RESET);
}
#ifdef __cplusplus
}
#endif


 #if ETH_STATS == 1
comResp_et Com_ethgetstat::Handle(CommandData_st* comData)
{
  char* textBuf  = new char[256];
/*
  LonGetRainStats_c* sig_p = new LonGetRainStats_c;
  sig_p->idx = 0;
  sig_p->task = xTaskGetCurrentTaskHandle();
  sig_p->Send();

  ulTaskNotifyTake(pdTRUE ,portMAX_DELAY );
*/
  Ethernet_c* eth_p = Ethernet_c::GetOwnPtr();
  EthStats_st* stat_p = &eth_p->stats;



  sprintf(textBuf," Ethernet statistics: \n");
  Print(comData->commandHandler,textBuf);

  sprintf(textBuf,"ActState:   %s\n",eth_p->GetLinkState() ? "UP" : "DOWN");
  Print(comData->commandHandler,textBuf);

  sprintf(textBuf,"Link UP:   %d \n",stat_p->cntLinkUp);
  Print(comData->commandHandler,textBuf);
  sprintf(textBuf,"Link Down: %d\n",stat_p->cntLinkDown);
  Print(comData->commandHandler,textBuf);
  sprintf(textBuf,"Config Success: %d\n",stat_p->cntConfigSucc);
  Print(comData->commandHandler,textBuf);
  sprintf(textBuf,"Config Failed : %d\n",stat_p->cntConfigFail);
  Print(comData->commandHandler,textBuf);

  delete[] textBuf;

  //delete sig_p;

  return COMRESP_OK;

}
#endif
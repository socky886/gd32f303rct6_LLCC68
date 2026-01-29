## project overview
1. the mcu is GD32F303RCT6
2. the semtech chip is LLCC68
3.  GD32F303RCT6 drive LLCC68 by spi
4. current transmit and receive work normal

## funciton overview
1. llcc68/src is the driver of llcc68, do not modify
2. sx126x-board.h and sx126x-board.c is driver of llcc68, do not modify

## edit folder or file, the permissiion
1. LLCC68/board/gd32f303rct6_board.c  can edit, can delete
2. LLCC68/board/gd32f303rct6_board.h  can edit, can delete
3. LLCC68/board/gd32f303rct6_delay.h  can edit, can delete
4. LLCC68/board/gd32f303rct6_gpio.h  can edit, can delete
5. LLCC68/board/gd32f303rct6_spi.h  can edit, can delete
6. project/main.c  can edit
7. project/main.h  can edit

## requirement
1. add config item can be configed by Micro include
   tx power
   frequency
   sf
   bandwidth
   code rate
   preamble length
   header type: explicit,implicit
   packet type: fixed len, variable packet length ; wichi is same to header type
   crc eanble or disable
   dio2 as switch or not
   dio3 as tcxo control or not
   dio1 interrupt set which include enable and mask, for example: TX_DONE, RX_DONE
   spi use hardware spi or software spi
2. put the config item micro in the main.c
3. can transmit cw
4. can transmit lora packet
5. can receive lora packet
6. please clean my old code and give me a clear structure project
7. I use the keil tool chain




## 2025、09/22
1. generate new bin to support 30DBM Radio Board
2. the carrier frequency is 471M, the PA is SGM33685C
## 2025/06/13
1. set the rx continous mode
2. can control the led by key press
3. the frequency is 916123456HZ
## 2025/05/29
1. add the pc software to burn the GD32F303RCT6
2. add the user guide to burn the bin
3. add the transmit bin and receiver bin
## 2025/03/25
1. add the user guide, the auther is wangpie
## 2025/03/06 09:40
1. the config is sf7,125k,4/5, 470000000, tx_power=8, variable packet, IQ invert is false
2. the RX interrupt mask is only RX_DONE
3. the project work normal with GD32F303
## 20241209
1. support keil V5.38
2. the compiler version is V6.19
3. work normal by test
4. add micro define to switch the print information of spi
## 20231006
1. can communication with ningbo water meter
## 20230908
1. add CAD support
2. add the 'weak' function
3. simplify the main function
## 20230722 update list
1. support CW mode to test rf performance
## 20230703 update list
1. support the SW1179
2. DIO2 as switch, DIO2=1 in TX mode, DIO2=0 in other mode
3. support the 30dbm module which integrate the PA and LNA
4. suport the keil 5 project
5. can print the rf frequency, the spread factor, the bandwidth by serial port
6. use RTC alarm interrupt to indicate the Rx timeout
## 20230608 update list
1. remove some unused folder
## 20230607 update list
1. simplicity the LLCC68 library
## 20230606 update list
1. support soft spi and hardware spi interface
## 20230605 update list
1. add the function of register_test to test the spi
## 2023/05/15 update list
1. compiler normal
2. tx packet normal
3. rx packet normal
4. special focus on spi polarity and phase
5. simplify the RadioIrqHandler
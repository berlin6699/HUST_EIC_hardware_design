## SPI DAC1
##Pmod Header JA
set_property -dict { PACKAGE_PIN K18   IOSTANDARD LVCMOS33 } [get_ports { ss1 }]; 
set_property -dict { PACKAGE_PIN H20   IOSTANDARD LVCMOS33 } [get_ports { sck1 }]; 
#set_property -dict { PACKAGE_PIN L20   IOSTANDARD LVCMOS33 } [get_ports { JA3 }]; 
set_property -dict { PACKAGE_PIN M20   IOSTANDARD LVCMOS33 } [get_ports { mosi1 }]; 
#set_property -dict { PACKAGE_PIN K17   IOSTANDARD LVCMOS33 } [get_ports { JA7 }]; 
#set_property -dict { PACKAGE_PIN J20   IOSTANDARD LVCMOS33 } [get_ports { JA8 }]; 
#set_property -dict { PACKAGE_PIN L19   IOSTANDARD LVCMOS33 } [get_ports { JA9 }]; 
#set_property -dict { PACKAGE_PIN M19   IOSTANDARD LVCMOS33 } [get_ports { JA10 }];

## AD9744 DAC1
#Pmod Header JB
set_property -dict { PACKAGE_PIN W20   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[13] }]; 
set_property -dict { PACKAGE_PIN Y19   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[11] }]; 
set_property -dict { PACKAGE_PIN Y17   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[9] }]; 
set_property -dict { PACKAGE_PIN W19   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[7] }]; 
set_property -dict { PACKAGE_PIN V20   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[12] }]; 
set_property -dict { PACKAGE_PIN Y18   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[10] }]; 
set_property -dict { PACKAGE_PIN Y16   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[8] }]; 
set_property -dict { PACKAGE_PIN W18   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[6] }];

#Pmod Header JC
set_property -dict { PACKAGE_PIN U20   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[5] }]; 
set_property -dict { PACKAGE_PIN P20   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[3] }]; 
set_property -dict { PACKAGE_PIN P18   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[1] }]; 
set_property -dict { PACKAGE_PIN P19   IOSTANDARD LVCMOS33 } [get_ports { dac1_clkp }]; 
set_property -dict { PACKAGE_PIN T20   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[4] }]; 
set_property -dict { PACKAGE_PIN N20   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[2] }]; 
set_property -dict { PACKAGE_PIN N17   IOSTANDARD LVCMOS33 } [get_ports { dac1_data[0] }]; 
set_property -dict { PACKAGE_PIN N18   IOSTANDARD LVCMOS33 } [get_ports { dac1_clkn }];

## AD9744 DAC2
#Pmod Header JD
set_property -dict { PACKAGE_PIN T14   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[13] }]; 
set_property -dict { PACKAGE_PIN U13   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[11] }]; 
set_property -dict { PACKAGE_PIN P14   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[9] }]; 
set_property -dict { PACKAGE_PIN T11   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[7] }]; 
set_property -dict { PACKAGE_PIN T15   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[12] }]; 
set_property -dict { PACKAGE_PIN V13   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[10] }]; 
set_property -dict { PACKAGE_PIN R14   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[8] }]; 
set_property -dict { PACKAGE_PIN T10   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[6] }];

#Pmod Header JE
set_property -dict { PACKAGE_PIN V15   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[5] }]; 
set_property -dict { PACKAGE_PIN T12   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[3] }]; 
set_property -dict { PACKAGE_PIN W14   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[1] }]; 
set_property -dict { PACKAGE_PIN V12   IOSTANDARD LVCMOS33 }  [get_ports { dac2_clkp }]; 
set_property -dict { PACKAGE_PIN W15   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[4] }]; 
set_property -dict { PACKAGE_PIN U12   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[2] }]; 
set_property -dict { PACKAGE_PIN Y14   IOSTANDARD LVCMOS33 }  [get_ports { dac2_data[0] }]; 
set_property -dict { PACKAGE_PIN W13   IOSTANDARD LVCMOS33 }  [get_ports { dac2_clkn }];

## SPI DAC2
##Pmod Header JF
set_property -dict { PACKAGE_PIN Y13   IOSTANDARD LVCMOS33 } [get_ports { ss2 }]; 
set_property -dict { PACKAGE_PIN W8    IOSTANDARD LVCMOS33 } [get_ports { sck2 }]; 
#set_property -dict { PACKAGE_PIN Y8    IOSTANDARD LVCMOS33 } [get_ports { JF3 }]; 
set_property -dict { PACKAGE_PIN Y6    IOSTANDARD LVCMOS33 } [get_ports { mosi2 }]; 
#set_property -dict { PACKAGE_PIN Y12   IOSTANDARD LVCMOS33 } [get_ports { JF7 }]; 
#set_property -dict { PACKAGE_PIN V8    IOSTANDARD LVCMOS33 } [get_ports { JF8 }]; 
#set_property -dict { PACKAGE_PIN Y9    IOSTANDARD LVCMOS33 } [get_ports { JF9 }]; 
#set_property -dict { PACKAGE_PIN Y7    IOSTANDARD LVCMOS33 } [get_ports { JF10 }];

## LTC2220 ADC
#Pmod Header JG
set_property -dict { PACKAGE_PIN W11   IOSTANDARD LVCMOS33 } [get_ports { adc_clkp }]; 
# set_property -dict { PACKAGE_PIN W10   IOSTANDARD LVCMOS33 } [get_ports { JG2 }]; 
set_property -dict { PACKAGE_PIN T5    IOSTANDARD LVCMOS33 } [get_ports { adc_data[0] }]; 
set_property -dict { PACKAGE_PIN V6    IOSTANDARD LVCMOS33 } [get_ports { adc_data[2] }]; 
set_property -dict { PACKAGE_PIN Y11   IOSTANDARD LVCMOS33 } [get_ports { adc_clkn }]; 
# set_property -dict { PACKAGE_PIN W9    IOSTANDARD LVCMOS33 } [get_ports { JG8 }]; 
set_property -dict { PACKAGE_PIN U5    IOSTANDARD LVCMOS33 } [get_ports { adc_data[1] }]; 
set_property -dict { PACKAGE_PIN W6    IOSTANDARD LVCMOS33 } [get_ports { adc_data[3] }];

#Pmod Header JH
set_property -dict { PACKAGE_PIN V7    IOSTANDARD LVCMOS33 } [get_ports { adc_data[4] }]; 
set_property -dict { PACKAGE_PIN U10   IOSTANDARD LVCMOS33 } [get_ports { adc_data[6] }]; 
set_property -dict { PACKAGE_PIN V10   IOSTANDARD LVCMOS33 } [get_ports { adc_data[8] }]; 
set_property -dict { PACKAGE_PIN U8    IOSTANDARD LVCMOS33 } [get_ports { adc_data[10] }]; 
set_property -dict { PACKAGE_PIN U7    IOSTANDARD LVCMOS33 } [get_ports { adc_data[5] }]; 
set_property -dict { PACKAGE_PIN T9    IOSTANDARD LVCMOS33 } [get_ports { adc_data[7] }]; 
set_property -dict { PACKAGE_PIN V11   IOSTANDARD LVCMOS33 } [get_ports { adc_data[9] }]; 
set_property -dict { PACKAGE_PIN U9    IOSTANDARD LVCMOS33 } [get_ports { adc_data[11] }];


# BASEBOARD USB UART
set_property -dict { PACKAGE_PIN P15   IOSTANDARD LVCMOS33 } [get_ports { UART0_rxd }];
set_property -dict { PACKAGE_PIN P16   IOSTANDARD LVCMOS33 } [get_ports { UART0_txd }];

# UART front side
set_property -dict { PACKAGE_PIN B19   IOSTANDARD LVCMOS33 } [get_ports { UART1_rxd }];
set_property -dict { PACKAGE_PIN A20   IOSTANDARD LVCMOS33 } [get_ports { UART1_txd }];


# Amplifier control
set_property -dict { PACKAGE_PIN L15   IOSTANDARD LVCMOS33 } [get_ports { C1K1 }];
set_property -dict { PACKAGE_PIN L14   IOSTANDARD LVCMOS33 } [get_ports { C2K2 }];
set_property -dict { PACKAGE_PIN J14   IOSTANDARD LVCMOS33 } [get_ports { C2K1 }];
set_property -dict { PACKAGE_PIN K14   IOSTANDARD LVCMOS33 } [get_ports { C1K2 }];

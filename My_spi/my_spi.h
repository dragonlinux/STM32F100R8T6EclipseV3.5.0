/*
 * my_spi.h
 *
 *  Created on: 2013-8-31
 *      Author: root
 */

#ifndef MY_SPI_H_
#define MY_SPI_H_

void GPIO_Configuration(void);
void SPI_Configuration(void);
void SPI1_master_SPI2_slave(void);
void clean(void);
void SPI2_master_SPI1_slave(void);
void SPI_RCC_Configuration(void);
void SPI_test(void);

#endif /* MY_SPI_H_ */

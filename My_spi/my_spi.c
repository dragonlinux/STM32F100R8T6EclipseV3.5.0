/*
 * my_spi.c
 *
 *  Created on: 2013-8-31
 *      Author: root
 */

#include "stm32f10x.h"
#include "stdio.h"
#include "my_spi.h"
/* 自定义同义关键字    --------------------------------------------------------*/

typedef enum
{
	FAILED = 0, PASSED = !FAILED
} TestStatus;

/* 自定义参数宏        --------------------------------------------------------*/
#define BufferSize 32
SPI_InitTypeDef SPI_InitStructure; /* 定义 SPI 初始化结构体 */
u8 SPI1_Buffer_Tx[BufferSize] = /* 定义待 SPI1 传输数据 */
{ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20 };

u8 SPI2_Buffer_Tx[BufferSize] = /* 定义待 SPI2 传输数据 */
{ 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70 };

u8 SPI1_Buffer_Rx[BufferSize]; /* 开辟内存空间待 SPI1 接收 */
u8 SPI2_Buffer_Rx[BufferSize]; /* 开辟内存空间待 SPI2 接收 */

u8 Tx_Idx = 0; /* 发送计数变量 */
u8 Rx_Idx = 0; /* 接收计数变量 */
vu8 k = 0, i = 0; /* 循环计数变量 */

/*******************************************************************************
 * 函数名  		: GPIO_Configuration
 * 函数描述    	: 设置各GPIO端口功能
 * 输入参数      : 无
 * 输出结果      : 无
 * 返回值        : 无
 *******************************************************************************/

void GPIO_Configuration(void)
{
	/* 定义 GPIO 初始化结构体 GPIO_InitStructure */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 设置 SPI1 引脚: SCK, MISO 和 MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 设置 SPI2 引脚: SCK, MISO 和 MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}
/*******************************************************************************
 * 函数名  		: SPI_Configuration
 * 函数描述    	: 设置 SPI 参数
 * 输入参数      : 无
 * 输出结果      : 无
 * 返回值        : 无
 *******************************************************************************/

void SPI_Configuration(void)
{
	/*
	 *	SPI 设置为双线双向全双工
	 *  	SPI 发送接收 8 位帧结构
	 *  	时钟悬空低
	 *  	数据捕获于第二个时钟沿
	 *	内部 NSS 信号由 SSI 位控制
	 *	波特率预分频值为 4
	 *	数据传输从 LSB 位开始
	 *	用于 CRC 值计算的多项式
	 */

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	/* 使能 SPI1 */
	SPI_Cmd(SPI1, ENABLE);
	/* 使能 SPI2 */
	SPI_Cmd(SPI2, ENABLE);
}

void SPI1_master_SPI2_slave(void)
{
	/* 设置 SPI1 为主机*/
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* 设置 SPI2 为从机*/
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	SPI_Init(SPI2, &SPI_InitStructure);

	while (Tx_Idx < BufferSize)
	{
		/* 等待 SPI1 发送缓冲空 */
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
			;
		/* SPI2 发送数据 */
		SPI_I2S_SendData(SPI2, SPI2_Buffer_Tx[Tx_Idx]);
		/* SPI1 发送数据 */
		SPI_I2S_SendData(SPI1, SPI1_Buffer_Tx[Tx_Idx++]);

		/* 等待 SPI2 接收数据完毕 */
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
			;
		/* 读出 SPI2 接收的数据 */
		SPI2_Buffer_Rx[Rx_Idx] = SPI_I2S_ReceiveData(SPI2);

		/* 等待 SPI1 接收数据完毕 */
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
			;
		/* 读出 SPI1 接收的数据 */
		SPI1_Buffer_Rx[Rx_Idx++] = SPI_I2S_ReceiveData(SPI1);
	}

	/* 打印试验结果信息 ---------------------------------------------------------------------------------------------*/

	printf("\r\nThe First transfer between the two SPI perpherals: The SPI1 Master and the SPI2 slaver. \r\n");

	printf("\r\nThe SPI1 has sended data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d ----------------------\r", *(SPI1_Buffer_Tx + k));
		for (i = 0; i < 200; i++)
			;
	}
	printf("\r\nThe SPI2 has receive data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d ++++++++++++++++++++++\r", *(SPI2_Buffer_Rx + k));
		for (i = 0; i < 200; i++)
			;
	}

	printf("\r\n \r\n");

	printf("\r\nThe SPI2 has sended data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d The SPI2 has sended data\r", *(SPI2_Buffer_Tx + k));
		for (i = 0; i < 200; i++)
			;
	}
	printf("\r\nThe SPI1 has receive data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d The SPI1 has receive data\r", *(SPI1_Buffer_Rx + k));
		for (i = 0; i < 200; i++)
			;
	}

	/* 打印试验结果信息 ---------------------------------------------------------------------------------------------*/

}
void clean(void)
{
	Tx_Idx = 0;
	Rx_Idx = 0;
	for (k = 0; k < BufferSize; k++)
	{
		*(SPI2_Buffer_Rx + k) = 0;
	}
	for (k = 0; k < BufferSize; k++)
	{
		*(SPI1_Buffer_Rx + k) = 0;
	}
}

void SPI2_master_SPI1_slave(void)
{
	/* 设置 SPI2 为主机*/
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI2, &SPI_InitStructure);
	/* 设置 SPI1 为从机*/
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	SPI_Init(SPI1, &SPI_InitStructure);

	while (Tx_Idx < BufferSize)
	{
		/* 等待 SPI2 发送缓冲空 */
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
			;
		/* SPI1 发送数据 */
		SPI_I2S_SendData(SPI1, SPI1_Buffer_Tx[Tx_Idx]);
		/* SPI2 发送数据 */
		SPI_I2S_SendData(SPI2, SPI2_Buffer_Tx[Tx_Idx++]);

		/* 等待 SPI1 接收数据完毕 */
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
			;
		/* 读出 SPI1 接收的数据 */
		SPI1_Buffer_Rx[Rx_Idx] = SPI_I2S_ReceiveData(SPI1);

		/* 等待 SPI2 接收数据完毕 */
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
			;
		/* 读出 SPI2 接收的数据 */
		SPI2_Buffer_Rx[Rx_Idx++] = SPI_I2S_ReceiveData(SPI2);
	}

	/* 打印试验结果信息 ---------------------------------------------------------------------------------------------*/

	printf("\r\n \r\nThe Second transfer between the two SPI perpherals: The SPI2 Master and the SPI1 slaver.  \r\n");

	printf("\r\nThe SPI2 has sended data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d \r", *(SPI2_Buffer_Tx + k));
		for (i = 0; i < 200; i++)
			;
	}
	printf("\r\nThe SPI1 has receive data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d \r", *(SPI1_Buffer_Rx + k));
		for (i = 0; i < 200; i++)
			;
	}

	printf("\r\n \r\n");

	printf("\r\nThe SPI1 has sended data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d \r", *(SPI1_Buffer_Tx + k));
		for (i = 0; i < 200; i++)
			;
	}
	printf("\r\nThe SPI2 has receive data below: \r\n");
	for (k = 0; k < BufferSize; k++)
	{
		printf("%0.2d \r", *(SPI2_Buffer_Rx + k));
		for (i = 0; i < 200; i++)
			;
	}
	/* 打印试验结果信息 ---------------------------------------------------------------------------------------------*/

}
void RCC_Configuration(void)
{

	/* 打开 SPI2 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	/* 打开 GPIOA，GPIOB，USART1 和 SPI1 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1 | RCC_APB2Periph_SPI1, ENABLE);
}

void SPI_test(void)
{
	/* 设置系统时钟 */
	RCC_Configuration();

	/* 设置 GPIO 端口 */
	GPIO_Configuration();

	/* 设置 SPI */
	SPI_Configuration();

	SPI1_master_SPI2_slave();
	clean();
	SPI2_master_SPI1_slave();
	while (1)
		;
}

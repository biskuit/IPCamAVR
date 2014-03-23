/*
 * Camera_LSY201.h
 *
 *  Created on: Mar 22, 2014
 *      Author: eprasetio
 */

#ifndef CAMERA_LSY201_H_
#define CAMERA_LSY201_H_

#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "USART_atmega328p.h"

/**
 * Camera
 */

/**
 * Error code.
 */
typedef enum{
	NoError = 0,
	UnexpectedReply,
	Timeout,
	SendError,
	RecvError,
	InvalidArguments
}ErrorCode ;

/**
 * Image size.
 */
typedef enum{
	ImageSize160x120, /**< 160x120. */
	ImageSize320x280, /**< 320x280. */
	ImageSize640x480 /**< 640x480. */
}ImageSize;

typedef enum{
	false =0,
	true
}bool;

typedef struct{
	int a;
	int b;
}serial;

/**
 * Initialize camera
 *
 * @return true if success
 */
bool camera_init();

/**
 * Reset module.
 *
 * @return Error code.
 */
ErrorCode reset();

/**
 * Set image size.
 *
 * @param is Image size.
 * @return Error code.
 */
ErrorCode setImageSize(ImageSize is);

/**
 * Take picture.
 *
 * @return Error code.
 */
ErrorCode takePicture();

/**
 * Read jpeg file size.
 *
 * @param fileSize File size.
 * @return Error code.
 */
ErrorCode readJpegFileSize(int *fileSize);

/**
 * Read jpeg file content.
 *
 * @param func A pointer to a call back function.
 * @return Error code.
 */
ErrorCode readJpegFileContent(
		void (*func)(int done, int total, uint8_t *buf, size_t siz));

/**
 * Stop taking pictures.
 *
 * @return Error code.
 */
ErrorCode stopTakingPictures();


/**
 * Wait init end codes.
 *
 * @return Error code.
 */
ErrorCode waitInitEnd();

/**
 * Send bytes to camera module.
 *
 * @param buf Pointer to the data buffer.
 * @param len Length of the data buffer.
 *
 * @return True if the data sended.
 */
bool sendBytes(uint8_t *buf, size_t len, int timeout_us);

/**
 * Receive bytes from camera module.
 *
 * @param buf Pointer to the data buffer.
 * @param len Length of the data buffer.
 *
 * @return True if the data received.
 */
bool recvBytes(uint8_t *buf, size_t len, int timeout_us);

/**
 * Wait received.
 *
 * @return True if the data received.
 */
bool waitRecv();

/**
 * Wait idle state.
 *
 * @return True if it succeed.
 */
bool waitIdle();


#endif /* CAMERA_LSY201_H_ */

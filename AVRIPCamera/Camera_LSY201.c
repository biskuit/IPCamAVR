/*
 * Camera_LSY201.c
 *
 *  Created on: Mar 22, 2014
 *      Author: eprasetio
 */

#include "Camera_LSY201.h"

bool camera_init() {
	usart_init(9600); //initialize usart
	return true;
}

/**
 * Reset module.
 *
 * @return Error code.
 */
ErrorCode reset() {
	uint8_t send[4] = { 0x56, 0x00, 0x26, 0x00 };
	uint8_t recv[4];

	waitIdle();
	if (!sendBytes(send, sizeof(send), 200 * 1000)) {
		return SendError;
	}
	if (!recvBytes(recv, sizeof(recv), 200 * 1000)) {
		return RecvError;
	}
	if ((recv[0] == 0x76) && (recv[1] == 0x00) && (recv[2] == 0x26)
			&& (recv[3] == 0x00)) {
		ErrorCode r = waitInitEnd();
		if (r != NoError) {
			return r;
		}
		_delay_ms(4000);
		return NoError;
	} else {
		return UnexpectedReply;
	}
}

/**
 * Set image size.
 *
 * @param is Image size.
 * @return Error code.
 */
ErrorCode setImageSize(ImageSize is) {
	uint8_t send[9] = { 0x56, 0x00, 0x31, 0x05, 0x04, 0x01, 0x00, 0x19, 0x00 // 0x11:320x240, 0x00:640x480, 0x22:160x120
			};
	uint8_t recv[5];
	switch (is) {
	case ImageSize160x120:
		send[8] = 0x22;
		break;
	case ImageSize320x280:
		send[8] = 0x11;
		break;
	case ImageSize640x480:
		send[8] = 0x00;
		break;
	default:
		return InvalidArguments;
	}
	if (!sendBytes(send, sizeof(send), 200 * 1000)) {
		return SendError;
	}
	if (!recvBytes(recv, sizeof(recv), 200 * 1000)) {
		return RecvError;
	}
	if ((recv[0] == 0x76) && (recv[1] == 0x00) && (recv[2] == 0x31)
			&& (recv[3] == 0x00) && (recv[4] == 0x00)) {
		_delay_ms(1000);
		return reset();
	} else {
		return UnexpectedReply;
	}
}

/**
 * Take picture.
 *
 * @return Error code.
 */
ErrorCode takePicture() {
	uint8_t send[5] = { 0x56, 0x00, 0x36, 0x01, 0x00 };
	uint8_t recv[5];

	if (!sendBytes(send, sizeof(send), 200 * 1000)) {
		return SendError;
	}
	if (!recvBytes(recv, sizeof(recv), 200 * 1000)) {
		return RecvError;
	}

	if ((recv[0] == 0x76) && (recv[1] == 0x00) && (recv[2] == 0x36)
			&& (recv[3] == 0x00) && (recv[4] == 0x00)) {
		/*
		 * I think the camera need a time for operating.
		 * But there is no any comments on the documents.
		 */
		_delay_ms(100);
		return NoError;
	} else {
		return UnexpectedReply;
	}
}

/**
 * Read jpeg file size.
 *
 * @param fileSize File size.
 * @return Error code.
 */
ErrorCode readJpegFileSize(int *fileSize) {
	uint8_t send[5] = { 0x56, 0x00, 0x34, 0x01, 0x00 };
	uint8_t recv[9];

	if (!sendBytes(send, sizeof(send), 200 * 1000)) {
		return SendError;
	}
	if (!recvBytes(recv, sizeof(recv), 200 * 1000)) {
		return RecvError;
	}

	if ((recv[0] == 0x76) && (recv[1] == 0x00) && (recv[2] == 0x34)
			&& (recv[3] == 0x00) && (recv[4] == 0x04) && (recv[5] == 0x00)
			&& (recv[6] == 0x00)) {
		*fileSize = ((recv[7] & 0x00ff) << 8) | ((recv[8] & 0x00ff) << 0);
		return NoError;
	} else {
		return UnexpectedReply;
	}
}

/**
 * Read jpeg file content.
 *
 * @param func A pointer to a call back function.
 * @return Error code.
 */
ErrorCode readJpegFileContent(
		void (*func)(int done, int total, uint8_t *buf, size_t siz)) {
	uint8_t send[16] = { 0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00, 0x00, // MH
			0x00, // ML
			0x00, 0x00, 0x00, // KH
			0x00, // KL
			0x00, // XX
			0x00  // XX
			};
	uint8_t body[32];
	uint16_t m = 0; // Staring address.
	uint16_t k = sizeof(body); // Packet size.
	uint16_t x = 10;    // Interval time. XX XX * 0.01m[sec]
	bool end = false;

	/*
	 * Get the data size.
	 */
	int siz_done = 0;
	int siz_total = 0;
	ErrorCode r = readJpegFileSize(&siz_total);
	if (r != NoError) {
		return r;
	}

	do {
		send[8] = (m >> 8) & 0xff;
		send[9] = (m >> 0) & 0xff;
		send[12] = (k >> 8) & 0xff;
		send[13] = (k >> 0) & 0xff;
		send[14] = (x >> 8) & 0xff;
		send[15] = (x >> 0) & 0xff;
		/*
		 * Send a command.
		 */
		if (!sendBytes(send, sizeof(send), 200 * 1000)) {
			return SendError;
		}
		/*
		 * Read the header of the response.
		 */
		uint8_t header[5];
		if (!recvBytes(header, sizeof(header), 2 * 1000 * 1000)) {
			return RecvError;
		}
		/*
		 * Check the response and fetch an image data.
		 */
		if ((header[0] == 0x76) && (header[1] == 0x00) && (header[2] == 0x32)
				&& (header[3] == 0x00) && (header[4] == 0x00)) {
			if (!recvBytes(body, sizeof(body), 2 * 1000 * 1000)) {
				return RecvError;
			}
			siz_done += sizeof(body);
			if (func != NULL ) {
				if (siz_done > siz_total) {
					siz_done = siz_total;
				}
				func(siz_done, siz_total, body, sizeof(body));
			}
			for (int i = 1; i < sizeof(body); i++) {
				if ((body[i - 1] == 0xFF) && (body[i - 0] == 0xD9)) {
					end = true;
				}
			}
		} else {
			return UnexpectedReply;
		}
		/*
		 * Read the footer of the response.
		 */
		uint8_t footer[5];
		if (!recvBytes(footer, sizeof(footer), 2 * 1000 * 1000)) {
			return RecvError;
		}

		m += sizeof(body);
	} while (!end);
	return NoError;
}

/**
 * Stop taking pictures.
 *
 * @return Error code.
 */
ErrorCode stopTakingPictures() {
	uint8_t send[5] = { 0x56, 0x00, 0x36, 0x01, 0x03 };
	uint8_t recv[5];

	if (!sendBytes(send, sizeof(send), 200 * 1000)) {
		return SendError;
	}
	if (!recvBytes(recv, sizeof(recv), 200 * 1000)) {
		return RecvError;
	}

	if ((recv[0] == 0x76) && (recv[1] == 0x00) && (recv[2] == 0x36)
			&& (recv[3] == 0x00) && (recv[4] == 0x00)) {
		/*
		 * I think the camera need a time for operating.
		 * But there is no any comments on the documents.
		 */
		_delay_ms(100);
		return NoError;
	} else {
		return UnexpectedReply;
	}
}

/**
 * Wait init end codes.
 *
 * @return True if the data sended.
 */
ErrorCode waitInitEnd() {
	static const char *PWR_ON_MSG = "Init end\x0d\x0a";
	for (int i = 0; i < strlen(PWR_ON_MSG); i++) {
		static const int MAXCNT = 128;
		int cnt = 0;
		uint8_t c = 0x00;
		do {
			if (!recvBytes(&c, sizeof(c), 500 * 1000)) {
				return Timeout;
			}

			/*
			 * What is the version of the camera.
			 * You can check the version with this code.
			 *
			 * VC0703 1.00
			 * 3o ctrl in
			 * Init end
			 */
#if 0
			printf("%c", c);
#endif

			cnt++;
			if (MAXCNT < cnt) {
				return UnexpectedReply;
			}
		} while (c != PWR_ON_MSG[i]);
	}
	return NoError;
}

/**
 * Send bytes to camera module.
 *
 * @param buf Pointer to the data buffer.
 * @param len Length of the data buffer.
 *
 * @return True if the data sended.
 */
#include "Camera_LSY201.h"

bool sendBytes(uint8_t *buf, size_t len, int timeout_us) {
	for (uint32_t i = 0; i < (uint32_t) len; i++) {
		int cnt = 0;
		while (!usart_isWriteable()) {
			_delay_us(1);
			cnt++;
			if (timeout_us < cnt) {
				return false;
			}
		}
		usart_putc(buf[i]);
	}
	return true;
}

/**
 * Receive bytes from camera module.
 *
 * @param buf Pointer to the data buffer.
 * @param len Length of the data buffer.
 *
 * @return True if the data received.
 */
bool recvBytes(uint8_t *buf, size_t len, int timeout_us) {
	for (uint32_t i = 0; i < (uint32_t) len; i++) {
		int cnt = 0;
		while (!usart_isReadable()) {
			_delay_us(1);
			cnt++;
			if (timeout_us < cnt) {
				return false;
			}
		}
		buf[i] = usart_getc();
	}
	return true;
}

/**
 * Wait received.
 *
 * @return True if the data received.
 */
bool waitRecv() {
	while (!usart_isReadable()) {
	}
	return true;
}

/**
 * Wait idle state.
 */
bool waitIdle() {
	while (usart_isReadable()) {
		usart_getc();
	}
	return true;
}


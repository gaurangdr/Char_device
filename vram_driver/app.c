/**
 * @file   app.c
 * @author gaurang
 * @date   12 Nov 2017
 * @brief  A Linux user space program that communicates with the first_driver.c
 *	   LKM. It passes a string to the LKM and reads the response from the
 *	   LKM. For this example to work the device must be called /dev/My_First_Driver.
 */

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(void)
{
	int ret, fd;
	char stringToSend[BUFFER_LENGTH];
	printf("Starting device test code example\n");
	fd = open("/dev/vram", O_RDWR); // Open the device with read/write access
	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}
	printf("Type in a short string to send to the kernel module:\n");
//	scanf("%s", stringToSend);
	fgets(stringToSend, BUFFER_LENGTH, stdin);
	ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}

	printf("Press ENTER to read back from the device...\n");
	getchar();

	printf("Reading from the device...\n");
	ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
	if (ret < 0) {
		perror("Failed to read the message from the device.");
		return errno;
	}
	printf("The received message is: %s\n", receive);
	printf("End of the program\n");
	return 0;
}

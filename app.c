/* system headers */
#include <conio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/* local headers */
#include "./text_logger_lib/text_logger.h"

/*
 * Defines
 */

#define MAX_STR_BYTE_SIZE     (1024)
#define MAX_FILE_SIZE         (2048)

/*
 * Static
 */

static char spFileName[MAX_STR_BYTE_SIZE] = "./TestLog.txt";
static char spFileLimitErrMsg[MAX_STR_BYTE_SIZE] = "\n[ERR LIMIT]";

/*
 * Codes
 */

/**
 * main creates and manages logger context(s).
 * 
 * @post All logger contexts created must be destroyed at the end of the main thread.
 * @return 0 if all operations are successful.
 * @return 1 if an error occurs when creating a logger context.
 * @return -1 if an error occurs during file-related operations.
 */
int main()
{
   bool sAppRunning = true;
   TextLoggerStatusType status;
   LoggerContextType* pLogContext1 = TextLogger_Create(spFileName, spFileLimitErrMsg, LOG_LEVEL_VERBOSE, MAX_STR_BYTE_SIZE, MAX_FILE_SIZE);
   if (NULL == pLogContext1) {
      printf("Log Context creation failed.\n");
      return 1;
   } else {
      printf("pLogContext1 created!\n");
   }
   
   printf("main - running...\n");
   while (sAppRunning) {
      Sleep(200);
      // detect keypress
      if (_kbhit()) {
         char keypressed = _getch();
         printf("\nkeypressed: %c\n", keypressed);

         if (keypressed == 'q') {
            printf("main - to stop...\n");
            sAppRunning = false;
         }
         else if (keypressed == '1') {
            status = TextLogger_LogError(pLogContext1, "Error statement");
         }
         else if (keypressed == '2') {
            status = TextLogger_LogWarn(pLogContext1, "Warn statement");
         }
         else if (keypressed == '3') {
            status = TextLogger_LogInfo(pLogContext1, "Info statement");
         }
         else if (keypressed == '4') {
            status = TextLogger_LogDebug(pLogContext1, "Debug statement");
         }
         else if (keypressed == '5') {
            status = TextLogger_LogVerbose(pLogContext1, "Verbose statement");
         }
         else if (keypressed == 'f') {
            status = TextLogger_FlushTextToFileStream(pLogContext1);
         }
         else if (keypressed == 'o') {
            status = TextLogger_PrintCurrFileSize(pLogContext1); // for debugging
         }
         else {
            // ignored
         }

         if (TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE == status) {
            printf("Insufficient file space.\n");
         } else if (TEXTLOGGER_ERR_FILE_ERROR == status) {
            printf("File error!\n");
            return -1;
         }
      }
   }

   printf("main - stopped\n");

   TextLogger_Destroy(pLogContext1);
   pLogContext1 = NULL;

   printf("Main finish\n");
   return 0;
}
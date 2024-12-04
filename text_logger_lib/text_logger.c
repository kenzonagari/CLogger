/* system headers */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* local headers */
#include "text_logger.h"

/*
 * Defines
 */

#define DEBUG_MODULE	(0)
#if DEBUG_MODULE
#define DBGPRINTF(x)	printf(x)
#else
#define DBGPRINTF(x)
#endif

#define MAX_STR_SIZE             (128)
#define LOG_EXTRA_STR_LENGTH     (6) // LOG_EXTRA_STR_LENGTH accounts for adding "[E]: \n" with the log message

/*
 * Structures
 */

/**
 * @brief This is the structure type of a logger context.
 *
 * User needs to provide strings containing
 * 1) Full path to file and
 * 2) Error message for if max file size has been reached.
 *
 * User needs to also specify
 * 1) minimum log level to filter log messages,
 * 2) max buffer size to store messages before flushing, and
 * 3) max file size allowed to contain all log messages.
 */
struct LoggerContext{
   FILE* pLogFile;
   char* pTextBuffer;
   char* pFilePath;
   char* pErrMsg;
   int logLevel; // refer to LogLevelType for list of log levels
   int maxBufferByteSize;
   int maxFileSize;
   int currBytePos; // starts 0
   int totalBytesStored; // starts at 0
   bool fileLimitIsReached; // starts at false
};

/*
 * Code
 */

LoggerContextType* TextLogger_Create(char* pFilePath, char* pErrMsg, int logLevel, int maxBufferByteSize, int maxFileSize)
{
   if (NULL == pFilePath || NULL == pErrMsg) {
      return NULL;
   }

   // initialize context
   LoggerContextType* pLoggerContext = (LoggerContextType*) malloc(sizeof(LoggerContextType));
   if (NULL == pLoggerContext) {
      return NULL;
   }

   // initialize parameters
   pLoggerContext->maxFileSize = maxFileSize - (strlen(pErrMsg) + 1); // reserve fixed amount of space in the file for error message
   if (0 >= pLoggerContext->maxFileSize) {
      free(pLoggerContext);
      return NULL; // maxFileSize is too small
   }
   pLoggerContext->logLevel = logLevel;
   pLoggerContext->maxBufferByteSize = maxBufferByteSize;
   pLoggerContext->currBytePos = 0;
   pLoggerContext->totalBytesStored = 0;
   pLoggerContext->fileLimitIsReached = false;

   // dynamically allocate & init file path
   pLoggerContext->pFilePath = (char*) malloc(strlen(pFilePath) + 1); // +1 for the null terminator
   if (NULL == pLoggerContext->pFilePath) {
      free(pLoggerContext);
      pLoggerContext = NULL;
      return NULL;
   }
   strcpy(pLoggerContext->pFilePath, pFilePath);

   // dynamically allocate text buffer
   pLoggerContext->pTextBuffer = (char*) malloc(sizeof(char) * maxBufferByteSize);
   if (NULL == pLoggerContext->pTextBuffer) {
      free(pLoggerContext->pFilePath);
      free(pLoggerContext);
      pLoggerContext = NULL;
      return NULL;
   }

   // dynamically allocate & init error msg
   pLoggerContext->pErrMsg = (char*) malloc(sizeof(char) * (strlen(pErrMsg) + 1));
   if (NULL == pLoggerContext->pErrMsg) {
      free(pLoggerContext->pFilePath);
      free(pLoggerContext->pTextBuffer);
      free(pLoggerContext);
      pLoggerContext = NULL;
      return NULL;
   }
   strcpy(pLoggerContext->pErrMsg, pErrMsg);

   return pLoggerContext;
}

TextLoggerStatusType TextLogger_Destroy(LoggerContextType* pLoggerContext)
{
   if (NULL == pLoggerContext) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // Flush any remaining text to file
   TextLoggerStatusType status = TextLogger_FlushTextToFileStream(pLoggerContext);

   // free allocated memory for pLoggerContext->pErrMsg
   if (NULL != pLoggerContext->pErrMsg) {
      free(pLoggerContext->pErrMsg);
      pLoggerContext->pErrMsg = NULL;
   }

   // free allocated memory for pLoggerContext->pFilePath
   if (NULL != pLoggerContext->pFilePath) {
      free(pLoggerContext->pFilePath);
      pLoggerContext->pFilePath = NULL;
   }

   // free allocated memory for pLoggerContext->pTextBuffer
   if (NULL != pLoggerContext->pTextBuffer) {
      free(pLoggerContext->pTextBuffer);
      pLoggerContext->pTextBuffer = NULL;
   }

   // free allocated memory for pLoggerContext
   if (NULL != pLoggerContext) {
      free(pLoggerContext);
      pLoggerContext = NULL;
   }

   return status;
}

/**
 * @internal
 *
 * Checks if buffer needs to be flushed to file.
 *
 * @param [in,out] pLoggerContext Pointer to logger context
 * @param [in] lengthOfTextToAdd Length of text to compare with the available space in file or current buffer
 * @return true if buffer needs to be flushed.
 */
static bool TextLogger_FlushBufferIsNeeded(LoggerContextType* pLoggerContext, int lengthOfTextToAdd)
{
   // check if maxFileSize is about to be reached or if maxBufferByteSize is about to be reached
   if ((pLoggerContext->maxFileSize - pLoggerContext->totalBytesStored) <= lengthOfTextToAdd ||
      (pLoggerContext->maxBufferByteSize - pLoggerContext->currBytePos) <= lengthOfTextToAdd) {
      return true;
   }
   return false;
}

TextLoggerStatusType TextLogger_LogTimeStamp(LoggerContextType* pLoggerContext)
{
   if (NULL == pLoggerContext) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // get current date and time as a string
   time_t rawtime;
   struct tm *timeinfo;
   char pTimeBuffer[MAX_STR_SIZE];
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   snprintf(pTimeBuffer, sizeof(pTimeBuffer), "[%04d-%02d-%02d | %02d:%02d:%02d] ",
            (timeinfo->tm_year) + 1900, (timeinfo->tm_mon) + 1, timeinfo->tm_mday,   // tm_year is years since 1900, tm_mon values are from 0-11
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

   // check if pTextBuffer must be flushed
   if (TextLogger_FlushBufferIsNeeded(pLoggerContext, strlen(pTimeBuffer))) {
      TextLoggerStatusType status = TextLogger_FlushTextToFileStream(pLoggerContext);
      if (TEXTLOGGER_SUCCESS != status) {
         return status;
      }
   }

   // write to buffer
   size_t bytesWritten = snprintf(  (pLoggerContext->pTextBuffer + pLoggerContext->currBytePos),
                                    (pLoggerContext->maxBufferByteSize - pLoggerContext->currBytePos),
                                    "%s", pTimeBuffer
                                 );
   // move currBytePos forward
   pLoggerContext->currBytePos += bytesWritten;
   pLoggerContext->totalBytesStored += bytesWritten;

   return TEXTLOGGER_SUCCESS;
}

/**
 * @internal
 *
 * Writes log message to buffer.
 *
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @param [in] pLogText String containing log message.
 * @param [in] logLength Length of log message.
 * @param [in] logLevel Level of log message.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
static TextLoggerStatusType TextLogger_WriteToBuffer(LoggerContextType* pLoggerContext, const char* pLogText, int logLength, LogLevelType logLevel)
{
   if (NULL == pLoggerContext || NULL == pLogText) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // initialize log message tag
   char pLogMsgTag[MAX_STR_SIZE];
   if (LOG_LEVEL_ERROR == logLevel) {
      strcpy(pLogMsgTag, "[E]: ");
   } else if (LOG_LEVEL_WARN == logLevel) {
      strcpy(pLogMsgTag, "[W]: ");
   } else if (LOG_LEVEL_INFO == logLevel) {
      strcpy(pLogMsgTag, "[I]: ");
   } else if (LOG_LEVEL_DEBUG == logLevel) {
      strcpy(pLogMsgTag, "[D]: ");
   } else if (LOG_LEVEL_VERBOSE == logLevel) {
      strcpy(pLogMsgTag, "[V]: ");
   }

   //write timestamp to buffer
   TextLoggerStatusType status = TextLogger_LogTimeStamp(pLoggerContext);
   if (TEXTLOGGER_SUCCESS != status) {
      return status;
   }

   // check if pTextBuffer must be flushed
   if (TextLogger_FlushBufferIsNeeded(pLoggerContext, logLength)) {
      status = TextLogger_FlushTextToFileStream(pLoggerContext);
      if (TEXTLOGGER_SUCCESS != status) {
         return status;
      }
   }

   // write to buffer
   size_t bytesWritten = snprintf(  (pLoggerContext->pTextBuffer + pLoggerContext->currBytePos),  // pointer to current position in buffer
                                    (pLoggerContext->maxBufferByteSize - pLoggerContext->currBytePos),  // maximum space available to write
                                    "%s%s\n", pLogMsgTag, pLogText
                                 );
   // increment currBytePos and totalBytesStored
   pLoggerContext->currBytePos += bytesWritten;
   pLoggerContext->totalBytesStored += bytesWritten;

   return TEXTLOGGER_SUCCESS;
}

TextLoggerStatusType TextLogger_LogError(LoggerContextType* pLoggerContext, const char* pLogText)
{
   if (NULL == pLoggerContext || NULL == pLogText) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // write to buffer
   TextLoggerStatusType status = TEXTLOGGER_SUCCESS;
   if (LOG_LEVEL_ERROR <= pLoggerContext->logLevel) {
      size_t logLength = strlen(pLogText) + LOG_EXTRA_STR_LENGTH; // LOG_EXTRA_STR_LENGTH corresponds to "[E]: \n"
      status = TextLogger_WriteToBuffer(pLoggerContext, pLogText, logLength, LOG_LEVEL_ERROR);
   }

   return status;
}

TextLoggerStatusType TextLogger_LogWarn(LoggerContextType* pLoggerContext, const char* pLogText)
{
   if (NULL == pLoggerContext || NULL == pLogText) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // write to buffer
   TextLoggerStatusType status = TEXTLOGGER_SUCCESS;
   if (LOG_LEVEL_WARN <= pLoggerContext->logLevel) {
      size_t logLength = strlen(pLogText) + LOG_EXTRA_STR_LENGTH; // LOG_EXTRA_STR_LENGTH corresponds to "[W]: \n"
      status = TextLogger_WriteToBuffer(pLoggerContext, pLogText, logLength, LOG_LEVEL_WARN);
   }

   return status;
}

TextLoggerStatusType TextLogger_LogInfo(LoggerContextType* pLoggerContext, const char* pLogText)
{
   if (NULL == pLoggerContext || NULL == pLogText) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // write to buffer
   TextLoggerStatusType status = TEXTLOGGER_SUCCESS;
   if (LOG_LEVEL_INFO <= pLoggerContext->logLevel) {
      size_t logLength = strlen(pLogText) + LOG_EXTRA_STR_LENGTH; // LOG_EXTRA_STR_LENGTH corresponds to "[I]: \n"
      status = TextLogger_WriteToBuffer(pLoggerContext, pLogText, logLength, LOG_LEVEL_INFO);
   }

   return status;
}

TextLoggerStatusType TextLogger_LogDebug(LoggerContextType* pLoggerContext, const char* pLogText)
{
   if (NULL == pLoggerContext || NULL == pLogText) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // write to buffer
   TextLoggerStatusType status = TEXTLOGGER_SUCCESS;
   if (LOG_LEVEL_DEBUG <= pLoggerContext->logLevel) {
      size_t logLength = strlen(pLogText) + LOG_EXTRA_STR_LENGTH; // LOG_EXTRA_STR_LENGTH corresponds to "[D]: \n"
      status = TextLogger_WriteToBuffer(pLoggerContext, pLogText, logLength, LOG_LEVEL_DEBUG);
   }

   return status;
}

TextLoggerStatusType TextLogger_LogVerbose(LoggerContextType* pLoggerContext, const char* pLogText)
{
   if (NULL == pLoggerContext || NULL == pLogText) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // write to buffer
   TextLoggerStatusType status = TEXTLOGGER_SUCCESS;
   if (LOG_LEVEL_VERBOSE <= pLoggerContext->logLevel) {
      size_t logLength = strlen(pLogText) + LOG_EXTRA_STR_LENGTH; // LOG_EXTRA_STR_LENGTH corresponds to "[V]: \n"
      status = TextLogger_WriteToBuffer(pLoggerContext, pLogText, logLength, LOG_LEVEL_VERBOSE);
   }

   return status;
}

/**
 * @internal
 *
 * Flushes error message to file.
 *
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_FILE_ERROR if error occurs when opening or writing to file.
 */
static TextLoggerStatusType TextLogger_FlushErrMsgToFileStream(LoggerContextType* pLoggerContext)
{
   // Open file in append mode - binary
   pLoggerContext->pLogFile = fopen(pLoggerContext->pFilePath, "ab");
   if (NULL == pLoggerContext->pLogFile) {
      // Failed to open the file
      return TEXTLOGGER_ERR_FILE_ERROR;
   }

   // Write buffer to the file
   size_t bytesWritten = fwrite(pLoggerContext->pErrMsg, sizeof(char), strlen(pLoggerContext->pErrMsg), pLoggerContext->pLogFile);
   if (bytesWritten != strlen(pLoggerContext->pErrMsg)) {
      // Failed to write all data to the file
      fclose(pLoggerContext->pLogFile);
      return TEXTLOGGER_ERR_FILE_ERROR;
   }

   fclose(pLoggerContext->pLogFile);
   return TEXTLOGGER_SUCCESS;
}

TextLoggerStatusType TextLogger_FlushTextToFileStream(LoggerContextType* pLoggerContext)
{
   if (NULL == pLoggerContext) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // check if max file size has been reached
   if (pLoggerContext->maxFileSize <= pLoggerContext->totalBytesStored) {
      if(false == pLoggerContext->fileLimitIsReached) {
         // write error msg on file (only once)
         pLoggerContext->fileLimitIsReached = true;
         TextLoggerStatusType status = TextLogger_FlushErrMsgToFileStream(pLoggerContext);
         if (TEXTLOGGER_ERR_FILE_ERROR == status) {
            return TEXTLOGGER_ERR_FILE_ERROR;
         }
      }
      return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE;
   }

   // check if buffer is currently empty
   if (0 == pLoggerContext->currBytePos) {
      return TEXTLOGGER_SUCCESS; // Nothing to flush, return success
   }

   // Open file in append mode - binary
   pLoggerContext->pLogFile = fopen(pLoggerContext->pFilePath, "ab");
   if (NULL == pLoggerContext->pLogFile) {
      // Failed to open the file
      return TEXTLOGGER_ERR_FILE_ERROR;
   }

   // Get current file size
   fseek(pLoggerContext->pLogFile, 0L, SEEK_END);
   long int currFileSize = ftell(pLoggerContext->pLogFile);

   // account for scenario where buffer flush might overshoot maxFileSize
   TextLoggerStatusType status;
   if (pLoggerContext->currBytePos + currFileSize > pLoggerContext->maxFileSize) {
      // overshot maxFileSize
      if(false == pLoggerContext->fileLimitIsReached) {
         pLoggerContext->fileLimitIsReached = true;
      }
      status = TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE;
   } else {
      // Write buffer to the file
      size_t bytesWritten = fwrite(pLoggerContext->pTextBuffer, sizeof(char), pLoggerContext->currBytePos, pLoggerContext->pLogFile);
      if (bytesWritten != pLoggerContext->currBytePos) {
         // Failed to write all data to the file
         fclose(pLoggerContext->pLogFile);
         return TEXTLOGGER_ERR_FILE_ERROR;
      }

      status = TEXTLOGGER_SUCCESS;
   }

   // currFileSize = ftell(pLoggerContext->pLogFile); // => for debug
   // printf("curr byte pos: %d, total byte stored: %dB, curr file size: %ldB\n", // => for debug
   //         pLoggerContext->currBytePos, pLoggerContext->totalBytesStored, currFileSize);

   // close file
   fclose(pLoggerContext->pLogFile);

   // Reset buffer position
   pLoggerContext->currBytePos = 0;

   return status;
}

TextLoggerStatusType TextLogger_PrintCurrFileSize(LoggerContextType* pLoggerContext)
{
   if (NULL == pLoggerContext) {
      return TEXTLOGGER_ERR_INVALID_INPUT;
   }

   // Open file in append mode - binary
   pLoggerContext->pLogFile = fopen(pLoggerContext->pFilePath, "ab");
   if (NULL == pLoggerContext->pLogFile) {
      // Failed to open the file
      return TEXTLOGGER_ERR_FILE_ERROR;
   }
   // Get current file size
   fseek(pLoggerContext->pLogFile, 0L, SEEK_END);
   long int currFileSizeAb = ftell(pLoggerContext->pLogFile);
   // close file
   fclose(pLoggerContext->pLogFile);

   // Open file in append mode - text
   pLoggerContext->pLogFile = fopen(pLoggerContext->pFilePath, "a");
   if (NULL == pLoggerContext->pLogFile) {
      // Failed to open the file
      return TEXTLOGGER_ERR_FILE_ERROR;
   }
   // Get current file size
   fseek(pLoggerContext->pLogFile, 0L, SEEK_END);
   long int currFileSizeA = ftell(pLoggerContext->pLogFile);
   // close file
   fclose(pLoggerContext->pLogFile);

   printf("Current file size - binary: %ld\n", currFileSizeAb);
   printf("Current file size - text: %ld\n", currFileSizeA);
   printf("Total bytes stored: %d\n", pLoggerContext->totalBytesStored);

   return TEXTLOGGER_SUCCESS;
}



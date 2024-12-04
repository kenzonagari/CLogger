/**
 * @addtogroup TextLogger
 * @{
 */

/**
 * @brief This module is a text logging library used for debugging purposes.
 */

#ifndef _TEXT_LOGGER_H_
#define _TEXT_LOGGER_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief This is the enum type for
 * various return status inside text_logger.c.
 */
typedef enum {
   TEXTLOGGER_SUCCESS = 0,
   TEXTLOGGER_ERR_FILE_ERROR,
   TEXTLOGGER_ERR_INVALID_INPUT,
   TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE
} TextLoggerStatusType;

/**
 * @brief This is the enum type for
 * log levels in an order from most to least
 * important, following Android convention.
 * docs: https://source.android.com/docs/core/tests/debug/understanding-logging
 */
typedef enum {
   LOG_LEVEL_ERROR = 1,
   LOG_LEVEL_WARN,
   LOG_LEVEL_INFO,
   LOG_LEVEL_DEBUG,
   LOG_LEVEL_VERBOSE
} LogLevelType;

typedef struct LoggerContext LoggerContextType;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Initializes a logger context.
 * @post this function needs to be called before calling any other function in this module
 * 
 * @param [in] pFilePath String containing full file path.
 * @param [in] pErrMsg String containing error message when file limit is reached.
 * @param [in] logLevel To filter log messages based on level of importance.
 * @param [in] maxBufferByteSize Max size of buffer in bytes.
 * @param [in] maxFileSize Max size of file that contains all combined buffer + error message. 
 * @return pointer to logger context type.
 */
LoggerContextType* TextLogger_Create(char* pFilePath, char* pErrMsg, int logLevel, int maxBufferByteSize, int maxFileSize);

/**
 * Destroys a logger context.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_Destroy(LoggerContextType* pLoggerContext);

/**
 * Writes current date and time to buffer.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_LogTimeStamp(LoggerContextType* pLoggerContext);

/**
 * Writes Error level log to buffer.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @param [in] pText String containing log message.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_LogError(LoggerContextType* pLoggerContext, const char* pText); // log level 1

/**
 * Writes Warn level log to buffer.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @param [in] pText String containing log message.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_LogWarn(LoggerContextType* pLoggerContext, const char* pText); // log level 2

/**
 * Writes Info level log to buffer.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @param [in] pText String containing log message.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_LogInfo(LoggerContextType* pLoggerContext, const char* pText); // log level 3

/**
 * Writes Debug level log to buffer.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @param [in] pText String containing log message.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_LogDebug(LoggerContextType* pLoggerContext, const char* pText); // log level 4

/**
 * Writes Verbose level log to buffer.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @param [in] pText String containing log message.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_LogVerbose(LoggerContextType* pLoggerContext, const char* pText); // log level 5

/**
 * Flushes buffer to file stream.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_INSUFFICIENT_FILE_SPACE if max file size has been reached.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_FlushTextToFileStream(LoggerContextType* pLoggerContext);

/**
 * Prints current file size.
 * 
 * @param [in,out] pLoggerContext Pointer to logger context.
 * @return TEXTLOGGER_SUCCESS if operation is successful.
 * @return TEXTLOGGER_ERR_INVALID_INPUT if pointer input(s) is NULL.
 * @return TEXTLOGGER_ERR_FILE_ERROR if an error occurs during file-related operations.
 */
TextLoggerStatusType TextLogger_PrintCurrFileSize(LoggerContextType* pLoggerContext);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _TEXT_LOGGER_H_

/**
 * @}
 */

/*
 * utils.h
 *
 * General-purpose timing and value-stability utilities.
 *
 *  Created on: May 5, 2024
 *      Author: Milan
 */

#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <stdint.h>


/**
 * @brief Calculate a CRC-8 checksum (Sensirion polynomial) over a byte buffer.
 *        Used to validate sensor I2C response packets as well as the CRC field
 *        of systemParams_t.
 * @param data Pointer to the data buffer.
 * @param len  Number of bytes to include in the calculation.
 * @return Computed CRC-8 byte.
 */
uint8_t calculateCrc(uint8_t* data, uint8_t len);

//////////////////////////////////////////////////////////////////////////////////
/////////// sleeper_t ////////////////////////////////////////////////////////////

/**
 * @brief Non-blocking timer utility – similar to HAL_Delay but without CPU blocking.
 *
 * The caller polls sleeper_IsElapsed() or sleeper_IsElapsedNext() inside its
 * main loop to detect when the configured time interval has elapsed.
 * The timer can also be stopped; in that case all _IsElapsedXX functions
 * return 0 regardless of elapsed time.
 */
#pragma pack(1)
typedef struct
{
	uint32_t SleepMS;	/**< Duration (ms) to wait; 0 means no pause */
	uint32_t InicTime;	/**< Reference timestamp captured at (re)start, used for comparison */
	uint8_t Stop;		/**< 1 – timer is stopped (all IsElapsed checks return 0); 0 – running */
} sleeper_t;
#pragma pack()

/**
 * @brief Initialise the sleeper and start timing from now.
 * @param v       Pointer to the sleeper_t instance to initialise.
 * @param sleepMS Duration in milliseconds after which IsElapsed returns 1.
 *                Pass 0 to create a stopped / instant timer.
 */
void sleeper_Init(sleeper_t *v, uint32_t sleepMS);

/**
 * @brief Check whether the configured time interval has elapsed.
 * @param v Pointer to a const sleeper_t instance.
 * @retval 1 The time has elapsed (or SleepMS == 0 and Stop == 0).
 * @retval 0 The time has not yet elapsed, or the timer is stopped.
 */
int sleeper_IsElapsed(const sleeper_t *v);

/**
 * @brief Check whether the time has elapsed and, if so, restart the timer for
 *        the next period (InicTime advances by SleepMS to avoid drift).
 * @param v Pointer to the sleeper_t instance.
 * @retval 1 The time has elapsed; timer restarted for the next cycle.
 * @retval 0 The time has not yet elapsed, or the timer is stopped.
 */
int sleeper_IsElapsedNext(sleeper_t *v);

/**
 * @brief Restart the timer so the next period begins from now.
 *        InicTime is updated to the current HAL tick.
 * @param v Pointer to the sleeper_t instance.
 */
void sleeper_Next(sleeper_t *v);

/**
 * @brief Change the sleep duration and restart the timer from now.
 * @param v        Pointer to the sleeper_t instance.
 * @param sleepMS  New duration in milliseconds.
 */
void sleeper_SetSleepMS(sleeper_t *v, uint32_t sleepMS);

/**
 * @brief Check whether the time has elapsed and, if so, stop the timer.
 * @note  Call sleeper_Next() to resume timing after this function returns 1.
 * @param v Pointer to the sleeper_t instance.
 * @retval 1 The time has elapsed; timer is now stopped.
 * @retval 0 The time has not yet elapsed, or the timer was already stopped.
 */
int sleeper_IsElapsedStop(sleeper_t *v);

/**
 * @brief Stop the timer so that all subsequent IsElapsed checks return 0.
 * @note  Call sleeper_Next() or sleeper_Init() to resume timing.
 * @param v Pointer to the sleeper_t instance.
 */
void sleeper_Stop(sleeper_t *v);

//////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Alias for the value type used by valueChanger_t (default: uint8_t).
 */
typedef uint8_t TVAL;

/**
 * @brief Value-stability detector – determines when a periodically updated
 *        value has remained unchanged for a configured time window.
 *
 * Call valueChanger_SetValue() each time a new sample arrives.
 * The function returns 1 only when the value has been stable (unchanged)
 * for the full duration specified at initialisation.
 *
 * When IsLocked is set, valueChanger_SetValue() always returns 0 until a
 * different value is provided, at which point the lock is automatically cleared.
 */
typedef struct
{
	TVAL LastValue;    /**< Most recently accepted value */
	sleeper_t Timer;   /**< Stability timer – restarted whenever the value changes */
	uint8_t IsLocked;  /**< 1 – locked (SetValue always returns 0); 0 – normal */
} valueChanger_t;

/**
 * @brief Initialise a valueChanger_t instance.
 * @param v         Pointer to the valueChanger_t to initialise.
 * @param inicValue Initial value stored in LastValue.
 * @param timeMS    Stability window in milliseconds; SetValue returns 1 only
 *                  after the value has been unchanged for this duration.
 */
void valueChanger_Inic(valueChanger_t *v, TVAL inicValue, uint32_t timeMS);

/**
 * @brief Submit a new value sample and check for stability.
 *
 * If newValue differs from LastValue, LastValue is updated, the stability
 * timer is restarted, and the instance is unlocked.
 * If newValue equals LastValue and the stability timer has elapsed, 1 is
 * returned to signal that the value has been stable for the full window.
 *
 * @param v        Pointer to the valueChanger_t instance.
 * @param newValue The latest sampled value.
 * @retval 1 The value has remained unchanged for the full stability window.
 * @retval 0 The value changed, the timer has not elapsed, or the instance is locked.
 */
int valueChanger_SetValue(valueChanger_t *v, TVAL newValue);

/**
 * @brief Lock the valueChanger_t so that SetValue always returns 0.
 *        The lock is cleared automatically the next time SetValue receives
 *        a value different from LastValue.
 * @param v Pointer to the valueChanger_t instance.
 */
void valueChanger_Lock(valueChanger_t *v);

/**
 * @brief Return the most recently stored value.
 * @param v Pointer to a const valueChanger_t instance.
 * @return Current value held in LastValue.
 */
TVAL valueChanger_GetValue(const valueChanger_t *v);

/////////////////////////////////////////////////////////////

/**
 * @brief Erase the external flash memory signature so that the next boot
 *        forces a full re-initialisation (used when flashing a new firmware
 *        version that changes persistent data structures).
 */
void clearFlash();

/**
 * @brief the calling of system restart with disabling of interrupts
 */
void systemRestart();

/////////////////////////////////////////////////////////////

#endif /* UTILS_UTILS_H_ */

/*
 * mydefs.h
 *
 * Common utility macros used across the application.
 *
 *  Created on: 23. 1. 2026
 *      Author: Milan
 */

#ifndef SRC_UTILS_MYDEFS_H_
#define SRC_UTILS_MYDEFS_H_

/**
 * @brief Compute the absolute value of x without using math.h
 * @param x Any numeric expression
 * @return Non-negative value of x
 */
#ifndef ABS
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#endif

/**
 * @brief printf format string for printing a float/double as integer with 2 decimal places.
 *        Use together with PRIf_02D to avoid floating-point printf on embedded targets.
 *
 * Example:
 * @code
 *   float val = 3.14f;
 *   printf("Value: " PRIf_02 "\n", PRIf_02D(val));  // prints "Value: 3.14"
 * @endcode
 */
#define PRIf_02 "%d.%02d"
#define PRIf_0X(X) "%" #X "d.%02d"

/**
 * @brief Expands to integer and fractional arguments for use with PRIf_02.
 *        Splits a float/double into the integer part and the two-digit fractional part.
 * @param fData The float or double value to format
 * @return Two comma-separated integer arguments: integer part and abs(fractional * 100)
 */
#define PRIf_02D(fData) (int)(fData), ABS(((int)((fData) * 100.0f) % 100))

/**
 * @brief Clamp val to the closed interval [minVal, maxVal].
 *        All three arguments are evaluated exactly once via temporary variables,
 *        avoiding double-evaluation side effects. Uses a GCC statement expression.
 * @param val    The value to clamp
 * @param minVal Lower bound (inclusive)
 * @param maxVal Upper bound (inclusive)
 * @return val if it is within [minVal, maxVal], minVal if val < minVal,
 *         or maxVal if val > maxVal
 */
#define CLAMP(val, minVal, maxVal) __extension__({ \
    __typeof__(val)    _v   = (val);    \
    __typeof__(minVal) _min = (minVal); \
    __typeof__(maxVal) _max = (maxVal); \
    _v < _min ? _min : (_v > _max ? _max : _v); \
})

/**
 * @brief The circular index GET - for circular queue both direction\n
 * Example usage:
 * @code
 * inx = INX_GET(inx+1, maxRange);
 * inx = INX_GET(inx-11, maxRange);
 * @endcode
 */
#define INX_GET(nInx, nMax) (((((nInx) % (nMax)) + (nMax)) % (nMax))) // circular queue

/**
 * @brief the macro for getting items count array
 */
#ifndef COUNT_OF
#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))
#endif

/*!
 * \brief Returns the minimum value between a and b
 *
 * \param [in] a 1st value
 * \param [in] b 2nd value
 * \retval minValue Minimum value
 */
#ifndef MIN
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

/*!
 * \brief Returns the maximum value between a and b
 *
 * \param [in] a 1st value
 * \param [in] b 2nd value
 * \retval maxValue Maximum value
 */
#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#endif /* SRC_UTILS_MYDEFS_H_ */

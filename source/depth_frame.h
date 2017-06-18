
#ifndef __DEPTH_FRAME_H__
#define __DEPTH_FRAME_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

using namespace std;

/// @brief Depth Frame structure to store depth data.
class DepthFrame{
public:
	int32_t w;            /// The Width of the depth frame
	int32_t h;            /// The Heigth of the depth frame 
	uint16_t *phase;      /// The Phase of the depth frame (distance = phase * K). Only low 12 bits has been used.
	uint16_t *amplitude;  /// The amplitude of each pixel. Only low 12 bits is used. (Some camera's low 4 bits is zero).
	uint8_t *ambient;     /// The ambient of each pixel. Only low 3 Bits has been used.
	uint8_t *flags;       /// The over explote flag of each pixel. Only low 1 bit has been used.
	bool amplitude_8bits; /// If the amplitude is only high 8bits.

	DepthFrame(int32_t _w, int32_t _h);
	~DepthFrame();

	/// @brief Copy frame data to given frame
	bool CopyTo(DepthFrame *df);
	/// @brief Copy frame data from given frame
	bool CopyFrom(const DepthFrame *df);
	/// @brief Copy frame data from given frame
	DepthFrame & operator=(const DepthFrame &from) { this->CopyFrom(&from); return *this; }
	/// @brief Calculate SUM of given rect from frame
	/// @param phase_or_amplitude 0 = phase, 1 = amplitude
	/// @param x Left-top point x of rect.
	/// @param y Left-top point y of rect.
	/// @param _w Rect width
	/// @param _h Rect Height
	/// @return SUM result
	uint32_t CalcRectSum(int32_t phase_or_amplitude, int32_t x, int32_t y, int32_t _w, int32_t _h);
	/// @brief Calculate SUM of given center rect from frame
	/// @param phase_or_amplitude 0 = phase, 1 = amplitude
	/// @param _w Rect width
	/// @param _h Rect Height
	/// @return SUM result
	uint32_t CalcCenterRectSum(int32_t phase_or_amplitude, int32_t _w, int32_t _h);
	/// @brief Convent all depth info to a Gray24 buffer
	/// @param gray24_buf Gray24 buffer
	/// @param gray24_buf_size the buffer size should >= w * h * 3 * 4
	/// @return ok = true else = false
	bool ToGray24(uint8_t *gray24_buf, int32_t gray24_buf_size);
	/// @brief Convent all depth info to a RGB24 buffer
	/// @param gray24_buf RGB24 buffer
	/// @param gray24_buf_size the buffer size should >= w * h * 3 * 4
	/// @return ok = true else = false
	bool ToRgb24(uint8_t *rgb24_buf, int32_t rgb24_buf_size);
	/// @brief Calculate Histogram of the frame
	/// @param phase_or_amplitude 0 = phase, 1 = amplitude
	/// @param histogram_buf The result buffer to store histogram. The length of this buffer should >= max
	/// @param max Maxium value of histogram (May consider to the length of the histogram_buf)
	/// @return max_histogram value
	template<typename T>
	inline T CalcHistogram(int32_t phase_or_amplitude, T *histogram_buf, int32_t max){
		const uint16_t * ptr = phase_or_amplitude ? amplitude : phase;
		int32_t size = w * h;
		T max_h = 0;
		memset(histogram_buf, 0, sizeof(T) * max);
		for (int i = 0; i < size; i++) {
			uint16_t v = *ptr++;
			if (phase_or_amplitude && amplitude_8bits)
				v >>= 4;
			if (v >= max) continue;			
			histogram_buf[v] ++;
		}

		for (int32_t i = 0; i < max; i++) {
			if (histogram_buf[i] > max_h) {
				max_h = histogram_buf[i];
			}				
		}
		return max_h;
	}

};


#endif


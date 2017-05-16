#include "depth_frame.h"
#include "string.h"

DepthFrame::DepthFrame(int32_t _w, int32_t _h)
{
	w = _w;
	h = _h;
	phase     = new uint16_t[w * h];
	amplitude = new uint16_t[w * h];
	ambient   = new uint8_t[w * h];
	flags     = new uint8_t[w * h];
	amplitude_8bits = true;
}

DepthFrame::~DepthFrame()
{
	delete[] phase;
	delete[] amplitude;
	delete[] ambient;
	delete[] flags;
}

bool DepthFrame::CopyFrom(const DepthFrame *df)
{
	if(!df || df->w != w || df->h != h)
		return false;
	int size = w * h;
	if (df->phase)     memcpy(phase,     df->phase,      size * sizeof(uint16_t));
	if (df->amplitude) memcpy(amplitude, df->amplitude,  size * sizeof(uint16_t));
	if (df->ambient)   memcpy(ambient,   df->ambient,    size * sizeof(uint8_t));
	if (df->flags)     memcpy(flags,     df->flags,      size * sizeof(uint8_t));
	return true;
}

uint32_t DepthFrame::CalcRectSum(int32_t phase_or_amplitude, int32_t x, int32_t y, int32_t _w, int32_t _h)
{
	const uint16_t * ptr = phase_or_amplitude ? amplitude : phase;
	const uint16_t *line_head = ptr + y * w + x;
	uint32_t sum = 0;
	for (int i = 0; i < _h; i++) {
		// calc line sum
		for (int j = 0; j < _w; j++) {
			sum += *line_head++;
		}
		// next line
		line_head = line_head - _w + w;
	}
	return sum;
}

uint32_t DepthFrame::CalcCenterRectSum(int32_t phase_or_amplitude, int32_t _w, int32_t _h)
{
	return CalcRectSum(phase_or_amplitude, (w - _w) / 2, (h - _h) / 2, _w, _h);
}

bool DepthFrame::ToRgb24(uint8_t * rgb24_buf, int32_t rgb24_buf_size)
{
	uint16_t *phase_ptr, *amplitude_ptr;
	uint8_t *flags_ptr, *ambient_ptr, *out = rgb24_buf;

	if (rgb24_buf_size < (w * h * 3 * 4))
		return false;

	phase_ptr = phase;
	amplitude_ptr = amplitude;
	ambient_ptr = ambient;
	flags_ptr = flags;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			uint16_t v16 = (*phase_ptr++);
			uint8_t v = v16 >> 4;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}

		for (int j = 0; j < w; j++) {
			uint8_t v = (*amplitude_ptr++) >> 4;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}
	}

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			uint8_t v = (*ambient_ptr++) << 5;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}

		for (int j = 0; j < w; j++) {
			uint8_t v = (*flags_ptr++) << 4;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}
	}

	return true;
}


bool DepthFrame::CopyTo(DepthFrame *df)
{
	if(!df || df->w != w || df->h != h)
		return false;
	int size = w * h;
	if (df->phase)     memcpy(df->phase,     phase,      size * sizeof(uint16_t));
	if (df->amplitude) memcpy(df->amplitude, amplitude,  size * sizeof(uint16_t));
	if (df->ambient)   memcpy(df->ambient,   ambient,    size * sizeof(uint8_t));
	if (df->flags)     memcpy(df->flags,     flags,      size * sizeof(uint8_t));
	return true;
}
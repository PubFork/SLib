/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "slib/media/codec_vpx.h"

#include "slib/core/log.h"
#include "slib/core/io.h"
#include "slib/core/scoped.h"

#include "vpx/vp8cx.h"
#include "vpx/vp8dx.h"
#include "vpx/vpx_encoder.h"
#include "vpx/vpx_decoder.h"

namespace slib
{

	VpxEncoderParam::VpxEncoderParam()
	{
		codec = VpxCodecName::VP8;
		bitrateMode = VpxBitrateMode::Constant;
		width = height = 192;
		framesPerSecond = 25;
		bitrate = 150;
		keyFrameInterval = 5;
		cpuUsage = 3;
		threadsCount = 1;
	}

	VpxEncoderParam::~VpxEncoderParam()
	{
	}


	VpxDecoderParam::VpxDecoderParam()
	{
		codec = VpxCodecName::VP8;
		width = height = 192;
	}

	VpxDecoderParam::~VpxDecoderParam()
	{
	}


	VpxEncoder::VpxEncoder()
	{
	}

	VpxEncoder::~VpxEncoder()
	{
	}

	class _priv_VpxEncoderImpl : public VpxEncoder
	{
	public:
		sl_uint32 m_nProcessFrameCount;
		vpx_codec_enc_cfg_t m_codec_config;
		vpx_codec_ctx_t* m_codec;
		vpx_image_t* m_codec_image;

	public:
		_priv_VpxEncoderImpl()
		{
			m_codec = sl_null;
			m_codec_image = sl_null;
			m_nProcessFrameCount = 0;
		}

		~_priv_VpxEncoderImpl()
		{
			if (m_codec_image != sl_null) {
				vpx_img_free(m_codec_image);
				delete m_codec_image;
				m_codec_image = sl_null;
			}
			if (m_codec != sl_null) {
				vpx_codec_destroy(m_codec);
				delete m_codec;
				m_codec = sl_null;
			}
		}

	public:
		static void logError(String str)
		{
			LogError("VideoVpxEncoder", str);
		}
		
		static vpx_rc_mode _getBitrateControlMode(VpxBitrateMode mode)
		{
			switch (mode) {
				case VpxBitrateMode::Variable:
					return VPX_VBR;
				case VpxBitrateMode::Constant:
					return VPX_CBR;
				case VpxBitrateMode::ConstrainedQuality:
					return VPX_CQ;
				case VpxBitrateMode::ConstantQuality:
					return VPX_Q;
			}
			return VPX_CBR;
		}

		static Ref<_priv_VpxEncoderImpl> create(const VpxEncoderParam& param)
		{
			vpx_codec_enc_cfg_t codec_config;
			vpx_codec_iface_t* codec_interface;
			switch (param.codec) {
				case VpxCodecName::VP9:
					codec_interface = vpx_codec_vp9_cx();
					break;
				default:
					codec_interface = vpx_codec_vp8_cx();
					break;
			}
			if (codec_interface) {
				vpx_codec_ctx_t* codec = new vpx_codec_ctx_t;
				if (codec) {
					vpx_image_t* codec_image = new vpx_image_t;
					if (codec_image) {
						if (vpx_img_alloc(codec_image, VPX_IMG_FMT_I420, param.width, param.height, 1)) {
							vpx_codec_err_t res = vpx_codec_enc_config_default(codec_interface, &codec_config, 0);
							if (!res) {
								codec_config.g_w = param.width;
								codec_config.g_h = param.height;
								codec_config.rc_end_usage = _getBitrateControlMode(param.bitrateMode);
								codec_config.rc_target_bitrate = param.bitrate;
								codec_config.g_threads = param.threadsCount;
								codec_config.g_usage = param.cpuUsage;
								codec_config.g_timebase.den = param.framesPerSecond;
								codec_config.g_timebase.num = 1;
								
								if (!vpx_codec_enc_init(codec, codec_interface, &codec_config, 0)) {
									Ref<_priv_VpxEncoderImpl> ret = new _priv_VpxEncoderImpl();
									if (ret.isNotNull()) {
										ret->m_codec = codec;
										ret->m_codec_config = codec_config;
										ret->m_codec_image = codec_image;
										ret->m_nWidth = param.width;
										ret->m_nHeight = param.height;
										ret->m_nKeyFrameInterval = param.keyFrameInterval;
										ret->setBitrate(param.bitrate);
										return ret;
									}
									vpx_codec_destroy(codec);
								} else {
									logError("Failed to initialize video encoder codec.");
								}
							} else {
								logError("Failed to set default video encoder codec configuration.");
							}
							vpx_img_free(codec_image);
						} else {
							logError("Failed to allocate video codec image");
						}
						delete codec_image;
					}
					delete codec;
				}
			} else {
				logError("Video codec is not supported");
			}
			
			return sl_null;
		}

		Memory encode(const VideoFrame& input) override
		{
			if (m_nWidth == input.image.width && m_nHeight == input.image.height) {
				
				BitmapData dst;
				dst.width = m_codec_image->w;
				dst.height = m_codec_image->h;
				dst.format = BitmapFormat::YUV_I420;
				dst.data = m_codec_image->planes[0];
				dst.pitch = m_codec_image->stride[0];
				dst.data1 = m_codec_image->planes[1];
				dst.pitch1 = m_codec_image->stride[1];
				dst.data2 = m_codec_image->planes[2];
				dst.pitch2 = m_codec_image->stride[2];
				
				dst.copyPixelsFrom(input.image);
				
				sl_int32 flags = 0;
				if (m_nProcessFrameCount > 0 && m_nProcessFrameCount % m_nKeyFrameInterval == 0) {
					flags |= VPX_EFLAG_FORCE_KF;
				}
				vpx_codec_err_t res = vpx_codec_encode(m_codec, m_codec_image, m_nProcessFrameCount++, 1, flags, VPX_DL_REALTIME);
				if (res == VPX_CODEC_OK) {
					vpx_codec_iter_t iter = sl_null;
					const vpx_codec_cx_pkt_t *pkt = sl_null;
					MemoryWriter encodeWriter;

					while ((pkt = vpx_codec_get_cx_data(m_codec, &iter)) != sl_null) {
						if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
							//const int keyframe = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) != 0;
							encodeWriter.writeInt64(pkt->data.frame.pts);
							encodeWriter.writeInt64(pkt->data.frame.sz);
							encodeWriter.write(pkt->data.frame.buf, pkt->data.frame.sz);
						}
					}

					return encodeWriter.getData();
					
				} else {
					logError("Failed to encode bitmap data.");
				}
			} else {
				logError("VideoFrame size is wrong.");
			}
			return sl_null;
		}

		void setBitrate(sl_uint32 _bitrate) override
		{
			sl_uint32 bitrate = _bitrate;
			if (bitrate < 50) {
				bitrate = 50;
			}
			if (bitrate > 1500) {
				bitrate = 1500;
			}
			VideoEncoder::setBitrate(bitrate);
		}
	};


	VpxDecoder::VpxDecoder()
	{
	}

	VpxDecoder::~VpxDecoder()
	{
	}

	class _priv_VpxDecoderImpl : public VpxDecoder
	{
	public:
		vpx_codec_ctx_t* m_codec;

	public:
		_priv_VpxDecoderImpl()
		{
			m_codec = sl_null;
		}
		
		~_priv_VpxDecoderImpl()
		{
			if (m_codec != sl_null) {
				vpx_codec_destroy(m_codec);
				delete m_codec;
				m_codec = sl_null;
			}
		}

	public:
		static void logError(String str)
		{
			LogError("VideoVpxDecoder", str);
		}

		static Ref<_priv_VpxDecoderImpl> create(const VpxDecoderParam& param)
		{
			vpx_codec_iface_t* codec_interface;
			switch (param.codec) {
				case VpxCodecName::VP9:
					codec_interface = vpx_codec_vp9_dx();
					break;
				default:
					codec_interface = vpx_codec_vp8_dx();
					break;
			}
			if (codec_interface) {
				vpx_codec_ctx_t* codec = new vpx_codec_ctx_t;
				if (codec) {
					if (!vpx_codec_dec_init(codec, codec_interface, NULL, 0)) {
						Ref<_priv_VpxDecoderImpl> ret = new _priv_VpxDecoderImpl;
						if (ret.isNotNull()) {
							ret->m_nWidth = param.width;
							ret->m_nHeight = param.height;
							ret->m_codec = codec;
							return ret;
						}
						vpx_codec_destroy(codec);
					} else{
						logError("Failed initialize video decoder codec.");
					}
				}
			} else {
				logError("Video codec is not supported");
			}
			return sl_null;
		}

		SLIB_INLINE sl_int32 vpx_img_plane_width(const vpx_image_t *img, sl_int32 plane)
		{
			if (plane > 0 && img->x_chroma_shift > 0) {
				return (img->d_w + 1) >> img->x_chroma_shift;
			} else {
				return img->d_w;
			}
		}

		SLIB_INLINE sl_int32 vpx_img_plane_height(const vpx_image_t *img, sl_int32 plane)
		{
			if (plane > 0 && img->y_chroma_shift > 0) {
				return (img->d_h + 1) >> img->y_chroma_shift;
			} else {
				return img->d_h;
			}
		}

		sl_bool decode(const void* input, const sl_uint32& inputSize , VideoFrame& output)
		{
			MemoryReader reader(input, inputSize);
			sl_int64 pts = reader.readInt64();
			SLIB_UNUSED(pts);
			sl_int64 size = reader.readInt64();

			if (!vpx_codec_decode(m_codec, (sl_uint8*)input + 16, (unsigned int)size, NULL, 0)) {
				
				vpx_codec_iter_t iter = NULL;
				
				vpx_image_t* image;

				while ((image = vpx_codec_get_frame(m_codec, &iter)) != NULL) {
					
					BitmapData src;
					src.width = image->w;
					src.height = image->h;
					src.format = BitmapFormat::YUV_I420;
					src.data = image->planes[0];
					src.pitch = image->stride[0];
					src.data1 = image->planes[1];
					src.pitch1 = image->stride[1];
					src.data2 = image->planes[2];
					src.pitch2 = image->stride[2];
					
					output.image.copyPixelsFrom(src);				
				}
			}
			return sl_false;
		}
	};

	Ref<VpxEncoder> VpxEncoder::create(const VpxEncoderParam& param)
	{
		return _priv_VpxEncoderImpl::create(param);
	}

	Ref<VpxDecoder> VpxDecoder::create(const VpxDecoderParam& param)
	{
		return _priv_VpxDecoderImpl::create(param);
	}

}

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

#include "slib/core/definition.h"

#if defined(SLIB_PLATFORM_IS_TIZEN)

#include "slib/media/media_player.h"
#include "slib/core/platform_tizen.h"

#include <player.h>

namespace slib
{
	class _priv_MediaPlayer : public MediaPlayer
	{
	public:
		player_h m_player;

		sl_bool m_flagInited;
		sl_bool m_flagPlaying;
		sl_bool m_flagPrepared;

		sl_real m_volume;

	public:
		_priv_MediaPlayer()
		{
			m_player = sl_null;
			m_flagInited = sl_false;
			m_flagPlaying = sl_false;
			m_flagPrepared = sl_false;
			m_volume = 1;
		}

		~_priv_MediaPlayer()
		{
			SLIB_REFERABLE_DESTRUCTOR
			_release(sl_true);
		}

	public:
		static Ref<_priv_MediaPlayer> create(const MediaPlayerParam& param)
		{
			player_h player;
			int errorCode = ::player_create(&player);

			if (PLAYER_ERROR_NONE != errorCode) {
				return sl_null;
			}

			if (param.url.isNotEmpty()) {
				errorCode = ::player_set_uri(player, param.url.getData());
			} else if (param.filePath.isNotEmpty()) {
				errorCode = ::player_set_uri(player, param.filePath.getData());
			} else if (param.assetFileName.isNotEmpty()) {
				errorCode = ::player_set_uri(player, Tizen::getAssetFilePath(param.assetFileName).getData());
			}

			if (PLAYER_ERROR_NONE == errorCode) {

				Ref<_priv_MediaPlayer> ret = new _priv_MediaPlayer;

				if (ret.isNotNull()) {

					ret->m_player = player;
					ret->m_flagInited = sl_true;
					ret->_init(param);

					errorCode = ::player_prepare_async(player, _priv_MediaPlayer::_onPrepared, ret.get());

					if (PLAYER_ERROR_NONE == errorCode) {

						::player_set_completed_cb(player, _priv_MediaPlayer::_onReachEnd, ret.get());

						if (param.flagAutoStart) {
							ret->resume();
						}

						return ret;
					}

					return sl_null;
				}

				::player_destroy(player);
			}

			return sl_null;

		}

	public:
		static void _onPrepared(void* userData)
		{
			Ref<_priv_MediaPlayer> player = (_priv_MediaPlayer*)userData;
			player->_onPrepared();

		}

		static void _onReachEnd(void* userData)
		{
			Ref<_priv_MediaPlayer> player = (_priv_MediaPlayer*)userData;
			player->_onReachEnd();
		}

		void _onPrepared()
		{
			ObjectLocker lock(this);
			if (!m_flagInited) {
				return;
			}
			player_state_e state = PLAYER_STATE_NONE;
			::player_get_state(m_player, &state);
			if (state == PLAYER_STATE_READY) {
				m_flagPrepared = sl_true;
				if (m_flagPlaying) {
					::player_start(m_player);
				}
				::player_set_volume(m_player, m_volume, m_volume);
			} else {
				_release(sl_false);
			}
		}

		void _onReachEnd()
		{
			ObjectLocker lock(this);
			if (!m_flagInited) {
				return;
			}
			if (m_flagAutoRepeat) {
				::player_set_play_position(m_player, 0, sl_true, NULL, NULL);
			} else {
				_release(sl_false);
			}
		}

		void _release(sl_bool flagFromDestructor)
		{
			ObjectLocker lock(this);
			if (!m_flagInited) {
				return;
			}

			m_flagInited = sl_false;
			m_flagPlaying = sl_false;
			lock.unlock();

			if (!flagFromDestructor) {
				_removeFromMap();
			}

			::player_stop(m_player);
			::player_unprepare(m_player);
			::player_destroy(m_player);

			m_player = sl_null;

		}

		void release()
		{
			_release(sl_false);
		}

		void resume()
		{
			ObjectLocker lock(this);

			if (!m_flagInited) {
				return;
			}
			if (m_flagPlaying) {
				return;
			}

			if (m_flagPrepared) {
				int errorCode = ::player_start(m_player);
				if (PLAYER_ERROR_NONE != errorCode) {
					return;
				}
			}

			m_flagPlaying = sl_true;

			_addToMap();

		}

		void pause()
		{
			ObjectLocker lock(this);

			if (!m_flagInited) {
				return;
			}
			if (!m_flagPlaying) {
				return;
			}

			if (m_flagPrepared) {
				int errorCode = ::player_pause(m_player);
				if (PLAYER_ERROR_NONE != errorCode) {
					return;
				}
			}

			m_flagPlaying = sl_false;

			_removeFromMap();

		}

		sl_bool isPlaying()
		{
			return m_flagPlaying;
		}

		sl_real getVolume()
		{
			return m_volume;
		}

		void setVolume(sl_real volume)
		{
			ObjectLocker lock(this);

			if (!m_flagInited) {
				return;
			}

			if (m_flagPrepared) {
				::player_set_volume(m_player, volume, volume);
			}

			m_volume = volume;
		}

		void renderVideo(MediaPlayerRenderVideoParam& param)
		{
		}

	};

	Ref<MediaPlayer> MediaPlayer::_createNative(const MediaPlayerParam& param)
	{
		return _priv_MediaPlayer::create(param);
	}
}

#endif
